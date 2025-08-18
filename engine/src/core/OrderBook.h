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
#include <fstream>
#include <string>

#include "Order.h"
#include "Trade.h"
#include "OrderBookState.h"
#include "../utils/BlockingQueue.h"
#include "market_data.grpc.pb.h"


class OrderBook
{
private:
    std::map<uint64_t, std::list<Order>> asks;
    std::map<uint64_t, std::list<Order>, std::greater<uint64_t>> bids;
    std::unordered_map<uint64_t, std::list<Order>::iterator> orderIdIndex;
    std::atomic<uint64_t> nextTradeId = 1;
    std::unordered_set<uint64_t> securities;
public:
    OrderBook(std::shared_ptr<BlockingQueue<aether_market_data::OrderDelta>> blockingQueue) : tradeQueue(blockingQueue) {}
    std::optional<std::vector<Trade>> addOrder(Order& order);
    bool removeOrder(uint64_t orderId);
    std::optional<uint64_t> getBestBid() const;
    std::optional<uint64_t> getBestAsk() const;
    std::shared_ptr<BlockingQueue<aether_market_data::OrderDelta>> tradeQueue;
    void processTrade(Order& aggressiveOrder, uint64_t restingOrderId, std::vector<Trade>& trades);

    size_t getAskCountAt(uint64_t p) const;
    size_t getBidCountAt(uint64_t p) const;
    const Order* getTopAskAt(uint64_t p) const;
    const Order* getTopBidAt(uint64_t p) const;
    const Order* getLastBidAt(uint64_t p) const;
    bool isValidSecurity(uint64_t securityId) const;
    int loadSecurities(const std::string& securitiesPath);
    OrderBookState getSnapshot();
};

#endif // ORDER_BOOK_H