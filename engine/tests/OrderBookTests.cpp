#include "../src/core/OrderBook.h"

#include <gtest/gtest.h>


class OrderBookTest : public testing::Test
{
protected:
    OrderBook book;
};


TEST_F(OrderBookTest, AddSingleLimitBuyOrder)
{
    Order buyOrder = {1, 1, OrderSide::BUY, OrderType::LIMIT, 100, 10, 0};
    book.addOrder(buyOrder);
    ASSERT_EQ(book.getBestBid().value_or(0), 100);
}


TEST_F(OrderBookTest, AddSingleLimitSellOrder)
{
    Order sellOrder = {2, 2, OrderSide::SELL, OrderType::LIMIT, 100, 10, 0};
    book.addOrder(sellOrder);
    ASSERT_EQ(book.getBestAsk().value_or(0), 100);
}


TEST_F(OrderBookTest, FullMatch)
{
    Order buyOrder = {3, 3, OrderSide::BUY, OrderType::LIMIT, 100, 10, 0};
    book.addOrder(buyOrder);
    Order sellOrder = {4, 4, OrderSide::SELL, OrderType::LIMIT, 100, 10, 0};
    book.addOrder(sellOrder);
    ASSERT_FALSE(book.getBestBid().has_value());
    ASSERT_FALSE(book.getBestAsk().has_value());
}


TEST_F(OrderBookTest, PartialFill_AgressorIsFilled)
{
    Order restingSellOrder = {5, 5, OrderSide::SELL, OrderType::LIMIT, 100, 100, 0};
    book.addOrder(restingSellOrder);
    Order aggressiveBuyOrder = {6, 6, OrderSide::BUY, OrderType::LIMIT, 100, 25, 0};
    book.addOrder(aggressiveBuyOrder);
    ASSERT_FALSE(book.getBestBid().has_value());
    ASSERT_EQ(book.getBestAsk().value_or(0), 100);
    ASSERT_EQ(book.getTopAskAt(100)->quantity, 75);
}


TEST_F(OrderBookTest, PartialFill_RestingIsFilled)
{
    Order restingSellOrder = {7, 7, OrderSide::SELL, OrderType::LIMIT, 100, 50, 0};
    book.addOrder(restingSellOrder);
    Order aggressiveBuyOrder = {8, 8, OrderSide::BUY, OrderType::LIMIT, 100, 100, 0};
    book.addOrder(aggressiveBuyOrder);
    ASSERT_FALSE(book.getBestAsk().has_value());
    ASSERT_EQ(book.getBestBid().value_or(0), 100);
    ASSERT_EQ(book.getTopBidAt(100)->quantity, 50);
}


TEST_F(OrderBookTest, WalkTheBook)
{
    Order restingSellOrder1 = {9, 9, OrderSide::SELL, OrderType::LIMIT, 100, 10, 0};
    Order restingSellOrder2 = {10, 10, OrderSide::SELL, OrderType::LIMIT, 101, 10, 0};
    book.addOrder(restingSellOrder1);
    book.addOrder(restingSellOrder2);
    Order aggressiveBuyOrder = {11, 11, OrderSide::BUY, OrderType::LIMIT, 101, 15, 0};
    book.addOrder(aggressiveBuyOrder);
    ASSERT_EQ(book.getBestAsk().value_or(0), 101);
    ASSERT_EQ(book.getTopAskAt(101)->quantity, 5);
    ASSERT_FALSE(book.getBestBid().has_value());
}


TEST_F(OrderBookTest, MarketOrderRejection)
{
    Order marketBuyOrder = {12, 12, OrderSide::BUY, OrderType::MARKET, 100, 10, 0};
    ASSERT_EQ(book.addOrder(marketBuyOrder), false);
}


TEST_F(OrderBookTest, TimePriority)
{
    Order sellOrder1 = {13, 13, OrderSide::SELL, OrderType::LIMIT, 100, 10, 0};
    Order sellOrder2 = {14, 14, OrderSide::SELL, OrderType::LIMIT, 100, 10, 0};
    book.addOrder(sellOrder1);
    book.addOrder(sellOrder2);
    Order aggressiveBuyOrder = {15, 15, OrderSide::BUY, OrderType::LIMIT, 300, 15, 0};
    book.addOrder(aggressiveBuyOrder);
    ASSERT_EQ(book.getAskCountAt(100), 1);
    ASSERT_EQ(book.getTopAskAt(100)->orderId, 14);
}


TEST_F(OrderBookTest, NoMatch_PriceNotGoodEnough)
{
    Order restingSellOrder = {15, 15, OrderSide::SELL, OrderType::LIMIT, 101, 10, 0};
    book.addOrder(restingSellOrder);
    Order limitBuyOrder = {16, 16, OrderSide::BUY, OrderType::LIMIT, 100, 10, 0};
    ASSERT_TRUE(book.addOrder(limitBuyOrder));
    ASSERT_EQ(book.getBestBid().value_or(0), 100);
}


int main(int argc, char **argv) 
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}