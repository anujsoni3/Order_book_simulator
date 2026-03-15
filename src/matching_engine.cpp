#include "matching_engine.h"

namespace order_book {

MatchingEngine::MatchingEngine(std::size_t max_orders, std::int64_t tick, std::int64_t lot)
    : pool(max_orders), bids(Side::BUY), asks(Side::SELL), lookup(), tick_size(tick), lot_size(lot) {}

std::vector<Trade> MatchingEngine::add_order(const NewOrder& new_order) {
    std::vector<Trade> trades;

    // Validation
    if (new_order.quantity <= 0 || new_order.quantity % lot_size != 0) [[unlikely]] {
        return trades;
    }
    if (new_order.type != OrderType::MARKET && (new_order.price <= 0 || new_order.price % tick_size != 0)) [[unlikely]] {
        return trades;
    }

    OrderBookSide& own_side = (new_order.side == Side::BUY) ? bids : asks;
    OrderBookSide& opp_side = (new_order.side == Side::BUY) ? asks : bids;

    // Post-Only check
    if (new_order.type == OrderType::POST_ONLY && !opp_side.empty()) {
        bool would_cross = (new_order.side == Side::BUY && new_order.price >= opp_side.get_best_price()) ||
                           (new_order.side == Side::SELL && new_order.price <= opp_side.get_best_price());
        if (would_cross) [[unlikely]] {
            return trades;
        }
    }

    // FOK check: Can we fill the whole thing?
    if (new_order.type == OrderType::FOK) {
        std::int64_t available = 0;
        if (new_order.side == Side::BUY) {
            for (auto it = asks.begin(); it != asks.end() && it->first <= new_order.price; ++it) {
                available += it->second.get_total_quantity();
                if (available >= new_order.quantity) break;
            }
        } else {
            for (auto it = bids.begin(); it != bids.end() && it->first >= new_order.price; ++it) {
                available += it->second.get_total_quantity();
                if (available >= new_order.quantity) break;
            }
        }
        if (available < new_order.quantity) [[unlikely]] {
            return trades;
        }
    }

    Order* order = pool.allocate();
    if (!order) [[unlikely]] {
        return trades;
    }

    order->id                 = new_order.id;
    order->side               = new_order.side;
    order->type               = new_order.type;
    order->price              = new_order.price;
    order->quantity           = new_order.quantity;
    order->remaining_quantity = new_order.quantity;
    order->timestamp          = new_order.timestamp;
    order->flags              = new_order.flags;
    order->next = nullptr;
    order->prev = nullptr;

    while (order->remaining_quantity > 0 && !opp_side.empty()) {
        std::int64_t best_price = opp_side.get_best_price();

        // check if prices cross
        bool crosses = (order->type == OrderType::MARKET) ||
                       (order->side == Side::BUY && order->price >= best_price) || 
                       (order->side == Side::SELL && order->price <= best_price);

        if (!crosses) [[likely]] {
            break;
        }

        PriceLevel* level = opp_side.get_best_price_level();
        Order* oldest = level->front();

        std::int64_t oldest_rem = oldest->remaining_quantity;
        std::int64_t qty = (order->remaining_quantity < oldest_rem) ? order->remaining_quantity : oldest_rem;

        Trade t;
        t.taker_id  = order->id;
        t.maker_id  = oldest->id;
        t.price     = oldest->price;
        t.quantity  = qty;
        t.timestamp = new_order.timestamp;
        trades.push_back(t);

        order->remaining_quantity -= qty;

        if (qty == oldest_rem) {
            level->pop_front();
            opp_side.erase_level_if_empty(oldest->price);
            lookup.erase(oldest->id);
            pool.deallocate(oldest);
        } else {
            oldest->remaining_quantity = oldest_rem - qty;
            level->reduce_total_quantity(qty);
        }
    }

    if (order->remaining_quantity > 0) {
        if (order->type == OrderType::LIMIT || order->type == OrderType::POST_ONLY) {
            own_side.insert_order(order);
            OrderHandle handle{ order->side, order };
            lookup.insert(order->id, handle);
        } else {
            // MARKET, IOC, or FOK (if partially filled which shouldn't happen for FOK) are deallocated
            pool.deallocate(order);
        }
    } else {
        pool.deallocate(order);
    }
    
    return trades;
}

bool MatchingEngine::cancel_order(OrderId id) {
    OrderHandle* handle = lookup.find(id);
    if (!handle) return false;

    Order* o = handle->order;
    Side side = handle->side;

    OrderBookSide& book_side = (side == Side::BUY) ? bids : asks;

    PriceLevel* level = book_side.find_price_level(o->price);
    if (!level) {
        lookup.erase(id);
        return false;
    }

    level->remove_order(o);
    book_side.erase_level_if_empty(o->price);
    lookup.erase(id);
    pool.deallocate(o);

    return true;
}

std::vector<Trade> MatchingEngine::modify_order(OrderId id, const NewOrder& new_order) {
    OrderHandle* handle = lookup.find(id);
    if (!handle) return {};

    Order* o = handle->order;
    Side side = handle->side;

    OrderBookSide& book_side = (side == Side::BUY) ? bids : asks;

    PriceLevel* level = book_side.find_price_level(o->price);
    if (level) {
        level->remove_order(o);
        book_side.erase_level_if_empty(o->price);
    }

    lookup.erase(id);
    pool.deallocate(o);

    NewOrder replaced = new_order;
    replaced.id = id;

    return add_order(replaced);
}

}
