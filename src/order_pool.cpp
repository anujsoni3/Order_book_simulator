#include "order_pool.h"
#include <limits>

namespace order_book {

static constexpr std::uint32_t INVALID_INDEX = std::numeric_limits<std::uint32_t>::max();

OrderPool::OrderPool(std::size_t capacity): nodes(capacity), free_head(0), capacity(capacity), size(0) {
    for (std::uint32_t i = 0; i < capacity; ++i) {
        nodes[i].next = (i + 1 < capacity) ? (i + 1) : INVALID_INDEX;
    }
}

// O(1)
Order* OrderPool::allocate() {
    if (free_head == INVALID_INDEX) {
        return nullptr;
    }

    std::uint32_t idx = free_head;
    Node& node = nodes[idx];

    free_head = node.next;
    node.next = INVALID_INDEX;
    ++size;

    return &node.order;
}

// O(1)
void OrderPool::deallocate(Order* order) {
    if (!order) return;

    std::uint32_t idx = get_idx_from_ptr(order);
    Node& node = nodes[idx];

    node.next = free_head;
    free_head = idx;
    
    --size;
}

void OrderPool::reset() {
    free_head = 0;
    size = 0;

    for (std::uint32_t i = 0; i < capacity; ++i) {
        nodes[i].next = (i+1 < capacity) ? (i+1) : INVALID_INDEX;
    }
}

std::uint32_t OrderPool::get_idx_from_ptr(const Order* order) const {
    const char* base = reinterpret_cast<const char*>(nodes.data());
    const char* ptr = reinterpret_cast<const char*>(order);

    std::ptrdiff_t bytes_from_start = ptr - base; 
    std::ptrdiff_t node_size = static_cast<std::ptrdiff_t>(sizeof(Node));

    std::uint32_t idx = static_cast<std::uint32_t>(bytes_from_start / node_size);

    return idx;
}

}