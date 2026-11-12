// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include <gtest/gtest.h>
#include <memory>
#include <optional>
#include <type_traits>
#include "lpc_types.hpp"
#include "object_class.hpp"
#include "vm_sandbox.hpp"

static_assert(!std::is_copy_constructible_v<ManagedRuntime>);
static_assert(!std::is_copy_assignable_v<ManagedRuntime>);
static_assert(!std::is_move_constructible_v<ManagedRuntime>);
static_assert(!std::is_move_assignable_v<ManagedRuntime>);
static_assert(std::is_same_v<decltype(std::declval<ManagedRuntime&>().objects()),
                             const ObjectTable&>);

TEST(ManagedRuntimeTest, ScopeExitReleasesObjectsAndStrings) {
    ObjectHandle object;
    lpcvm::Handle<LpcString, LpcStringRef> string;
    {
        auto runtime = std::make_shared<ManagedRuntime>();
        EXPECT_TRUE(runtime->objects().empty());
        object = LpcObject::create(runtime, "test/object", nullptr);
        auto name = runtime->find_object("test/object")->name();
        string = decltype(string)(name);
        EXPECT_FALSE(object.expired());
        EXPECT_FALSE(string.expired());
    }
    EXPECT_TRUE(object.expired());
    EXPECT_TRUE(string.expired());
}

TEST(ManagedRuntimeTest, ExceptionUnwindingReleasesOwnedResources) {
    ObjectHandle object;
    lpcvm::Handle<LpcString, LpcStringRef> string;
    EXPECT_THROW({
        auto runtime = std::make_shared<ManagedRuntime>();
        object = LpcObject::create(runtime, "test/object", nullptr);
        auto name = runtime->find_object("test/object")->name();
        string = decltype(string)(name);
        throw std::runtime_error("test failure");
    }, std::runtime_error);
    EXPECT_TRUE(object.expired());
    EXPECT_TRUE(string.expired());
}

TEST(LpcObjectHandleTest, LockRetainsObjectUntilStrongReferenceIsReleased) {
    LpcObject::HandleType handle;
    std::optional<LpcObjectRef> locked;
    {
        auto owner = lpcvm::create_shared<LpcObject>(nullptr, std::make_shared<ObjectClass>());
        handle = LpcObject::HandleType(owner);
        locked = handle.lock();
        ASSERT_TRUE(locked.has_value());
        EXPECT_EQ(*locked, owner);
    }

    EXPECT_TRUE(handle.lock().has_value());
    locked.reset();
    EXPECT_TRUE(handle.expired());
    EXPECT_FALSE(handle.lock().has_value());
}

TEST(LpcObjectHandleTest, HandleOutlivesObject) {
    auto owner = std::make_optional<LpcObjectRef>(lpcvm::create_shared<LpcObject>(nullptr, std::make_shared<ObjectClass>()));
    auto h1 = LpcObject::HandleType(*owner);
    auto h2 = h1;
    owner.reset();
    EXPECT_FALSE(h1.lock().has_value());
    h1 = LpcObject::HandleType{};
    EXPECT_TRUE(h2.expired());
}

TEST(LpcObjectTest, CreatingDuplicateNameReplacesWithoutInvalidatingOldName) {
    auto sb = std::make_shared<lpcvm::Sandbox>("/mudlib");
    auto old_object = LpcObject::create(sb, "duplicate-name", std::make_shared<ObjectClass>());
    auto old_ref = old_object.lock();
    ASSERT_TRUE(old_ref.has_value());

    auto new_object = LpcObject::create(sb, "duplicate-name", std::make_shared<ObjectClass>());
    auto new_ref = new_object.lock();
    ASSERT_TRUE(new_ref.has_value());

    EXPECT_EQ((*old_ref)->name()->view(), "duplicate-name");
    EXPECT_EQ((*old_ref)->name(), (*new_ref)->name());
    EXPECT_EQ(sb->find_object("duplicate-name"), new_ref);

    sb->destruct_object(old_object);
    EXPECT_EQ(sb->find_object("duplicate-name"), new_ref);
    sb->destruct_object(new_object);
    EXPECT_FALSE(sb->find_object("duplicate-name"));
}

