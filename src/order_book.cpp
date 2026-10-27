#include "mini_exchange/order_book.hpp"

#include <algorithm>
#include <stdexcept>

namespace mini_exchange {
namespace {

// Both sides use the same matching loop; the map determines price priority.
template <typename PriceLevels>
void match(Order& incoming, PriceLevels& opposite, std::vector<Trade>& trades) {
    while (incoming.quantity > 0 && !opposite.empty()) {
        auto level = opposite.begin();
        const Price resting_price = level->first;
        const bool crosses = incoming.side == Side::Buy
            ? incoming.price >= resting_price
            : incoming.price <= resting_price;
        if (!crosses) {
            break;
        }

        auto& queue = level->second;
        Order& resting = queue.front();
        const Quantity executed = std::min(incoming.quantity, resting.quantity);
        const OrderId buy_id = incoming.side == Side::Buy ? incoming.id : resting.id;
        const OrderId sell_id = incoming.side == Side::Sell ? incoming.id : resting.id;
        trades.push_back({buy_id, sell_id, resting_price, executed});
        incoming.quantity -= executed;
        resting.quantity -= executed;

        if (resting.quantity == 0) {
            queue.pop_front();
        }
        if (queue.empty()) {
            opposite.erase(level);
        }
    }
}

template <typename PriceLevels>
std::vector<Order> snapshot(const PriceLevels& levels) {
    std::vector<Order> result;
    for (const auto& level : levels) {
        result.insert(result.end(), level.second.begin(), level.second.end());
    }
    return result;
}

} // namespace

std::vector<Trade> OrderBook::add_order(Order order) {
    if (order.id == 0 || (order.side != Side::Buy && order.side != Side::Sell)
        || order.price <= 0 || order.quantity <= 0) {
        throw std::invalid_argument("Order requires a positive ID, price, quantity and a valid side");
    }
    if (!used_ids_.insert(order.id).second) {
        throw std::invalid_argument("Order ID has already been used in this book");
    }

    std::vector<Trade> trades;
    if (order.side == Side::Buy) {
        match(order, asks_, trades);
        if (order.quantity > 0) {
            bids_[order.price].push_back(order);
        }
    } else {
        match(order, bids_, trades);
        if (order.quantity > 0) {
            asks_[order.price].push_back(order);
        }
    }
    return trades;
}

std::vector<Order> OrderBook::orders(Side side) const {
    if (side == Side::Buy) {
        return snapshot(bids_);
    }
    if (side == Side::Sell) {
        return snapshot(asks_);
    }
    throw std::invalid_argument("Unknown order side");
}

} // namespace mini_exchange
