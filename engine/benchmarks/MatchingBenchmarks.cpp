#include <benchmark/benchmark.h>
#include <random>

#include "../src/core/OrderBook.h"


class OrderBookBM_Fixture : public benchmark::Fixture
{
public:
    OrderBook book;
    std::vector<uint64_t> restingOrderIds;

    void SetUp(::benchmark::State& state) override
    {
        for (int i = 0; i < 1000; i++) {
            price p = 1000 - i;
            Order bid = {(uint64_t)i + 1, 1, OrderSide::BUY, OrderType::LIMIT, p, 10, 0};
            book.addOrder(bid);
            restingOrderIds.push_back(bid.orderId);
        }
    }

    void TearDown(::benchmark::State& state) {}     // NOTE: is tear down needed?
};


BENCHMARK_F(OrderBookBM_Fixture, BM_RealisticWorkload)(benchmark::State& state)
{
    std::mt19937 rng(123);
    std::uniform_int_distribution<int> action_dist(0, 9);
    uint64_t nextId = 2000;

    for (auto _ : state) {
        int an_action = action_dist(rng);
        if (an_action < 5) {              // 50% chance
            Order passiveBid = {nextId++, 1, OrderSide::BUY, OrderType::LIMIT, 9000, 5, 0};
            book.addOrder(passiveBid);
        } else if (an_action < 8) {       // 30% chance
            Order aggressiveSell = {nextId++, 1, OrderSide::SELL, OrderType::LIMIT, 9500, 5, 0};
            book.addOrder(aggressiveSell);
        } else {                            // 20% chance
            if (!restingOrderIds.empty()) {
                book.removeOrder(restingOrderIds.back());
                restingOrderIds.pop_back();
            }
        }
    }
}


BENCHMARK_F(OrderBookBM_Fixture, BM_FullMatch)(benchmark::State& state)
{
    for (auto _ : state) {
        state.PauseTiming();
        Order restingSell = {1, 1, OrderSide::SELL, OrderType::LIMIT, 100, 10, 0};
        book.addOrder(restingSell);
        Order aggressingBuy = {1, 1, OrderSide::BUY, OrderType::LIMIT, 100, 10, 0};
        state.ResumeTiming();
        book.addOrder(aggressingBuy);
    }
}


BENCHMARK_F(OrderBookBM_Fixture, BM_RestingLimitOrders_NoMatch)(benchmark::State& state)
{
    for (auto _ : state) {
        state.PauseTiming();
        Order restingSell1 = {2, 2, OrderSide::SELL, OrderType::LIMIT, 100, 10, 0};
        Order restingSell2 = {3, 3, OrderSide::SELL, OrderType::LIMIT, 90, 10, 0};
        Order restingBuy1 = {4, 4, OrderSide::BUY, OrderType::LIMIT, 40, 10, 0};
        Order restingBuy2 = {5, 5, OrderSide::BUY, OrderType::LIMIT, 20, 20, 0};
        state.ResumeTiming();
        book.addOrder(restingSell1);
        book.addOrder(restingSell2);
        book.addOrder(restingBuy1);
        book.addOrder(restingBuy2);
    }
}


BENCHMARK_F(OrderBookBM_Fixture, BM_RemoveOrder)(benchmark::State& state)
{
    for (auto _ : state) {
        state.PauseTiming();
        Order restingBuy = {6, 6, OrderSide::SELL, OrderType::LIMIT, 100, 10, 0};
        book.addOrder(restingBuy);
        state.ResumeTiming();
        book.removeOrder(restingBuy.orderId);
    }
}

BENCHMARK_MAIN();