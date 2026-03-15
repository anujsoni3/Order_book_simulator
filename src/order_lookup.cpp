#include "order_lookup.h"

namespace order_book {

void OrderLookup::insert(OrderId id, const OrderHandle& handle) {
    table[id] = handle;
}

const OrderHandle* OrderLookup::find(OrderId id) const {
    auto it = table.find(id);
    if (it == table.end()) {
        return nullptr;
    }
    return &it->second;
}

OrderHandle* OrderLookup::find(OrderId id) {
    auto it = table.find(id);
    if (it == table.end()) {
        return nullptr;
    }
    return &it->second;
}

void OrderLookup::erase(OrderId id) {
    table.erase(id);
}

void OrderLookup::clear() {
    table.clear();
}

}