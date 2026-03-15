#include "price_level.h"

namespace order_book {

void PriceLevel::add_order_back(Order* order) {
    if (!order) {
        return;
    }

    order->next = nullptr;
    order->prev = tail;

    if (tail) {
        tail->next = order;
    } else {
        head = order;
    }

    tail = order;
    total_quantity += order->remaining_quantity;
}
    
Order* PriceLevel::front() { return head; }

const Order* PriceLevel::front() const { return head; }

void PriceLevel::pop_front() {
    if (!head) {
        return;
    }

    Order* tmp = head;
    total_quantity -= head->remaining_quantity;

    head = head->next;

    if (head) {
        head->prev = nullptr;
    } else {
        tail = nullptr;
    }

    tmp->next = nullptr;
    tmp->prev = nullptr;
}

void PriceLevel::remove_order(Order* order) {
    if (!order) return;

    total_quantity -= order->remaining_quantity;

    Order* prev = order->prev;
    Order* next = order->next;

    if (prev)
        prev->next = next;
    else
        head = next;

    if (next)
        next->prev = prev;
    else
        tail = prev;

    order->prev = nullptr;
    order->next = nullptr;
}

}