TEST(LpcObjectTest, RegistrationRejectsInvalidObjectsWithoutChangingTable) {
    auto runtime = std::make_shared<ManagedRuntime>();
    auto other = std::make_shared<ManagedRuntime>();
    auto existing = LpcObject::create(runtime, "std/player", nullptr);
    auto foreign = LpcObject::create(other, "std/player", nullptr);
    auto unnamed = lpcvm::create_shared<LpcObject>(runtime, nullptr);

    EXPECT_THROW(runtime->emplace_object({}), std::invalid_argument);
    EXPECT_THROW(runtime->emplace_object(unnamed), std::invalid_argument);
    EXPECT_THROW(runtime->emplace_object(*foreign.lock()), std::invalid_argument);
    EXPECT_EQ(runtime->find_object("std/player"), *existing.lock());
    EXPECT_EQ(other->find_object("std/player"), *foreign.lock());
    EXPECT_EQ(runtime->objects().size(), 1u);
}

TEST(LpcObjectTest, RegistrationCanReinsertTheSameObject) {
    auto runtime = std::make_shared<ManagedRuntime>();
    auto handle = LpcObject::create(runtime, "std/player", nullptr);
    EXPECT_EQ(runtime->emplace_object(runtime->find_object("std/player")), handle);
    auto retained = *handle.lock();
    runtime->destruct_object(handle);
    EXPECT_EQ(runtime->emplace_object(retained), handle);
    EXPECT_EQ(runtime->find_object("std/player"), retained);
}

TEST(LpcObjectTest, ReplacementKeyUsesNewObjectNameStorage) {
    class UninternedRuntime : public ManagedRuntime {
    public:
        LpcStringRef make_shared_string(std::string_view value) override {
            return allocate_shared_string(value);
        }
    };
    auto runtime = std::make_shared<UninternedRuntime>();
    auto old_handle = LpcObject::create(runtime, "std/player", nullptr);
    auto old_name = runtime->find_object("std/player")->name();
    auto new_handle = LpcObject::create(runtime, "std/player", nullptr);
    auto new_name = runtime->find_object("std/player")->name();
    EXPECT_TRUE(old_handle.expired());
    EXPECT_FALSE(old_name == new_name);
    const auto& table = runtime->objects();
    ASSERT_EQ(table.size(), 1u);
    EXPECT_EQ(table.begin()->first.data(), new_name->data());
    old_name = {};
    EXPECT_EQ(runtime->find_object("std/player"), *new_handle.lock());
}

TEST(LpcObjectTest, NameSharesRuntimeStringAndSurvivesObjectDestruction) {
    auto runtime = std::make_shared<ManagedRuntime>();
    std::string input = "std/player";
    auto handle = LpcObject::create(runtime, input, nullptr);
    input.assign("changed");
    auto name = runtime->find_object("std/player")->name();
    EXPECT_EQ(name, runtime->make_shared_string("std/player"));
    const auto bytes = runtime->shared_string_allocation_size();
    EXPECT_GT(bytes, 0u);

    // The object's reference alone protects both its name and the table key.
    name = {};
    EXPECT_EQ(runtime->sweep_shared_strings(), 0u);
    ASSERT_TRUE(runtime->find_object("std/player"));
    name = runtime->find_object("std/player")->name();
    runtime->destruct_object(handle);
    EXPECT_TRUE(handle.expired());
    EXPECT_FALSE(runtime->find_object("std/player"));
    EXPECT_EQ(runtime->sweep_shared_strings(), 0u);
    EXPECT_EQ(name->view(), "std/player");

    name = {};
    // The expired object handle must not retain the string.
    EXPECT_EQ(runtime->sweep_shared_strings(), bytes);
    EXPECT_EQ(runtime->shared_string_allocation_size(), 0u);
}

TEST(LpcObjectTest, NameSurvivesRuntimeDestruction) {
    LpcStringRef name;
    LpcObjectRef object;
    {
        auto runtime = std::make_shared<ManagedRuntime>();
        auto handle = LpcObject::create(runtime, "std/player", nullptr);
        object = *handle.lock();
        EXPECT_EQ(object->retain_fabric(), runtime);
        name = object->name();
    }
    EXPECT_FALSE(object->retain_fabric());
    EXPECT_EQ(object->name(), name);
    EXPECT_EQ(object->name()->view(), "std/player");
    object = {};
    EXPECT_EQ(name->view(), "std/player");
    EXPECT_EQ(name.use_count(), 1u);
}

