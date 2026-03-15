// include/order.h
// declarations of order structs

#pragma once

#include <cstdint>

namespace order_book {

enum class Side : std::uint8_t {
    BUY,
    SELL
};

enum class OrderType : std::uint8_t {
    LIMIT,
    MARKET,
    IOC,
    FOK,
    POST_ONLY
};

using OrderId = std::uint64_t;

// The next/prev pointers are used by PriceLevel to chain orders together
// without extra list node allocations. This is more cache-friendly and
// memory-efficient than using std::list<Order> for a hot order queue.
struct alignas(64) Order {
    Order* next;
    Order* prev;

    OrderId id;
    Side side;
    OrderType type;
    std::int64_t price;
    std::int64_t quantity;
    std::int64_t remaining_quantity;
    std::int64_t timestamp;
    std::int64_t flags;
};

struct Trade {
    OrderId taker_id;
    OrderId maker_id;
    std::int64_t price;
    std::int64_t quantity;
    std::uint64_t timestamp;
};

}
