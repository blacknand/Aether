#ifndef ORDER_BOOK_H
#define ORDER_BOOK_H

#include <map>
#include <queue>

#include "Order.h"

using price = std::uint64_t;

class OrderBook
{
private:
    std::map<Price, std::queue<Order>> asks;
    std::map<Price, std::queue<Order>, std::greater<Price>> bids;
public:
    bool AddOrder(const Order& order);
    bool RemoveOrder(uint64_t orderId);
    std::optional<Price> getBestBid() const;
};

#endif // ORDER_BOOK_H