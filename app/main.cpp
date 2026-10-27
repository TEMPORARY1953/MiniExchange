#include "mini_exchange/order_book.hpp"

#include <iostream>

namespace {

const char* side_name(mini_exchange::Side side) {
    return side == mini_exchange::Side::Buy ? "BUY" : "SELL";
}

void print_book(const mini_exchange::OrderBook& book) {
    std::cout << "Remaining orders (best price, then FIFO):\n";
    for (const auto side : {mini_exchange::Side::Buy, mini_exchange::Side::Sell}) {
        std::cout << side_name(side) << ":\n";
        const auto orders = book.orders(side);
        if (orders.empty()) {
            std::cout << "  (empty)\n";
        }
        for (const auto& order : orders) {
            std::cout << "  #" << order.id << " price=" << order.price
                      << " quantity=" << order.quantity << '\n';
        }
    }
}

} // namespace

int main() {
    using namespace mini_exchange;
    OrderBook book;
    // #1 and #2 demonstrate FIFO; #3 demonstrates another price level.
    const Order input[] = {
        {1, Side::Sell, 100, 5},
        {2, Side::Sell, 100, 4},
        {3, Side::Sell, 101, 6},
        {4, Side::Buy, 101, 12},
        {5, Side::Buy, 99, 7},
        {6, Side::Sell, 98, 2},
    };
    std::cout << "Mini Exchange: prices in ticks, quantities in units\n";
    for (const auto& order : input) {
        std::cout << "ADD #" << order.id << ' ' << side_name(order.side)
                  << " price=" << order.price << " quantity=" << order.quantity << '\n';
        for (const auto& trade : book.add_order(order)) {
            std::cout << "  TRADE buy=#" << trade.buy_order_id
                      << " sell=#" << trade.sell_order_id << " price=" << trade.price
                      << " quantity=" << trade.quantity << '\n';
        }
    }
    print_book(book);
}
