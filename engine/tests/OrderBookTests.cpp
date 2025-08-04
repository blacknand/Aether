#include "../src/core/OrderBook.h"

#include <gtest/gtest.h>


class OrderBookTest : public testing::Test
{
protected:
    OrderBook book;
};


TEST_F(OrderBookTest, AddSingleLimitBuyOrder)
{
    Order buyOrder = {1, 1, OrderSide::BID, OrderType::LIMIT, 100, 10, 0};
    book.addOrder(buyOrder);
    ASSERT_EQ(book.getBestBid().value_or(0), 100);
}


TEST_F(OrderBookTest, AddSingleLimitSellOrder)
{
    Order sellOrder = {2, 2, OrderSide::ASK, OrderType::LIMIT, 100, 10, 0};
    book.addOrder(sellOrder);
    ASSERT_EQ(book.getBestAsk().value_or(0), 100);
}


TEST_F(OrderBookTest, FullMatch_LimitOrder)
{
    Order buyOrder = {3, 3, OrderSide::BID, OrderType::LIMIT, 100, 10, 0};
    book.addOrder(buyOrder);
    Order sellOrder = {4, 4, OrderSide::ASK, OrderType::LIMIT, 100, 10, 0};
    book.addOrder(sellOrder);
    ASSERT_FALSE(book.getBestBid().has_value());
    ASSERT_FALSE(book.getBestAsk().has_value());
}


TEST_F(OrderBookTest, PartialFill_AgressorIsFilled)
{
    Order restingSellOrder = {5, 5, OrderSide::ASK, OrderType::LIMIT, 100, 100, 0};
    book.addOrder(restingSellOrder);
    Order aggressiveBuyOrder = {6, 6, OrderSide::BID, OrderType::LIMIT, 100, 25, 0};
    book.addOrder(aggressiveBuyOrder);
    ASSERT_FALSE(book.getBestBid().has_value());
    ASSERT_EQ(book.getBestAsk().value_or(0), 100);
    ASSERT_EQ(book.getTopAskAt(100)->quantity, 75);
}


TEST_F(OrderBookTest, PartialFill_RestingIsFilled)
{
    Order restingSellOrder = {7, 7, OrderSide::ASK, OrderType::LIMIT, 100, 50, 0};
    book.addOrder(restingSellOrder);
    Order aggressiveBuyOrder = {8, 8, OrderSide::BID, OrderType::LIMIT, 100, 100, 0};
    book.addOrder(aggressiveBuyOrder);
    ASSERT_FALSE(book.getBestAsk().has_value());
    ASSERT_EQ(book.getBestBid().value_or(0), 100);
    ASSERT_EQ(book.getTopBidAt(100)->quantity, 50);
}


TEST_F(OrderBookTest, WalkTheBook_LimitOrder)
{
    Order restingSellOrder1 = {9, 9, OrderSide::ASK, OrderType::LIMIT, 100, 10, 0};
    Order restingSellOrder2 = {10, 10, OrderSide::ASK, OrderType::LIMIT, 101, 10, 0};
    book.addOrder(restingSellOrder1);
    book.addOrder(restingSellOrder2);
    Order aggressiveBuyOrder = {11, 11, OrderSide::BID, OrderType::LIMIT, 101, 15, 0};
    book.addOrder(aggressiveBuyOrder);
    ASSERT_EQ(book.getBestAsk().value_or(0), 101);
    ASSERT_EQ(book.getTopAskAt(101)->quantity, 5);
    ASSERT_FALSE(book.getBestBid().has_value());
}


TEST_F(OrderBookTest, MarketOrderRejection)
{
    Order marketBuyOrder = {12, 12, OrderSide::BID, OrderType::MARKET, 100, 10, 0};
    ASSERT_TRUE(book.addOrder(marketBuyOrder).empty());
}


TEST_F(OrderBookTest, TimePriority)
{
    Order sellOrder1 = {13, 13, OrderSide::ASK, OrderType::LIMIT, 100, 10, 0};
    Order sellOrder2 = {14, 14, OrderSide::ASK, OrderType::LIMIT, 100, 10, 0};
    book.addOrder(sellOrder1);
    book.addOrder(sellOrder2);
    Order aggressiveBuyOrder = {15, 15, OrderSide::BID, OrderType::LIMIT, 300, 15, 0};
    book.addOrder(aggressiveBuyOrder);
    ASSERT_EQ(book.getAskCountAt(100), 1);
    ASSERT_EQ(book.getTopAskAt(100)->orderId, 14);
}


