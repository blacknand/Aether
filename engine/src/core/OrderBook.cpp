#include "OrderBook.h"

std::optional<std::vector<Trade>> OrderBook::addOrder(Order& order) 
{
    // TODO: will need to add MARKET (FOK) logic etc,
    // TODO: can remove the nested if loops with ternary op

    std::vector<Trade> trades;
    if (order.side == OrderSide::ASK) {
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
        auto& protoSide = (order.side == OrderSide::BID) ? order_management::OrderSide::BID : order_management::OrderSide::ASK;
        auto& mapToUse = (order.side == OrderSide::BID) ? bids : asks;
        bool mapContainsPriceLevel = mapsToUse.contains(order.price);
        mapsToUse[order.price].push_back(order);
        auto it = std::prev(mapsToUse[order.price].end());
        orderIdIndex[order.orderId] = it;

        uint64_t newQuantity {0};
        for (const auto& order : mapsToUse.at(order.price))
            newQuantity += order.quantity;

        aether_market_data::OrderDelta delta{
            .action = (mapContainsPriceLevel) ? aether_market_data::DeltaAction::UPDATE : aether_market_data::DeltaAction::ADD,
            .side = order_management::OrderSide::protoSide,
            .price = order.price,
            .quantity = newQuantity
        };

        tradeQueue->push(delta);
    }

    return trades;
}


bool OrderBook::removeOrder(uint64_t orderId)
{
    auto it = orderIdIndex.find(orderId);
    if (it == orderIdIndex.end()) return false;
    auto orderIt = it->second;

    price p = orderIt->price;
    OrderSide side = orderIt->side;

    auto& mapsToUse = (side == OrderSide::BID) ? bids : asks;
    auto& orderList = mapsToUse.at(p);

    orderList.erase(orderIt);
    orderIdIndex.erase(it);

    uint64_t newTotalQuantity = 0;
    for (const auto& order : orderList)
        newTotalQuantity += order.quantity;

    aether_market_data::DeltaAction action;
    if (orderList.empty()) {
        action = aether_market_data::DeltaAction::DELETE;
        mapsToUse.erase(p);
    } else {
        action = aether_market_data::DeltaAction::UPDATE;
    }
   
   auto& deltaSide = (side == OrderSide::BID) ? order_management::OrderSide::BID : order_management::OrderSide::ASK;

    aether_market_data::OrderDelta delta{
        .action = action,
        .side = deltaSide,
        .price = p,
        .quantity = newTotalQuantity
    };

    tradeQueue->push(delta);

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

    auto listIteratorCopy = restingOrderIter;
    OrderSide side = restingOrderIter->side;
    price p = restingOrderIter->price;
    auto& mapToUse = (side == OrderSide::BID) ? bids : asks;

    // NOTE: You left off at step 1 on the mac screen. Continue

    if (mapToUse.contains(p)) {
        uint64_t newTotalQuantity {0};
        for (const auto& order : mapToUse.at(p)) 
            newTotalQuantity += order.quantity;

        order_management::OrderSide deltaSide = (side == OrderSide::BID) ? order_management::OrderSide::BID : order_management::OrderSide::ASK;
        aether_market_data::DeltaAction action = (newTotalQuantity > 0) ? aether_market_data::DeltaAction::UPDATE : aether_market_data::DeltaAction::DELETE;

        aether_market_data::OrderDelta delta{
            .action = action,
            .side = deltaSide,
            .price = p,
            .quantity = newTotalQuantity
        };
        tradeQueue->push(delta);
    }

    if (restingOrderIter->quantity == 0) {
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


OrderBookState OrderBook::getSnapshot()
{
    OrderBookState state;
    for (const auto& [key, value] : asks) {
        uint32_t totalShares {0};
        for (const auto& order : value)
            totalShares += order.quantity;
        state.askPriceLevel.emplace_back(key, totalShares);
    }

    for (const auto& [key, value] : bids) {
        uint32_t totalShares {0};
        for (const auto& order : value)
            totalShares += order.quantity;
        state.bidPriceLevel.emplace_back(key, totalShares);
    }

    return state;
}


int OrderBook::loadSecurities(const std::string& securitiesPath)
{
    std::ifstream file(securitiesPath);
    if (!file.is_open()) {
        std::cerr << "[gRPC-SERVER] Could not open securities.csv file" << std::endl;
        return -1;
    }

    std::string line;
    while (std::getline(file, line)) {
        size_t commaPos = line.find(',');
        if (commaPos != std::string::npos) {
            uint64_t securityId = std::stoull(line.substr(0, commaPos));
            securities.insert(securityId);
        }
    }
    file.close();
    return 0;
}


bool OrderBook::isValidSecurity(uint64_t securityId) const
{
    return securities.contains(securityId);
}