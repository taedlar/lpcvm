// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "fabric.hpp"
#include <algorithm>

LpcContainer::~LpcContainer() {
    if (!gc_owner_)
        return;
    if (gc_prev)
        gc_prev->gc_next = gc_next;
    else
        gc_owner_->gc_head_ = gc_next;
    if (gc_next)
        gc_next->gc_prev = gc_prev;
}

std::size_t ManagedRuntime::collect_garbage() {
    std::vector<LpcContainer*> containers;
    for (auto* container = gc_head_; container; container = container->gc_next)
        containers.push_back(container);

    // Allocate all scratch space before changing reference counts or edges.
    std::vector<LpcContainer*> reachable;
    reachable.reserve(containers.size());
    for (auto* container : containers) {
        container->gc_external_refs_ = *container->gc_ref_count_;
        container->gc_marked_ = false;
    }

    for (auto* container : containers) {
        container->visit_edges([](LpcContainer* child, void* context) {
            if (child->gc_owner_ == context)
                --child->gc_external_refs_;
        }, this);
    }

    for (auto* container : containers) {
        if (container->gc_external_refs_ != 0) {
            container->gc_marked_ = true;
            reachable.push_back(container);
        }
    }
    // Every node enters the worklist at most once; traversal is iterative.
    auto traversal = std::make_pair(this, &reachable);
    for (std::size_t i = 0; i < reachable.size(); ++i) {
        reachable[i]->visit_edges([](LpcContainer* child, void* context) {
            auto& state = *static_cast<decltype(traversal)*>(context);
            if (child->gc_owner_ == state.first && !child->gc_marked_) {
                child->gc_marked_ = true;
                state.second->push_back(child);
            }
        }, &traversal);
    }

    containers.erase(std::remove_if(containers.begin(), containers.end(),
        [](LpcContainer* container) { return container->gc_marked_; }), containers.end());

    // Pin all garbage before breaking any edge: releasing one cycle must not
    // destroy a node that we still need to visit. Ordinary SharedPtr teardown
    // then preserves weak handles and each concrete type's destructor.
    for (auto* container : containers)
        ++*container->gc_ref_count_;
    for (auto* container : containers)
        container->clear_edges();
    for (auto* container : containers)
        container->gc_release_(container);
    return containers.size();
}
