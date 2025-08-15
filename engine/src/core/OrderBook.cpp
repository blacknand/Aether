#include "OrderBook.h"

std::optional<std::vector<Trade>> OrderBook::addOrder(Order& order) 
{
    // TODO: will need to add MARKET (FOK) logic etc,

    std::vector<Trade> trades;
    if (order.side == OrderSide::BID) {
        if (order.type == OrderType::MARKET) {
            while (order.quantity > 0 && !asks.empty()) {
                uint64_t restingOrderId = asks.begin()->second.front().orderId;
                processTrade(order, restingOrderId, trades);
            }
        } else {
            while (order.quantity > 0 && !asks.empty() && order.price >= asks.begin()->first) {
                uint64_t restingOrderId = asks.begin()->second.front().orderId;
                processTrade(order, restingOrderId, trades);
            }
        }
    } else {
        if (order.type == OrderType::MARKET) {
            while (order.quantity > 0 && !bids.empty()) {
                uint64_t restingOrderId = bids.begin()->second.front().orderId;
                processTrade(order, restingOrderId, trades);
            }
        } else {
            while (order.quantity > 0 && !bids.empty() && order.price <= bids.begin()->first) {
                uint64_t restingOrderId = bids.begin()->second.front().orderId;
                processTrade(order, restingOrderId, trades);
            }
        }
    }

    if (order.quantity > 0 && order.type == OrderType::MARKET) {
        if (!trades.empty()) {
            std::optional<uint64_t> lastPrice = trades.back().price;
            order.price = lastPrice;
            order.type = OrderType::LIMIT;
        } else {
            return std::nullopt;
        }
    }

    if (order.quantity > 0 && order.type == OrderType::LIMIT) {
        if (order.side == OrderSide::BID) {
            bids[order.price].push_back(order);
            auto it = std::prev(bids[order.price].end());
            orderIdIndex[order.orderId] = it;
        } else {
            asks[order.price].push_back(order);
            auto it = std::prev(asks[order.price].end());
            orderIdIndex[order.orderId] = it;
        }
    }

    return trades;
}


bool OrderBook::removeOrder(uint64_t orderId)
{
    auto it = orderIdIndex.find(orderId);
    if (it == orderIdIndex.end()) return false;
    auto orderIt = it->second;

    if (orderIt->side == OrderSide::BID) {
        auto& orderList = bids.at(orderIt->price);
        orderList.erase(orderIt);
        if (orderList.empty())
            bids.erase(orderIt->price);
    } else {
        auto& orderList = asks.at(orderIt->price);
        orderList.erase(orderIt);
        if (orderList.empty())
            asks.erase(orderIt->price);
    }
    orderIdIndex.erase(it);
    return true;
}


void OrderBook::processTrade(Order& aggressiveOrder, uint64_t restingOrderId, std::vector<Trade>& trades)
{
    nextTradeId++;        
    auto& restingOrderIter = orderIdIndex.at(restingOrderId);
    uint32_t tradeQuantity = std::min(aggressiveOrder.quantity, restingOrderIter->quantity);
    trades.emplace_back(Trade{
        .tradeId = nextTradeId++,
        .securityId = aggressiveOrder.securityId,
        .price = restingOrderIter->price,
        .quantity = tradeQuantity,
        .aggressingOrderId = aggressiveOrder.orderId,
        .restingOrderId = restingOrderId,
        .timestamp = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count())
    });

    aggressiveOrder.quantity -= tradeQuantity;
    restingOrderIter->quantity -= tradeQuantity;

    if (restingOrderIter->quantity == 0) {
        auto listIteratorCopy = restingOrderIter;
        OrderSide side = restingOrderIter->side;
        price p = restingOrderIter->price;

        if (side == OrderSide::BID) {
            auto& orderList = bids.at(p);
            orderList.erase(listIteratorCopy);

            if (orderList.empty())
                bids.erase(p);
        } else {
            auto& orderList = asks.at(p);
            orderList.erase(listIteratorCopy);

            if (orderList.empty())
                asks.erase(p);
        }
        orderIdIndex.erase(restingOrderId);
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


size_t OrderBook::getAskCountAt(price p) const 
{
    auto it = asks.find(p);
    return (it != asks.end()) ? it->second.size() : 0;
}


size_t OrderBook::getBidCountAt(price p) const 
{
    auto it = bids.find(p);
    return (it != bids.end()) ? it->second.size() : 0;
}


const Order* OrderBook::getTopAskAt(price p) const 
{
    auto it = asks.find(p);
    if (it == asks.end() || it->second.empty()) {
        return nullptr;
    }
    return &it->second.front();
}


const Order* OrderBook::getTopBidAt(price p) const 
{
    auto it = bids.find(p);
    if (it == bids.end() || it->second.empty()) {
        return nullptr;
    }
    return &it->second.front();
}


const Order* OrderBook::getLastBidAt(price p) const 
{
    auto it = bids.find(p);
    if (it == bids.end() || it->second.empty()) {
        return nullptr;
    }
    return &it->second.back(); 
}


OrderBookState getSnapshot()
{
    OrderBookState state;
    for (const auto& [key, value] : asks) {
        uint32_t totalShares {0};
        for (const auto& order : value)
            totalShares += order.value().quantity;
        state.askPriceLevel.emplace_back(key, total);
    }

    for (const auto& [key, value] : bids) {
        uint32_t totalShares {0};
        for (const auto& order : value)
            totalShares += order.value().quantity;
        state.bidPriceLevel.emplace_back(key, total);
    }

    return state;
}