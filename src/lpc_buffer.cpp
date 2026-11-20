// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include "lpc_types.hpp"
#include "fabric.hpp"
#include <new>

namespace {
constexpr auto node_size = sizeof(LpcBufferPool::ControlBlock) + sizeof(LpcBuffer);
static_assert(sizeof(LpcBufferPool::ControlBlock) % alignof(LpcBuffer) == 0,
              "buffer must be aligned after the control block");
}

std::size_t LpcBufferPool::small_capacity() noexcept {
    static const std::size_t capacity = std::string().capacity();
    return capacity;
}

LpcBufferPool* LpcBufferPool::create(BufferPoolLimits limits) {
    return new LpcBufferPool(limits);
}

LpcBufferPool::~LpcBufferPool() {
    trim();
}

LpcBuffer* LpcBufferPool::buffer_of(ControlBlock* cb) noexcept {
    return std::launder(reinterpret_cast<LpcBuffer*>(cb + 1));
}

LpcBufferPool::ControlBlock* LpcBufferPool::allocate(std::size_t capacity) {
    // One allocation: [control block][LpcBuffer]; large bytes live in std::string.
    void* memory = ::operator new(node_size);
    auto* cb = new (memory) ControlBlock{};
    auto* buffer = new (static_cast<void*>(cb + 1)) LpcBuffer();
    try {
        buffer->data.reserve(capacity);
    } catch (...) {
        destroy(cb);
        throw;
    }
    return cb;
}

void LpcBufferPool::destroy(ControlBlock* cb) noexcept {
    buffer_of(cb)->~LpcBuffer();
    cb->~ControlBlock();
    ::operator delete(static_cast<void*>(cb));
}

LpcBufferRef LpcBufferPool::acquire(std::size_t capacity) {
    ControlBlock* cb;
    if (capacity <= small_capacity() && small_free_) {
        cb = small_free_;
        small_free_ = cb->next;
        --stats_.small_buffers;
    } else if (capacity > small_capacity() && large_free_) {
        auto& data = buffer_of(large_free_)->data;
        const auto retained = data.capacity();
        data.reserve(capacity); // on failure the node stays recycled
        cb = large_free_;
        large_free_ = cb->next;
        --stats_.large_buffers;
        stats_.large_capacity -= retained;
    } else {
        cb = allocate(capacity);
    }
    cb->ref_count = 1;
    cb->pool = this;
    ++ref_count_;
    ++stats_.live_buffers;
    return LpcBufferRef(buffer_of(cb), LpcBufferRef::AdoptRetainedReference{});
}

void LpcBufferPool::recycle(ControlBlock* cb) noexcept {
    LpcBufferPool* pool = cb->pool;
    auto& data = buffer_of(cb)->data;
    const auto capacity = data.capacity();
    --pool->stats_.live_buffers;

    if (pool->closed_) {
        destroy(cb);
    } else if (capacity <= small_capacity()) {
        if (pool->stats_.small_buffers < pool->limits_.max_small_buffers) {
            data.clear();
            cb->next = pool->small_free_;
            pool->small_free_ = cb;
            ++pool->stats_.small_buffers;
        } else {
            destroy(cb);
        }
    } else if (capacity <= pool->limits_.max_large_capacity
               && pool->stats_.large_buffers < pool->limits_.max_large_buffers) {
        data.clear();
        cb->next = pool->large_free_;
        pool->large_free_ = cb;
        ++pool->stats_.large_buffers;
        pool->stats_.large_capacity += capacity;
    } else {
        destroy(cb);
    }
    pool->unref();
}

std::size_t LpcBufferPool::trim() noexcept {
    std::size_t bytes = 0;
    while (small_free_) {
        auto* cb = std::exchange(small_free_, small_free_->next);
        destroy(cb);
        bytes += node_size;
    }
    while (large_free_) {
        auto* cb = std::exchange(large_free_, large_free_->next);
        bytes += node_size + buffer_of(cb)->data.capacity() + 1;
        destroy(cb);
    }
    stats_.small_buffers = 0;
    stats_.large_buffers = 0;
    stats_.large_capacity = 0;
    return bytes;
}

void LpcBufferPool::close() noexcept {
    trim();
    closed_ = true;
    unref();
}

LpcBufferPool::BufferPoolStats LpcBufferPool::stats() const noexcept {
    return stats_;
}

void LpcBufferPool::unref() noexcept {
    if (--ref_count_ == 0)
        delete this;
}

LpcBufferRef ManagedRuntime::make_buffer(std::size_t capacity) {
    if (!buffer_pool_)
        buffer_pool_ = LpcBufferPool::create();
    return buffer_pool_->acquire(capacity);
}

LpcBufferPool::BufferPoolStats ManagedRuntime::buffer_pool_stats() const noexcept {
    return buffer_pool_ ? buffer_pool_->stats() : LpcBufferPool::BufferPoolStats{};
}

std::size_t ManagedRuntime::trim_buffer_pool() noexcept {
    return buffer_pool_ ? buffer_pool_->trim() : 0;
}
