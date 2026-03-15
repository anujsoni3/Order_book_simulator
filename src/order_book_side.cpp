#include "order_book_side.h"

namespace order_book {

OrderBookSide::OrderBookSide(Side side): side(side) {}

bool OrderBookSide::empty() const { return levels.empty(); }

void OrderBookSide::insert_order(Order* order) {
    if (!order) return;

    PriceLevel& level = levels[order->price];
    level.add_order_back(order);
}

std::int64_t OrderBookSide::get_best_price() const {
    if (levels.empty()) return 0;

    if (side == Side::BUY) {
        return levels.rbegin()->first;
    } else {
        return levels.begin()->first;
    }
}

PriceLevel* OrderBookSide::get_best_price_level() {
    if (levels.empty()) return nullptr;

    if (side == Side::BUY) {
        return &levels.rbegin()->second;
    } else {
        return &levels.begin()->second;
    }
}
const PriceLevel* OrderBookSide::get_best_price_level() const {
    if (levels.empty()) {
        return nullptr;
    }

    if (side == Side::BUY) {
        return &levels.rbegin()->second;
    } else {
        return &levels.begin()->second;
    }
}

PriceLevel* OrderBookSide::find_price_level(std::int64_t price) {
    auto it = levels.find(price);

    if (it == levels.end()) {
        return nullptr;
    }

    return &it->second;
}

void OrderBookSide::erase_level_if_empty(std::int64_t price) {
    auto it = levels.find(price);
    if (it == levels.end()) {
        return;
    }

    if (it->second.empty()) {
        levels.erase(it);
    }
}

}
