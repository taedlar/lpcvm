// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "config.hpp"
#include "fabric.hpp"
#include <algorithm>
#include "lpc_types.hpp"
#include "vm_svalue.hpp"
#include <cassert>
#include <memory>
#include <functional>

std::size_t LpcMapping::string_hash(std::string_view value) noexcept {
    return std::hash<std::string_view>{}(value);
}

std::size_t LpcMapping::hash(const lpcvm::svalue_t& key) noexcept {
    assert(!key.value.valueless_by_exception());
    if (key.value.valueless_by_exception())
        return 0;

    const auto value_hash = std::visit([](const auto& value) noexcept -> std::size_t {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, std::monostate>) {
            return 0;
        } else if constexpr (std::is_arithmetic_v<T>) {
            // std::hash also gives +0 and -0 the same hash.
            return std::hash<T>{}(value);
        } else if constexpr (std::is_same_v<T, LpcStringRef>) {
            return value ? string_hash(value->view()) : 0;
        } else if constexpr (std::is_same_v<T, LpcBufferRef>) {
            return value ? string_hash(value->data) : 0;
        } else if constexpr (std::is_same_v<T, TemporaryString>) {
            // The descriptor alone cannot resolve the stack-owned bytes.
            assert(false && "Use string_hash with the temporary string's stack-owned bytes");
            return 0;
        } else if constexpr (std::is_same_v<T, LpcObject::HandleType>) {
            // Keep the identity even after expiration; locking would lose it.
            return std::hash<const void*>{}(value.ptr);
        } else {
            // Arrays and mappings have reference semantics.
            return std::hash<const void*>{}(value ? std::addressof(*value) : nullptr);
        }
    }, key.value);
    // String representations share a hash, including resolved temporary strings.
    if (std::holds_alternative<LpcStringRef>(key.value) ||
        std::holds_alternative<LpcBufferRef>(key.value))
        return value_hash;
    // Mapping keys retain the numeric variant type, as constant-pool keys do.
    return value_hash ^ (key.value.index() + std::size_t{0x9e3779b9}
                         + (value_hash << 6) + (value_hash >> 2));
}

LpcMappingRef LpcMapping::create(std::shared_ptr<FabricType> sb, std::size_t size) {
    if (!sb)
        throw std::invalid_argument("A fabric is required to create a tracked mapping");
    return sb->make_container<LpcMappingRef>(size);
}

LpcMapping::LpcMapping(std::size_t size) {
    if (size > MAX_ENTRIES)
        throw std::length_error("Mapping capacity exceeds the supported limit");
    if (size > 0) {
        capacity_ = static_cast<uint32_t>((size + ENTRY_CHUNK_SIZE - 1) / ENTRY_CHUNK_SIZE
                                          * ENTRY_CHUNK_SIZE);
        entries_ = new lpcvm::svalue_pair_t[capacity_];
    }
}

LpcMapping::~LpcMapping() {
    clear();
}

void LpcMapping::clear() noexcept {
    // Detach storage before releasing references that can destroy other containers.
    auto* entries = std::exchange(entries_, nullptr);
    auto indices = std::exchange(indices_, std::monostate{});
    size_ = 0;
    capacity_ = 0;
    erased_ = 0;
    bucket_bits_ = 0;
    std::visit([](auto buckets) {
        if constexpr (!std::is_same_v<decltype(buckets), std::monostate>)
            delete[] buckets;
    }, indices);
    delete[] entries;
}

void LpcMapping::visit_edges(EdgeVisitor visitor, void* context) const noexcept {
    const auto visit = [&](const SValue& value) {
        if (const auto* array = std::get_if<LpcArrayRef>(&value); array && *array)
            visitor(&**array, context);
        else if (const auto* mapping = std::get_if<LpcMappingRef>(&value); mapping && *mapping)
            visitor(&**mapping, context);
    };
    for (uint32_t i = 0; i < size_; ++i) {
        // Each key/value slot owns a separate reference, even when targets repeat.
        visit(entries_[i].first);
        visit(entries_[i].second.value);
    }
}

uint32_t LpcMapping::append(const lpcvm::svalue_pair_t& entry) {
    if (size_ >= MAX_ENTRIES)
        throw std::length_error("Mapping capacity exceeds the supported limit");
    if (size_ == capacity_) {
        const uint32_t new_capacity = capacity_ == 0 ? ENTRY_CHUNK_SIZE
            : static_cast<uint32_t>(std::min(uint64_t{capacity_} * 2, uint64_t{MAX_ENTRIES}));
        auto new_entries = std::make_unique<lpcvm::svalue_pair_t[]>(new_capacity);
        if (size_)
            std::copy_n(entries_, size_, new_entries.get());
        // Copy before releasing storage in case entry aliases an existing entry.
        new_entries[size_] = entry;
        delete[] entries_;
        entries_ = new_entries.release();
        capacity_ = new_capacity;
    } else {
        entries_[size_] = entry;
    }
    return size_++;
}

void LpcMapping::grow() {
    if (bucket_bits_ >= 31)
        throw std::length_error("Mapping hash table exceeds the supported limit");
    const uint32_t new_bits = bucket_bits_ == 0 ? 3 : bucket_bits_ + 1;
    rehash(new_bits);
}