TEST(LpcObjectTest, UnnamedObjectHasNullNameAndCanBeDestructed) {
    auto runtime = std::make_shared<ManagedRuntime>();
    auto object = lpcvm::create_shared<LpcObject>(runtime, nullptr);
    EXPECT_FALSE(object->name());
    EXPECT_NO_THROW(runtime->destruct_object(LpcObject::HandleType(object)));
}

TEST(LpcObjectTest, EmptyNameIsRejectedBeforeInterning) {
    auto runtime = std::make_shared<ManagedRuntime>();
    EXPECT_THROW(LpcObject::create(runtime, "", nullptr), std::invalid_argument);
    EXPECT_EQ(runtime->shared_string_allocation_size(), 0u);
}

TEST(LpcObjectTest, NullFabricIsRejected) {
    EXPECT_THROW(LpcObject::create(nullptr, "std/player", nullptr), std::invalid_argument);
}

TEST(LpcObjectTest, RegistrationRejectsObjectWithExpiredFabric) {
    auto runtime = std::make_shared<ManagedRuntime>();
    auto handle = LpcObject::create(runtime, "std/player", nullptr);
    auto object = *handle.lock();
    runtime.reset();
    EXPECT_FALSE(object->retain_fabric());
    auto other = std::make_shared<ManagedRuntime>();
    EXPECT_THROW(other->emplace_object(object), std::invalid_argument);
    EXPECT_TRUE(other->objects().empty());
}

// Thread-confinement contract: counts are plain integers, not atomics.
static_assert(std::is_same_v<decltype(LpcObjectRef::ControlBlock::ref_count), uint32_t>);
static_assert(std::is_same_v<decltype(LpcObjectRef::ControlBlock::weak_count), uint32_t>);
static_assert(sizeof(LpcObjectRef::ControlBlock) == 8);
static_assert(sizeof(LpcObjectRef) == sizeof(void*));
static_assert(sizeof(LpcObject::HandleType) == sizeof(void*));

TEST(LpcSharedPtrTest, CopyAndDestroyTrackStrongCount) {
    auto a = lpcvm::create_shared<LpcObject>(nullptr, std::make_shared<ObjectClass>());
    EXPECT_EQ(a.use_count(), 1u);
    {
        LpcObjectRef b = a;
        EXPECT_TRUE(a == b);
        EXPECT_EQ(a.use_count(), 2u);
        LpcObjectRef c = std::move(b);
        EXPECT_EQ(a.use_count(), 2u);
        EXPECT_EQ(b.use_count(), 0u);
    }
    auto other = lpcvm::create_shared<LpcObject>(nullptr, std::make_shared<ObjectClass>());
    EXPECT_FALSE(a == other);
    EXPECT_TRUE(a == a);

    LpcObjectRef empty_a;
    LpcObjectRef empty_b;
    EXPECT_TRUE(empty_a == empty_b);
    EXPECT_FALSE(a == empty_a);

    EXPECT_EQ(a.use_count(), 1u);
    a = a;
    EXPECT_EQ(a.use_count(), 1u);
}

TEST(LpcSharedPtrTest, ConstructFromLockResultRetainsObject) {
    auto owner = lpcvm::create_shared<int>(42);
    lpcvm::Handle<int, lpcvm::SharedPtr<int>> handle(owner);
    {
        lpcvm::SharedPtr<int> locked = handle.lock();
        EXPECT_EQ(owner.use_count(), 2u);
        owner = {};
        EXPECT_FALSE(handle.expired());
        EXPECT_EQ(locked.use_count(), 1u);
        EXPECT_EQ(*locked, 42);
    }
    EXPECT_TRUE(handle.expired());
    lpcvm::SharedPtr<int> expired = handle.lock();
    EXPECT_FALSE(expired);
    EXPECT_EQ(expired.use_count(), 0u);
}

