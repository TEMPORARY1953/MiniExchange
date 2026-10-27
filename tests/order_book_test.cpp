#include "mini_exchange/order_book.hpp"

#include <gtest/gtest.h>

#include <limits>
#include <stdexcept>

namespace mini_exchange {
namespace {

void add(OrderBook& book, Order order) {
    EXPECT_TRUE(book.add_order(order).empty());
}

void expect_order(const Order& actual, OrderId id, Side side, Price price, Quantity quantity) {
    EXPECT_EQ(actual.id, id);
    EXPECT_EQ(actual.side, side);
    EXPECT_EQ(actual.price, price);
    EXPECT_EQ(actual.quantity, quantity);
}

void expect_trade(const Trade& actual, OrderId buy, OrderId sell, Price price, Quantity quantity) {
    EXPECT_EQ(actual.buy_order_id, buy);
    EXPECT_EQ(actual.sell_order_id, sell);
    EXPECT_EQ(actual.price, price);
    EXPECT_EQ(actual.quantity, quantity);
}

TEST(OrderBookTest, StartsEmpty) {
    const OrderBook book;
    EXPECT_TRUE(book.orders(Side::Buy).empty());
    EXPECT_TRUE(book.orders(Side::Sell).empty());
}

TEST(OrderBookTest, NonCrossingOrdersRestOnBothSides) {
    OrderBook book;
    add(book, {1, Side::Buy, 99, 3});
    add(book, {2, Side::Sell, 100, 4});
    const auto buys = book.orders(Side::Buy);
    const auto sells = book.orders(Side::Sell);
    ASSERT_EQ(buys.size(), 1U);
    ASSERT_EQ(sells.size(), 1U);
    expect_order(buys[0], 1, Side::Buy, 99, 3);
    expect_order(sells[0], 2, Side::Sell, 100, 4);
}

class MatchingTest : public ::testing::TestWithParam<Side> {
protected:
    Side incoming_side() const { return GetParam(); }
    Side resting_side() const { return GetParam() == Side::Buy ? Side::Sell : Side::Buy; }
    Price worse_price() const { return GetParam() == Side::Buy ? 101 : 99; }
    void check(const Trade& trade, OrderId incoming, OrderId resting, Price price, Quantity quantity) {
        expect_trade(trade,
                     incoming_side() == Side::Buy ? incoming : resting,
                     incoming_side() == Side::Sell ? incoming : resting,
                     price, quantity);
    }
    OrderBook book;
};

TEST_P(MatchingTest, EqualPriceFullyExecutesBothOrders) {
    add(book, {1, resting_side(), 100, 5});
    const auto trades = book.add_order({2, incoming_side(), 100, 5});
    ASSERT_EQ(trades.size(), 1U);
    check(trades[0], 2, 1, 100, 5);
    EXPECT_TRUE(book.orders(Side::Buy).empty());
    EXPECT_TRUE(book.orders(Side::Sell).empty());
}

TEST_P(MatchingTest, UsesRestingPriceWhenLimitsDiffer) {
    add(book, {1, resting_side(), 100, 5});
    const auto trades = book.add_order({2, incoming_side(), worse_price(), 5});
    ASSERT_EQ(trades.size(), 1U);
    check(trades[0], 2, 1, 100, 5);
}

TEST_P(MatchingTest, BetterPriceHasPriorityOverEarlierArrival) {
    add(book, {1, resting_side(), worse_price(), 3});
    add(book, {2, resting_side(), 100, 4});
    const auto trades = book.add_order({3, incoming_side(), worse_price(), 5});
    ASSERT_EQ(trades.size(), 2U);
    check(trades[0], 3, 2, 100, 4);
    check(trades[1], 3, 1, worse_price(), 1);
    const auto remaining = book.orders(resting_side());
    ASSERT_EQ(remaining.size(), 1U);
    expect_order(remaining[0], 1, resting_side(), worse_price(), 2);
}

TEST_P(MatchingTest, EqualPriceUsesArrivalOrderRatherThanId) {
    add(book, {90, resting_side(), 100, 3});
    add(book, {10, resting_side(), 100, 4});
    const auto trades = book.add_order({50, incoming_side(), 100, 5});
    ASSERT_EQ(trades.size(), 2U);
    check(trades[0], 50, 90, 100, 3);
    check(trades[1], 50, 10, 100, 2);
}

TEST_P(MatchingTest, PartialRestingOrderKeepsFifoPosition) {
    add(book, {1, resting_side(), 100, 5});
    add(book, {2, resting_side(), 100, 4});
    const auto first = book.add_order({3, incoming_side(), 100, 2});
    ASSERT_EQ(first.size(), 1U);
    check(first[0], 3, 1, 100, 2);
    const auto second = book.add_order({4, incoming_side(), 100, 4});
    ASSERT_EQ(second.size(), 2U);
    check(second[0], 4, 1, 100, 3);
    check(second[1], 4, 2, 100, 1);
}

TEST_P(MatchingTest, IncomingRemainderRestsAtItsOwnLimit) {
    add(book, {1, resting_side(), 100, 2});
    const auto trades = book.add_order({2, incoming_side(), worse_price(), 7});
    ASSERT_EQ(trades.size(), 1U);
    check(trades[0], 2, 1, 100, 2);
    const auto remaining = book.orders(incoming_side());
    ASSERT_EQ(remaining.size(), 1U);
    expect_order(remaining[0], 2, incoming_side(), worse_price(), 5);
    EXPECT_TRUE(book.orders(resting_side()).empty());
}

TEST_P(MatchingTest, StopsBeforePriceOutsideLimit) {
    add(book, {1, resting_side(), 100, 2});
    add(book, {2, resting_side(), worse_price(), 4});
    const auto trades = book.add_order({3, incoming_side(), 100, 7});
    ASSERT_EQ(trades.size(), 1U);
    check(trades[0], 3, 1, 100, 2);
    const auto incoming = book.orders(incoming_side());
    const auto resting = book.orders(resting_side());
    ASSERT_EQ(incoming.size(), 1U);
    ASSERT_EQ(resting.size(), 1U);
    expect_order(incoming[0], 3, incoming_side(), 100, 5);
    expect_order(resting[0], 2, resting_side(), worse_price(), 4);
}

INSTANTIATE_TEST_SUITE_P(BothSides, MatchingTest, ::testing::Values(Side::Buy, Side::Sell));

TEST(OrderBookTest, SnapshotHasBestPriceThenFifoOrderAndIsIndependent) {
    OrderBook book;
    add(book, {1, Side::Buy, 98, 1});
    add(book, {2, Side::Buy, 99, 2});
    add(book, {3, Side::Buy, 99, 3});
    add(book, {4, Side::Sell, 102, 4});
    add(book, {5, Side::Sell, 101, 5});
    add(book, {6, Side::Sell, 101, 6});
    auto buys = book.orders(Side::Buy);
    const auto sells = book.orders(Side::Sell);
    ASSERT_EQ(buys.size(), 3U);
    ASSERT_EQ(sells.size(), 3U);
    EXPECT_EQ(buys[0].id, 2U);
    EXPECT_EQ(buys[1].id, 3U);
    EXPECT_EQ(buys[2].id, 1U);
    EXPECT_EQ(sells[0].id, 5U);
    EXPECT_EQ(sells[1].id, 6U);
    EXPECT_EQ(sells[2].id, 4U);
    buys[0].quantity = 999;
    EXPECT_EQ(book.orders(Side::Buy)[0].quantity, 2);
}

TEST(OrderBookTest, RejectsInvalidFieldsWithoutChangingBookOrReservingId) {
    OrderBook book;
    add(book, {1, Side::Sell, 100, 3});
    const Order invalid[] = {
        {0, Side::Buy, 100, 2}, {2, Side::Buy, 0, 2},
        {2, Side::Buy, -1, 2}, {2, Side::Buy, 100, 0},
        {2, Side::Buy, 100, -1}, {2, static_cast<Side>(42), 100, 2},
    };
    for (const auto& order : invalid) {
        EXPECT_THROW((void)book.add_order(order), std::invalid_argument);
        EXPECT_TRUE(book.orders(Side::Buy).empty());
        const auto sells = book.orders(Side::Sell);
        ASSERT_EQ(sells.size(), 1U);
        expect_order(sells[0], 1, Side::Sell, 100, 3);
    }
    const auto trades = book.add_order({2, Side::Buy, 100, 2});
    ASSERT_EQ(trades.size(), 1U);
    expect_trade(trades[0], 2, 1, 100, 2);
}

TEST(OrderBookTest, RejectsDuplicateActiveIdBeforeMatching) {
    OrderBook book;
    add(book, {1, Side::Sell, 100, 3});
    EXPECT_THROW((void)book.add_order({1, Side::Buy, 100, 3}), std::invalid_argument);
    EXPECT_TRUE(book.orders(Side::Buy).empty());
    const auto sells = book.orders(Side::Sell);
    ASSERT_EQ(sells.size(), 1U);
    expect_order(sells[0], 1, Side::Sell, 100, 3);
}

TEST(OrderBookTest, RejectsReuseOfFullyExecutedIds) {
    OrderBook book;
    add(book, {1, Side::Sell, 100, 3});
    ASSERT_EQ(book.add_order({2, Side::Buy, 100, 3}).size(), 1U);
    EXPECT_THROW((void)book.add_order({1, Side::Buy, 100, 3}), std::invalid_argument);
    EXPECT_THROW((void)book.add_order({2, Side::Sell, 100, 3}), std::invalid_argument);
    EXPECT_TRUE(book.orders(Side::Buy).empty());
    EXPECT_TRUE(book.orders(Side::Sell).empty());
}

TEST(OrderBookTest, RejectsInvalidSnapshotSide) {
    const OrderBook book;
    EXPECT_THROW((void)book.orders(static_cast<Side>(-1)), std::invalid_argument);
}

TEST(OrderBookTest, HandlesLargestPriceQuantityAndIdWithoutOverflow) {
    OrderBook book;
    const auto max_price = std::numeric_limits<Price>::max();
    const auto max_quantity = std::numeric_limits<Quantity>::max();
    const auto max_id = std::numeric_limits<OrderId>::max();
    add(book, {max_id, Side::Sell, max_price, max_quantity});
    const auto first = book.add_order({1, Side::Buy, max_price, max_quantity - 1});
    ASSERT_EQ(first.size(), 1U);
    expect_trade(first[0], 1, max_id, max_price, max_quantity - 1);
    const auto second = book.add_order({2, Side::Buy, max_price, 1});
    ASSERT_EQ(second.size(), 1U);
    expect_trade(second[0], 2, max_id, max_price, 1);
    EXPECT_TRUE(book.orders(Side::Sell).empty());
}

} // namespace
} // namespace mini_exchange
