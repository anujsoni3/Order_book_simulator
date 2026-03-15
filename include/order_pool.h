#pragma once

#include <vector>
#include <cstddef>
#include <cstdint>
#include "order.h"

namespace order_book {

// fixed-capacity memory pool for Order objects.
// all orders are stored in a contiguous block.
// this reduces heap allocations, improves cache locality, and is more predictable
class OrderPool {
private:
    struct Node {
        Order order;
        std::uint32_t next; // idx of next free node
    };

    std::vector<Node> nodes;
    std::uint32_t free_head; // index of first free node
    std::size_t capacity; // total nodes
    std::size_t size; // n of currently allocated nodes

    std::uint32_t get_idx_from_ptr(const Order* order) const; // helper to get index from private, needed for freeing
public:
    explicit OrderPool(std::size_t capacity);
    Order* allocate(); // allocate a new order in the pool
    void deallocate(Order* order); // deallocate a specific order
    void reset();
    std::size_t get_capacity() const { return capacity; }
    std::size_t get_size() const { return size; }
};

}