TEST(LpcSharedPtrTest, ConstructFromOptionalCopiesOrMovesOwnership) {
    auto owner = lpcvm::create_shared<int>(42);
    std::optional<lpcvm::SharedPtr<int>> source(owner);
    {
        lpcvm::SharedPtr<int> copied(source);
        EXPECT_TRUE(*source == owner);
        EXPECT_TRUE(copied == owner);
        EXPECT_EQ(owner.use_count(), 3u);
        lpcvm::SharedPtr<int> moved(std::move(source));
        ASSERT_TRUE(source.has_value());
        EXPECT_FALSE(*source);
        EXPECT_TRUE(moved == owner);
        EXPECT_EQ(owner.use_count(), 3u);
    }
    EXPECT_EQ(owner.use_count(), 1u);

    lpcvm::SharedPtr<int> null_value(source);
    EXPECT_FALSE(null_value);
    EXPECT_EQ(null_value.use_count(), 0u);
    source.reset();
    lpcvm::SharedPtr<int> empty(source);
    EXPECT_FALSE(empty);
    EXPECT_EQ(empty.use_count(), 0u);
}

TEST(LpcHandleTest, CopyAndDestroyTrackWeakCount) {
    auto owner = lpcvm::create_shared<LpcObject>(nullptr, std::make_shared<ObjectClass>());
    auto h = LpcObject::HandleType(owner);
    EXPECT_EQ(h.weak_count(), 1u);
    {
        auto h2 = h;
        EXPECT_EQ(h.weak_count(), 2u);
        auto h3 = std::move(h2);
        EXPECT_EQ(h.weak_count(), 2u);
        h3 = h;
        EXPECT_EQ(h.weak_count(), 2u);
    }
    EXPECT_EQ(h.weak_count(), 1u);
    EXPECT_EQ(h.use_count(), 1u); // weak handles never extend lifetime
}

TEST(LpcHandleTest, LockIncrementsStrongCountAndDefaultHandleIsExpired) {
    LpcObject::HandleType empty;
    EXPECT_TRUE(empty.expired());
    EXPECT_FALSE(empty.lock().has_value());

    auto owner = lpcvm::create_shared<LpcObject>(nullptr, std::make_shared<ObjectClass>());
    auto h = LpcObject::HandleType(owner);
    {
        auto locked = h.lock();
        ASSERT_TRUE(locked.has_value());
        EXPECT_EQ(owner.use_count(), 2u);
    }
    EXPECT_EQ(owner.use_count(), 1u);
}

TEST(LpcHandleTest, HandleHeldAcrossLastStrongReleaseExpiresSafely) {
    auto owner = std::make_optional<LpcObjectRef>(lpcvm::create_shared<LpcObject>(nullptr, std::make_shared<ObjectClass>()));
    auto h = LpcObject::HandleType(*owner);
    owner.reset();
    EXPECT_TRUE(h.expired());
    EXPECT_EQ(h.use_count(), 0u);
    EXPECT_EQ(h.weak_count(), 1u); // only this handle keeps the block alive
}

namespace {
struct Probe {
    Probe(bool* destroyed, std::optional<lpcvm::Handle<Probe, lpcvm::SharedPtr<Probe>>>* self_handle)
        : destroyed_(destroyed), self_handle_(self_handle) {}
    ~Probe() {
        *destroyed_ = true;
        self_handle_->reset(); // handle destroyed re-entrantly during ~Probe must not free the block
    }
    bool* destroyed_;
    std::optional<lpcvm::Handle<Probe, lpcvm::SharedPtr<Probe>>>* self_handle_;
};
}

TEST(LpcSharedPtrTest, HandleReleasedInsideDestructorDoesNotDoubleFree) {
    bool destroyed = false;
    std::optional<lpcvm::Handle<Probe, lpcvm::SharedPtr<Probe>>> self;
    {
        auto p = lpcvm::create_shared<Probe>(&destroyed, &self);
        self.emplace(p);
    }
    EXPECT_TRUE(destroyed);
    EXPECT_FALSE(self.has_value());
}

TEST(LpcSharedPtrTest, DefaultConstructedIsNull) {
    LpcObjectRef p;
    EXPECT_FALSE(p);
    EXPECT_EQ(p.use_count(), 0u);
    LpcArrayRef a;
    EXPECT_FALSE(a);
    auto copy = p;
    EXPECT_FALSE(copy);
}
