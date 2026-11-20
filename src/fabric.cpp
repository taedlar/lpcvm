// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include "fabric.hpp"
#include <algorithm>

ManagedRuntime::~ManagedRuntime() {
    // External container references can outlive the runtime.
    while (gc_head_) {
        auto* container = gc_head_;
        gc_head_ = container->gc_next;
        container->gc_owner_ = nullptr;
        container->fabric_ = nullptr;
        container->gc_prev = nullptr;
        container->gc_next = nullptr;
    }
    if (buffer_pool_)
        buffer_pool_->close();
}

std::size_t ManagedRuntime::collect_garbage() {
    std::vector<LpcContainer*> containers;
    for (auto* container = gc_head_; container; container = container->gc_next)
        containers.push_back(container);

    // Allocate all scratch space before changing reference counts or edges.
    struct CollectionState {
        uint32_t* ref_count;
        uint32_t external_refs;
        bool marked = false;
    };
    std::unordered_map<LpcContainer*, CollectionState> states;
    states.reserve(containers.size());
    std::vector<LpcContainer*> reachable;
    reachable.reserve(containers.size());
    for (auto* container : containers) {
        auto& ref_count = container->gc_ref_count();
        states.emplace(container, CollectionState{&ref_count, ref_count});
    }

    for (auto* container : containers) {
        container->visit_edges([](LpcContainer* child, void* context) {
            auto& collection = *static_cast<decltype(states)*>(context);
            if (auto it = collection.find(child); it != collection.end())
                --it->second.external_refs;
        }, &states);
    }

    for (auto* container : containers) {
        auto& state = states.find(container)->second;
        if (state.external_refs != 0) {
            state.marked = true;
            reachable.push_back(container);
        }
    }
    // Every node enters the worklist at most once; traversal is iterative.
    auto traversal = std::make_pair(&states, &reachable);
    for (std::size_t i = 0; i < reachable.size(); ++i) {
        reachable[i]->visit_edges([](LpcContainer* child, void* context) {
            auto& state = *static_cast<decltype(traversal)*>(context);
            auto it = state.first->find(child);
            if (it != state.first->end() && !it->second.marked) {
                it->second.marked = true;
                state.second->push_back(child);
            }
        }, &traversal);
    }

    containers.erase(std::remove_if(containers.begin(), containers.end(),
        [&states](LpcContainer* container) { return states.find(container)->second.marked; }),
        containers.end());

    // Pin all garbage before breaking any edge: releasing one cycle must not
    // destroy a node that we still need to visit. Ordinary SharedPtr teardown
    // then preserves weak handles and each concrete type's destructor.
    for (auto* container : containers)
        ++*states.find(container)->second.ref_count;
    for (auto* container : containers)
        container->clear_edges();
    for (auto* container : containers)
        container->release_gc_reference();
    return containers.size();
}
