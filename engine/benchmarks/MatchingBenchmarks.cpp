// engine/benchmarks/MatchingBenchmarks.cpp
// Tests the core C++ engine standalone by directly interfacing

#include <benchmark/benchmark.h>
#include <random>
#include <memory>

#include "../src/core/OrderBook.h"
#include "market_data.grpc.pb.h"


static void SetupOrderBook(OrderBook& book)
{
    for (int i = 0; i < 1000; i++) {
        Order bid = {(uint64_t)i + 1, 1, OrderSide::BID, OrderType::LIMIT, (uint64_t)(10000 - i), 10, 0};
        book.addOrder(bid);
    }

    for (int i = 0; i < 1000; i++) {
        Order ask = {(uint64_t)i + 1001, 1, OrderSide::ASK, OrderType::LIMIT, (uint64_t)(10001 + i), 10, 0};
        book.addOrder(ask);
    }
}


static void BM_RealisticTrade(benchmark::State& state) 
{
    for (auto _ : state) {
        state.PauseTiming();

        auto blockingQueue = std::make_shared<BlockingQueue<aether_market_data::OrderDelta>>();
        auto book = std::make_shared<OrderBook>(blockingQueue);
        SetupOrderBook(*book);
        Order aggressiveSell = {2001, 1, OrderSide::ASK, OrderType::LIMIT, 10000, 5, 0};
        state.ResumeTiming();
        auto trades = book->addOrder(aggressiveSell);
        benchmark::ClobberMemory(); 
        benchmark::DoNotOptimize(trades);
    }
}
BENCHMARK(BM_RealisticTrade);


static void BM_PassiveInsert(benchmark::State& state) 
{
    for (auto _ : state) {
        state.PauseTiming();
        auto blockingQueue = std::make_shared<BlockingQueue<aether_market_data::OrderDelta>>();
        auto book = std::make_shared<OrderBook>(blockingQueue);
        SetupOrderBook(*book);
        Order passiveBuy = {2001, 1, OrderSide::BID, OrderType::LIMIT, 9990, 5, 0};
        state.ResumeTiming();

        auto trades = book->addOrder(passiveBuy);
        
        benchmark::ClobberMemory();
        benchmark::DoNotOptimize(trades);
    }
}
BENCHMARK(BM_PassiveInsert);


static void BM_RemoveOrder(benchmark::State& state) 
{
    for (auto _ : state) {
        state.PauseTiming();
        auto blockingQueue = std::make_shared<BlockingQueue<aether_market_data::OrderDelta>>();
        auto book = std::make_shared<OrderBook>(blockingQueue);
        SetupOrderBook(*book);
        state.ResumeTiming();
        book->removeOrder(500); 
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_RemoveOrder);


static void BM_MarketOrderTrade(benchmark::State& state) 
{
    for (auto _ :  state) {
        state.PauseTiming();
        auto blockingQueue = std::make_shared<BlockingQueue<aether_market_data::OrderDelta>>();
        auto book = std::make_shared<OrderBook>(blockingQueue);
        SetupOrderBook(*book);
        Order aggressiveBid = {2001, 1, OrderSide::BID, OrderType::MARKET, 10000, 10, 0};
        state.ResumeTiming();
        auto trades = book->addOrder(aggressiveBid);
        benchmark::ClobberMemory();
        benchmark::DoNotOptimize(trades);
    }
}
BENCHMARK(BM_MarketOrderTrade);


BENCHMARK_MAIN();