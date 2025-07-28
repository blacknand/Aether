#include "OrderBook.h"

bool OrderBook::addOrder(Order& order) 
{
    switch (order.side) {
        case OrderSide::BUY:
            while (order.quantity > 0 && !asks.empty() && order.price >= asks.begin()->first) {
                std::list<Order>& restingSellOrders = asks.begin()->second;
                processTrade(restingSellOrders, order);
                if (asks.begin()->second.empty())
                    asks.erase(asks.begin());
            }

            if (order.quantity != 0) {
                switch (order.type) {
                    case OrderType::LIMIT: {
                        bids[order.price].push_back(order);
                        auto it = std::prev(bids[order.price].end());
                        orderIdIndex[order.orderId] = it;
                        break;
                    }
                    case OrderType::MARKET: {
                        return false;
                    }
                    default: {
                        break;
                    }
                }
            }
            break;
        case OrderSide::SELL:
            while (order.quantity > 0 && !bids.empty() && order.price <= bids.begin()->first) {
                std::list<Order>& restingBidOrders = bids.begin()->second;
                processTrade(restingBidOrders, order);
                if (bids.begin()->second.empty())
                    bids.erase(bids.begin());
            }

            if (order.quantity != 0) {
                switch (order.type) {
                    case OrderType::LIMIT: {
                        asks[order.price].push_back(order);
                        auto it = std::prev(asks[order.price].end());
                        orderIdIndex[order.orderId] = it;
                        break;
                    }
                    case OrderType::MARKET: {
                        return false;
                    }
                    default: {
                        break;
                    }
                }
            }
            break;
        default:
            break;
    }

    return true;
}


bool OrderBook::removeOrder(uint64_t orderId)
{
    auto indexIt = orderIdIndex.find(orderId);
    if (indexIt == orderIdIndex.end()) return false;

    auto orderIt = indexIt->second;
    price p = orderIt ->price;

    if (orderIt->side == OrderSide::BUY) {
        bids.at(p).erase(orderIt);
        if (bids.at(p).empty())
            bids.erase(p);
    } else {
        asks.at(p).erase(orderIt);
        if (asks.at(p).empty())
            asks.erase(p);
    }

    orderIdIndex.erase(indexIt);
    return true;
}


void OrderBook::processTrade(std::list<Order>& restingOrders, Order& order)
{
    static uint32_t nextTradeId = 1;        // TODO: Improve this, probably not static
    while (order.quantity > 0 && !restingOrders.empty()) {
        Order& restingOrder = restingOrders.front();
        uint64_t tradeQuantity = std::min(order.quantity, restingOrder.quantity);
        Trade newTrade;
        newTrade.tradeId = nextTradeId++;
        newTrade.securityId = order.securityId;
        newTrade.price = restingOrder.price;
        newTrade.quantity = tradeQuantity;
        newTrade.aggressingOrderId = order.orderId;
        newTrade.restingOrderId = restingOrder.orderId;
        newTrade.timestamp = std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::system_clock::now().time_since_epoch()).count();

        order.quantity -= newTrade.quantity;
        restingOrder.quantity -= newTrade.quantity;
                
        if (restingOrder.quantity == 0)
            restingOrders.pop_front();
    }
}


std::optional<price> OrderBook::getBestBid() const
{
    if (bids.empty()) return {};
    return bids.begin()->second.front().price;
}


std::optional<price> OrderBook::getBestAsk() const
{
    if (asks.empty()) return {};
    return asks.begin()->second.front().price;
}


size_t OrderBook::getAskCountAt(price p) const {
    auto it = asks.find(p);
    return (it != asks.end()) ? it->second.size() : 0;
}


size_t OrderBook::getBidCountAt(price p) const {
    auto it = bids.find(p);
    return (it != bids.end()) ? it->second.size() : 0;
}


const Order* OrderBook::getTopAskAt(price p) const {
    auto it = asks.find(p);
    if (it == asks.end() || it->second.empty()) {
        return nullptr;
    }
    return &it->second.front();
}


const Order* OrderBook::getTopBidAt(price p) const {
    auto it = bids.find(p);
    if (it == bids.end() || it->second.empty()) {
        return nullptr;
    }
    return &it->second.front();
}


const Order* OrderBook::getLastBidAt(price p) const {
    auto it = bids.find(p);
    if (it == bids.end() || it->second.empty()) {
        return nullptr;
    }
    return &it->second.back(); 
}