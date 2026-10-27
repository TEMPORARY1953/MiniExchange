#pragma once

#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <unordered_set>
#include <vector>

/** @file order_book.hpp
 *  @brief Public data types and the single-instrument matching engine.
 */

/** @brief Independent matching library; no input/output or networking. */
namespace mini_exchange {

using OrderId = std::uint64_t; ///< Positive identifier, unique for a book's lifetime.
using Price = std::int64_t;    ///< Positive price in integer ticks; no floating point.
using Quantity = std::int64_t; ///< Positive number of indivisible units.

/** @brief Direction of a limit order. */
enum class Side {
    Buy, ///< Buy at or below the limit price.
    Sell ///< Sell at or above the limit price.
};

/** @brief A limit order; in a snapshot quantity is its unfilled remainder. */
struct Order {
    OrderId id;         ///< Caller-assigned identifier, greater than zero.
    Side side;          ///< Buy or sell.
    Price price;        ///< Maximum buy price or minimum sell price, in ticks.
    Quantity quantity;  ///< Requested or remaining units.
};

/** @brief One execution against one resting order. */
struct Trade {
    OrderId buy_order_id;  ///< Identifier of the buying order.
    OrderId sell_order_id; ///< Identifier of the selling order.
    Price price;          ///< Execution price: the resting order's limit price.
    Quantity quantity;    ///< Executed units, always positive.
};

/** @brief A single-threaded, single-instrument limit order book.
 *
 * Bids are matched highest price first; asks lowest price first. Orders at
 * the same price execute in add_order() call order (FIFO). An unfilled
 * remainder is kept at its own limit price. No wall clock is needed.
 * All accepted IDs remain reserved, even after full execution.
 * The caller must serialize access to an instance.
 */
class OrderBook {
public:
    /** @brief Match an incoming order immediately and store its remainder.
     * @param order Positive ID, valid side, positive price and quantity.
     * @return Executions in matching order; an empty vector means no trade.
     * @throws std::invalid_argument If any field is invalid or ID was used.
     * Validation errors leave the book unchanged and do not reserve the ID.
     */
    [[nodiscard]] std::vector<Trade> add_order(Order order);

    /** @brief Return a copy of resting orders in execution priority order.
     * @param side Side of the book to inspect.
     * @return Best price first, FIFO within a price; an empty vector if empty.
     * @throws std::invalid_argument If side is neither Buy nor Sell.
     * Modifying this snapshot does not change the book.
     */
    [[nodiscard]] std::vector<Order> orders(Side side) const;

private:
    std::map<Price, std::deque<Order>, std::greater<Price>> bids_;
    std::map<Price, std::deque<Order>> asks_;
    std::unordered_set<OrderId> used_ids_;
};

} // namespace mini_exchange
