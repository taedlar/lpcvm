// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include <gtest/gtest.h>
#include <memory>
#include <optional>
#include <type_traits>
#include "lpc_types.hpp"

TEST(LpcObjectHandleTest, LockRetainsObjectUntilStrongReferenceIsReleased) {
    LpcObject::HandleType handle;
    std::optional<LpcObjectRef> locked;
    {
        auto owner = lpc_make_shared<LpcObject>(std::make_shared<RuntimeClass>());
        handle = owner->handle();
        locked = handle.lock();
        ASSERT_TRUE(locked.has_value());
        EXPECT_EQ((*locked)->handle().ptr, handle.ptr);
    }

    EXPECT_TRUE(handle.lock().has_value());
    locked.reset();
    EXPECT_TRUE(handle.expired());
    EXPECT_FALSE(handle.lock().has_value());
}

TEST(LpcObjectHandleTest, HandleOutlivesObject) {
    auto owner = std::make_optional<LpcObjectRef>(lpc_make_shared<LpcObject>(std::make_shared<RuntimeClass>()));
    auto h1 = (*owner)->handle();
    auto h2 = h1;
    owner.reset();
    EXPECT_FALSE(h1.lock().has_value());
    h1 = LpcObject::HandleType{};
    EXPECT_TRUE(h2.expired());
}

// Thread-confinement contract: counts are plain integers, not atomics.
static_assert(std::is_same_v<decltype(LpcObjectRef::ControlBlock::ref_count), uint32_t>);
static_assert(std::is_same_v<decltype(LpcObjectRef::ControlBlock::weak_count), uint32_t>);
static_assert(sizeof(LpcObjectRef::ControlBlock) == 8);
static_assert(sizeof(LpcObjectRef) == sizeof(void*));
static_assert(sizeof(LpcObject::HandleType) == sizeof(void*));

TEST(LpcSharedPtrTest, CopyAndDestroyTrackStrongCount) {
    auto a = lpc_make_shared<LpcObject>(std::make_shared<RuntimeClass>());
    EXPECT_EQ(a.use_count(), 1u);
    {
        LpcObjectRef b = a;
        EXPECT_EQ(a.use_count(), 2u);
        LpcObjectRef c = std::move(b);
        EXPECT_EQ(a.use_count(), 2u);
        EXPECT_EQ(b.use_count(), 0u);
    }
    EXPECT_EQ(a.use_count(), 1u);
    a = a;
    EXPECT_EQ(a.use_count(), 1u);
}

TEST(LpcHandleTest, CopyAndDestroyTrackWeakCount) {
    auto owner = lpc_make_shared<LpcObject>(std::make_shared<RuntimeClass>());
    auto h = owner->handle();
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

    auto owner = lpc_make_shared<LpcObject>(std::make_shared<RuntimeClass>());
    auto h = owner->handle();
    {
        auto locked = h.lock();
        ASSERT_TRUE(locked.has_value());
        EXPECT_EQ(owner.use_count(), 2u);
    }
    EXPECT_EQ(owner.use_count(), 1u);
}

TEST(LpcHandleTest, HandleHeldAcrossLastStrongReleaseExpiresSafely) {
    auto owner = std::make_optional<LpcObjectRef>(lpc_make_shared<LpcObject>(std::make_shared<RuntimeClass>()));
    auto h = (*owner)->handle();
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
        auto p = lpc_make_shared<Probe>(&destroyed, &self);
        self.emplace(p.ptr);
    }
    EXPECT_TRUE(destroyed);
    EXPECT_FALSE(self.has_value());
}

TEST(LpcSharedPtrTest, DefaultConstructedIsNull) {
    LpcObjectRef p;
    EXPECT_EQ(p.ptr, nullptr);
    EXPECT_EQ(p.use_count(), 0u);
    LpcArrayRef a;
    EXPECT_EQ(a.ptr, nullptr);
    auto copy = p;
    EXPECT_EQ(copy.ptr, nullptr);
}
