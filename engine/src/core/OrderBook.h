#ifndef ORDER_BOOK_H
#define ORDER_BOOK_H

#include <map>
#include <list>
#include <unordered_map>
#include <iostream>
#include <chrono>

#include "Order.h"
#include "Trade.h"

using price = std::uint64_t;

class OrderBook
{
private:
    std::map<price, std::list<Order>> asks;
    std::map<price, std::list<Order>, std::greater<price>> bids;
    std::unordered_map<uint64_t, std::list<Order>::iterator> orderIdIndex;
public:
    bool addOrder(Order& order);
    bool removeOrder(price orderId);
    std::optional<price> getBestBid() const;
    std::optional<price> getBestAsk() const;
    void processTrade(std::list<Order>& restingOrder, Order& order);

    size_t getAskCountAt(price p) const;
    size_t getBidCountAt(price p) const;
    const Order* getTopAskAt(price p) const;
    const Order* getTopBidAt(price p) const;
    const Order* getLastBidAt(price p) const;
};

#endif // ORDER_BOOK_H