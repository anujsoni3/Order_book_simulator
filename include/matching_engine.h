#pragma once

#include "order.h"
#include "order_pool.h"
#include "price_level.h"
#include "order_lookup.h"
#include "order_book_side.h"
#include <cstdint>

namespace order_book {

struct NewOrder {
    OrderId id;
    Side side;
    OrderType type;
    std::int64_t price;
    std::int64_t quantity;
    std::uint64_t timestamp;
    std::int64_t flags;
};

class MatchingEngine {
private:
    OrderPool pool;
    OrderBookSide bids;
    OrderBookSide asks;
    OrderLookup lookup;

    std::int64_t tick_size{1};
    std::int64_t lot_size{1};

public:
    explicit MatchingEngine(std::size_t max_orders, std::int64_t tick = 1, std::int64_t lot = 1);
    // create and store a new order in the engine: allocate an Order from the pool, fill its fields,
        // insert it into the correct OrderBookSide and PriceLevel, register it in OrderLookup for O(1) cancel/modify_order
    // try to match it immediately against resting orders on the opposite side
    std::vector<Trade> add_order(const NewOrder& new_order);
    bool cancel_order(OrderId id);
    std::vector<Trade> modify_order(OrderId id, const NewOrder& new_order);

    const OrderBookSide& get_bids() const { return bids; }
    const OrderBookSide& get_asks() const { return asks; }
};

}