TEST_F(OrderBookTest, NoMatch_PriceNotGoodEnough)
{
    Order restingSellOrder = {15, 15, OrderSide::ASK, OrderType::LIMIT, 101, 10, 0};
    book.addOrder(restingSellOrder);
    Order limitBuyOrder = {16, 16, OrderSide::BID, OrderType::LIMIT, 100, 10, 0};
    ASSERT_TRUE(book.addOrder(limitBuyOrder).empty());
    ASSERT_EQ(book.getBestBid().value_or(0), 100);
}


TEST_F(OrderBookTest, RemoveExistingOrder)
{
    Order limitOrder = {16, 16, OrderSide::BID, OrderType::LIMIT, 100, 1, 0};
    book.addOrder(limitOrder);
    ASSERT_EQ(book.getBidCountAt(100), 1);
    ASSERT_TRUE(book.removeOrder(16));
}


TEST_F(OrderBookTest, RemoveNonExistentOrder)
{
    ASSERT_FALSE(book.removeOrder(17));
}


TEST_F(OrderBookTest, RemoveFromMiddleOfList)
{
    Order limitOrder1 = {18, 18, OrderSide::BID, OrderType::LIMIT, 100, 1, 0};
    Order limitOrder2 = {19, 19, OrderSide::BID, OrderType::LIMIT, 100, 1, 0};
    Order limitOrder3 = {20, 20, OrderSide::BID, OrderType::LIMIT, 100, 1, 0};
    book.addOrder(limitOrder1);
    book.addOrder(limitOrder2);
    book.addOrder(limitOrder3);
    ASSERT_EQ(book.getBidCountAt(100), 3);
    book.removeOrder(19);
    ASSERT_EQ(book.getBidCountAt(100), 2);
    ASSERT_EQ(book.getTopBidAt(100)->orderId, 18);
    ASSERT_EQ(book.getLastBidAt(100)->orderId, 20);
}


TEST_F(OrderBookTest, RemoveLastOrderAtPriceLevel)
{
    Order limitOrder1 = {21, 21, OrderSide::BID, OrderType::LIMIT, 100, 1, 0};
    book.addOrder(limitOrder1);
    ASSERT_EQ(book.getBidCountAt(100), 1);
    book.removeOrder(21);
    ASSERT_EQ(book.getBidCountAt(100), 0);
}


TEST_F(OrderBookTest, FullMatch_MarketOrder)
{
    Order restingLimitAsk = {22, 22, OrderSide::ASK, OrderType::LIMIT, 100, 10, 0};
    book.addOrder(restingLimitAsk);
    ASSERT_EQ(book.getBestAsk().value_or(0), 100);
    Order aggressiveMarketBid = {23, 23, OrderSide::BID, OrderType::MARKET, 100, 10, 0};
    book.addOrder(aggressiveMarketBid);
    ASSERT_FALSE(book.getBestBid().has_value());
    ASSERT_FALSE(book.getBestAsk().has_value());
}


TEST_F(OrderBookTest, MarketOrderRestsWithNoLiquidity)
{
    Order marketOrderBid = {23, 23, OrderSide::BID, OrderType::MARKET, 100, 10, 0};
    book.addOrder(marketOrderBid);
    ASSERT_EQ(book.getBidCountAt(100), 1);
    ASSERT_EQ(book.getBestBid().value_or(0), 100);
}


TEST_F(OrderBookTest, PartialFill_MarketOrder)
{
    Order restingLimitAsk = {25, 25, OrderSide::ASK, OrderType::LIMIT, 101, 5, 0};
    book.addOrder(restingLimitAsk);
    Order marketOrderBid = {24, 24, OrderSide::BID, OrderType::MARKET, 100, 10, 0};
    book.addOrder(marketOrderBid);
    ASSERT_FALSE(book.getBestAsk().has_value());
    ASSERT_TRUE(book.getBestBid().has_value());
    ASSERT_EQ(book.getBestBid().value_or(0), 100);
    ASSERT_EQ(book.getTopBidAt(100)->quantity, 5);
}


TEST_F(OrderBookTest, WalkTheBook_MarketOrder)
{
    Order restingLimitAsk1 = {28, 28, OrderSide::ASK, OrderType::LIMIT, 100, 5, 0};
    Order restingLimitAsk2 = {29, 29, OrderSide::ASK, OrderType::LIMIT, 95, 5, 0};
    book.addOrder(restingLimitAsk1);
    book.addOrder(restingLimitAsk2);
    Order marketOrderBid = {27, 27, OrderSide::BID, OrderType::MARKET, 100, 10, 0};
    book.addOrder(marketOrderBid);
    ASSERT_FALSE(book.getBestBid().has_value());
    ASSERT_FALSE(book.getBestAsk().has_value());
}


int main(int argc, char **argv) 
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}