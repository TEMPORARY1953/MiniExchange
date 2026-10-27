#include "mini_exchange/order_book.hpp"

#include <benchmark/benchmark.h>

#include <cstdint>

namespace {

using namespace mini_exchange;

// One iteration is a fresh, bounded batch. Construction and destruction are
// included in the measurement; console output and snapshots are excluded.
void BM_InsertNonCrossing(benchmark::State& state) {
    const auto count = static_cast<OrderId>(state.range(0));
    for (auto iteration : state) {
        (void)iteration;
        OrderBook book;
        for (OrderId id = 1; id <= count; ++id) {
            const auto trades = book.add_order({id, Side::Buy, 100 + static_cast<Price>(id % 32), 10});
            benchmark::DoNotOptimize(trades.data());
        }
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(count));
}

void BM_MatchPairs(benchmark::State& state) {
    const auto count = static_cast<OrderId>(state.range(0));
    for (auto iteration : state) {
        (void)iteration;
        OrderBook book;
        for (OrderId i = 0; i < count; ++i) {
            const auto resting = book.add_order({2 * i + 1, Side::Sell, 100, 10});
            const auto trades = book.add_order({2 * i + 2, Side::Buy, 101, 10});
            benchmark::DoNotOptimize(resting.data());
            benchmark::DoNotOptimize(trades.data());
        }
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(2 * count));
}

void BM_SweepPriceLevels(benchmark::State& state) {
    const auto count = static_cast<OrderId>(state.range(0));
    for (auto iteration : state) {
        (void)iteration;
        OrderBook book;
        for (OrderId id = 1; id <= count; ++id) {
            const auto resting = book.add_order({id, Side::Sell, 100 + static_cast<Price>(id % 32), 10});
            benchmark::DoNotOptimize(resting.data());
        }
        // Sweep all 32 levels, leaving one unit of the final resting order.
        const auto trades = book.add_order({count + 1, Side::Buy, 131,
                                            static_cast<Quantity>(10 * count - 1)});
        benchmark::DoNotOptimize(trades.data());
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(count + 1));
}

BENCHMARK(BM_InsertNonCrossing)->Arg(128)->Arg(4096);
BENCHMARK(BM_MatchPairs)->Arg(128)->Arg(4096);
BENCHMARK(BM_SweepPriceLevels)->Arg(128)->Arg(4096);

} // namespace
