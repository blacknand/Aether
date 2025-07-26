#ifndef ORDER_BOOK_H
#define ORDER_BOOK_H

#include <map>
#include <queue>
#include <iostream>
#include <chrono>

#include "Order.h"

using price = std::uint64_t;

class OrderBook
{
private:
    std::map<price, std::queue<Order>> asks;
    std::map<price, std::queue<Order>, std::greater<price>> bids;
public:
    bool addOrder(const Order& order);
    bool removeOrder(price orderId);
    std::optional<price> getBestBid() const;
};

#endif // ORDER_BOOK_H