void LpcMapping::rehash(uint32_t new_bits) {
    const std::size_t count = std::size_t{1} << new_bits;
    const auto rebuild = [&](auto index_tag) {
        using Index = decltype(index_tag);
        auto buckets = std::make_unique<Index[]>(count);
        std::fill_n(buckets.get(), count, static_cast<Index>(IX_EMPTY));
        for (uint32_t i = 0; i < size_; ++i) {
            const auto key_hash = hash({entries_[i].first});
            auto bucket = key_hash & (count - 1);
            auto perturb = key_hash;
            while (buckets[bucket] >= 0) {
                bucket = (bucket * 5 + 1 + perturb) & (count - 1);
                perturb >>= 5;
            }
            buckets[bucket] = static_cast<Index>(i);
        }
        // Commit only after allocation and rehashing succeed.
        std::visit([](auto indices) {
            if constexpr (!std::is_same_v<decltype(indices), std::monostate>)
                delete[] indices;
        }, indices_);
        indices_ = buckets.release();
        bucket_bits_ = new_bits;
        erased_ = 0;
    };
    // At 50% occupancy, the largest index is count / 2 - 1.
    if (count / 2 - 1 <= std::numeric_limits<int8_t>::max())
        rebuild(int8_t{});
    else if (count / 2 - 1 <= std::numeric_limits<int16_t>::max())
        rebuild(int16_t{});
    else
        rebuild(int32_t{});
}

std::optional<uint32_t> LpcMapping::find_index(const lpcvm::svalue_t& key) const noexcept {
    if (size_ == 0 || bucket_bits_ == 0)
        return std::nullopt;
    const auto key_hash = hash(key);
    const auto mask = (std::size_t{1} << bucket_bits_) - 1;
    return std::visit([&](auto indices) -> std::optional<uint32_t> {
        if constexpr (std::is_same_v<decltype(indices), std::monostate>) {
            return std::nullopt;
        } else {
            auto bucket = key_hash & mask;
            auto perturb = key_hash;
            while (indices[bucket] != IX_EMPTY) {
                const auto index = indices[bucket];
                if (index >= 0 && static_cast<uint32_t>(index) < size_ &&
                    lpcvm::compare_svalues(entries_[index].first, key.value) == 0)
                    return static_cast<uint32_t>(index);
                bucket = (bucket * 5 + 1 + perturb) & mask;
                perturb >>= 5;
            }
            return std::nullopt;
        }
    }, indices_);
}

lpcvm::svalue_t* LpcMapping::find(const lpcvm::svalue_t& key) noexcept {
    const auto index = find_index(key);
    return index ? &entries_[*index].second : nullptr;
}

const lpcvm::svalue_t* LpcMapping::find(const lpcvm::svalue_t& key) const noexcept {
    const auto index = find_index(key);
    return index ? &entries_[*index].second : nullptr;
}

lpcvm::svalue_t& LpcMapping::find_or_insert(const lpcvm::svalue_t& key) {
    if (key.value.valueless_by_exception() || std::holds_alternative<TemporaryString>(key.value))
        throw std::invalid_argument("Mapping keys must be materialized values");
    if (auto* existing = find(key))
        return *existing;

    lpcvm::svalue_pair_t entry{key.value, {std::monostate{}}};
    if (const auto* buffer = std::get_if<LpcBufferRef>(&key.value); buffer && *buffer) {
        auto* owner = fabric();
        if (!owner)
            throw std::runtime_error("A live fabric is required to intern a buffer key");
        entry.first = owner->make_shared_string((*buffer)->data);
    }
    const auto key_hash = hash({entry.first});
    if (need_grow())
        grow();
    else if (size_ + erased_ >= (uint32_t{1} << (bucket_bits_ - 1)))
        rehash(bucket_bits_);
    const auto index = append(entry);
    const auto mask = (std::size_t{1} << bucket_bits_) - 1;
    std::visit([&](auto indices) {
        if constexpr (!std::is_same_v<decltype(indices), std::monostate>) {
            auto bucket = key_hash & mask;
            auto perturb = key_hash;
            while (indices[bucket] >= 0) {
                bucket = (bucket * 5 + 1 + perturb) & mask;
                perturb >>= 5;
            }
            using Index = std::remove_pointer_t<decltype(indices)>;
            if (indices[bucket] == IX_ERASED)
                --erased_;
            indices[bucket] = static_cast<Index>(index);
        }
    }, indices_);
    return entries_[index].second;
}

void LpcMapping::erase(const lpcvm::svalue_t& key) {
    if (key.value.valueless_by_exception() || std::holds_alternative<TemporaryString>(key.value))
        throw std::invalid_argument("Mapping keys must be materialized values");
    const auto index = find_index(key);
    if (!index)
        return;

    // Resolve both hashes before touching entries: key may alias a mapped value.
    const auto key_hash = hash(key);
    const auto last = size_ - 1;
    const auto last_hash = *index != last ? hash({entries_[last].first}) : 0;
    const auto mask = (std::size_t{1} << bucket_bits_) - 1;
    std::visit([&](auto indices) {
        if constexpr (!std::is_same_v<decltype(indices), std::monostate>) {
            using Index = std::remove_pointer_t<decltype(indices)>;
            const auto bucket_for = [&](std::size_t key_hash, uint32_t entry_index) {
                auto bucket = key_hash & mask;
                auto perturb = key_hash;
                while (indices[bucket] != static_cast<Index>(entry_index)) {
                    bucket = (bucket * 5 + 1 + perturb) & mask;
                    perturb >>= 5;
                }
                return bucket;
            };
            const auto bucket = bucket_for(key_hash, *index);
            if (*index != last) {
                const auto last_bucket = bucket_for(last_hash, last);
                std::swap(entries_[*index], entries_[last]);
                indices[last_bucket] = static_cast<Index>(*index);
            }
            indices[bucket] = static_cast<Index>(IX_ERASED);
            ++erased_;
            --size_;
            // Release the removed key and value immediately, retaining capacity.
            entries_[last] = {};
            if (size_ == 0) {
                std::fill_n(indices, mask + 1, static_cast<Index>(IX_EMPTY));
                erased_ = 0;
            }
        }
    }, indices_);
}
