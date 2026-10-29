// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include <gtest/gtest.h>
#include <memory>
#include <optional>
#include <type_traits>
#include "lpc_types.hpp"
#include "object_class.hpp"
#include "vm_sandbox.hpp"

TEST(LpcObjectHandleTest, LockRetainsObjectUntilStrongReferenceIsReleased) {
    LpcObject::HandleType handle;
    std::optional<LpcObjectRef> locked;
    {
        auto owner = lpcvm::make_shared<LpcObject>(std::make_shared<ObjectClass>());
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
    auto owner = std::make_optional<LpcObjectRef>(lpcvm::make_shared<LpcObject>(std::make_shared<ObjectClass>()));
    auto h1 = LpcObject::HandleType(*owner);
    auto h2 = h1;
    owner.reset();
    EXPECT_FALSE(h1.lock().has_value());
    h1 = LpcObject::HandleType{};
    EXPECT_TRUE(h2.expired());
}

TEST(LpcObjectTest, CreatingDuplicateNameReplacesWithoutInvalidatingOldName) {
    lpcvm::Sandbox sb ("/mudlib");
    auto old_object = LpcObject::create(&sb, "duplicate-name", std::make_shared<ObjectClass>());
    auto old_ref = old_object.lock();
    ASSERT_TRUE(old_ref.has_value());

    auto new_object = LpcObject::create(&sb, "duplicate-name", std::make_shared<ObjectClass>());
    auto new_ref = new_object.lock();
    ASSERT_TRUE(new_ref.has_value());

    EXPECT_EQ((*old_ref)->name(), "duplicate-name");
    EXPECT_EQ(LpcObject::find(&sb, "duplicate-name"), *new_ref);

    LpcObject::destruct(&sb, *old_ref);
    EXPECT_EQ(LpcObject::find(&sb, "duplicate-name"), *new_ref);
    LpcObject::destruct(&sb, *new_ref);
    EXPECT_FALSE(LpcObject::find(&sb, "duplicate-name"));
}

// Thread-confinement contract: counts are plain integers, not atomics.
static_assert(std::is_same_v<decltype(LpcObjectRef::ControlBlock::ref_count), uint32_t>);
static_assert(std::is_same_v<decltype(LpcObjectRef::ControlBlock::weak_count), uint32_t>);
static_assert(sizeof(LpcObjectRef::ControlBlock) == 8);
static_assert(sizeof(LpcObjectRef) == sizeof(void*));
static_assert(sizeof(LpcObject::HandleType) == sizeof(void*));

TEST(LpcSharedPtrTest, CopyAndDestroyTrackStrongCount) {
    auto a = lpcvm::make_shared<LpcObject>(std::make_shared<ObjectClass>());
    EXPECT_EQ(a.use_count(), 1u);
    {
        LpcObjectRef b = a;
        EXPECT_TRUE(a == b);
        EXPECT_EQ(a.use_count(), 2u);
        LpcObjectRef c = std::move(b);
        EXPECT_EQ(a.use_count(), 2u);
        EXPECT_EQ(b.use_count(), 0u);
    }
    auto other = lpcvm::make_shared<LpcObject>(std::make_shared<ObjectClass>());
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

TEST(LpcHandleTest, CopyAndDestroyTrackWeakCount) {
    auto owner = lpcvm::make_shared<LpcObject>(std::make_shared<ObjectClass>());
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

    auto owner = lpcvm::make_shared<LpcObject>(std::make_shared<ObjectClass>());
    auto h = LpcObject::HandleType(owner);
    {
        auto locked = h.lock();
        ASSERT_TRUE(locked.has_value());
        EXPECT_EQ(owner.use_count(), 2u);
    }
    EXPECT_EQ(owner.use_count(), 1u);
}

TEST(LpcHandleTest, HandleHeldAcrossLastStrongReleaseExpiresSafely) {
    auto owner = std::make_optional<LpcObjectRef>(lpcvm::make_shared<LpcObject>(std::make_shared<ObjectClass>()));
    auto h = LpcObject::HandleType(*owner);
    owner.reset();
    EXPECT_TRUE(h.expired());
    EXPECT_EQ(h.use_count(), 0u);
    EXPECT_EQ(h.weak_count(), 1u); // only this handle keeps the block alive
}

namespace {
struct Probe {
    Probe(bool* destroyed, std::optional<LpcHandle<Probe, LpcSharedPtr<Probe>>>* self_handle)
        : destroyed_(destroyed), self_handle_(self_handle) {}
    ~Probe() {
        *destroyed_ = true;
        self_handle_->reset(); // handle destroyed re-entrantly during ~Probe must not free the block
    }
    bool* destroyed_;
    std::optional<LpcHandle<Probe, LpcSharedPtr<Probe>>>* self_handle_;
};
}

TEST(LpcSharedPtrTest, HandleReleasedInsideDestructorDoesNotDoubleFree) {
    bool destroyed = false;
    std::optional<LpcHandle<Probe, LpcSharedPtr<Probe>>> self;
    {
        auto p = lpcvm::make_shared<Probe>(&destroyed, &self);
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
