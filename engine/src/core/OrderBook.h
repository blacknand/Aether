#ifndef ORDER_BOOK_H
#define ORDER_BOOK_H

#include <map>
#include <list>
#include <unordered_map>
#include <iostream>
#include <chrono>
#include <vector>
#include <iterator>
#include <atomic>
#include <optional>
#include <memory>

#include "Order.h"
#include "Trade.h"
#include "OrderBookState.h"
#include "../utils/BlockingQueue.h"
#include "market_data.grpc.pb.h"

using price = std::optional<std::uint64_t>;

class OrderBook
{
private:
    std::map<price, std::list<Order>> asks;
    std::map<price, std::list<Order>, std::greater<price>> bids;
    std::unordered_map<uint64_t, std::list<Order>::iterator> orderIdIndex;
    std::atomic<uint64_t> nextTradeId = 1;
    std::unordered_set<uint64_t> securities;
public:
    OrderBook(std::shared_ptr<BlockingQueue<aether_market_data::OrderDelta>> blockingQueue) : tradeQueue(blockingQueue) {}
    std::optional<std::vector<Trade>> addOrder(Order& order);
    bool removeOrder(uint64_t orderId);
    std::optional<price> getBestBid() const;
    std::optional<price> getBestAsk() const;
    std::shared_ptr<BlockingQueue<aether_market_data::OrderDelta>> tradeQueue;
    void processTrade(Order& aggressiveOrder, uint64_t restingOrderId, std::vector<Trade>& trades);

    size_t getAskCountAt(price p) const;
    size_t getBidCountAt(price p) const;
    const Order* getTopAskAt(price p) const;
    const Order* getTopBidAt(price p) const;
    const Order* getLastBidAt(price p) const;
    bool isValidSecurity(uint64_t securityId) const;
    int loadSecurities(const std::string& securitiesPath);
    OrderBookState getSnapshot();
};

#endif // ORDER_BOOK_H