#include "OrderBook.h"

std::optional<std::vector<Trade>> OrderBook::addOrder(Order& order) 
{
    // TODO: will need to add MARKET (FOK) logic etc,
    // TODO: can remove the nested if loops with ternary op

    std::vector<Trade> trades;
    if (order.side == OrderSide::ASK) {
        if (order.type == OrderType::MARKET) {
            while (order.quantity > 0 && !bids.empty()) {
                uint64_t restingOrderId = bids.begin()->second.front().orderId;
                processTrade(order, restingOrderId, trades);
            }
        } else {
            while (order.quantity > 0 && !bids.empty() && order.price.value() <= bids.begin()->first) {
                uint64_t restingOrderId = bids.begin()->second.front().orderId;
                processTrade(order, restingOrderId, trades);
            }
        }
    } else {
        if (order.type == OrderType::MARKET) {
            while (order.quantity > 0 && !asks.empty()) {
                uint64_t restingOrderId = asks.begin()->second.front().orderId;
                processTrade(order, restingOrderId, trades);
            }
        } else {
            while (order.quantity > 0 && !asks.empty() && order.price.value() >= asks.begin()->first) {
                uint64_t restingOrderId = asks.begin()->second.front().orderId;
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
        uint64_t priceVal = order.price.value();
        // TODO: optimise since this is a lot of redundant code
        if (order.side == OrderSide::BID) {
            bool wasLevelPresent = bids.contains(priceVal);
            bids[priceVal].push_back(order);
            auto it = std::prev(bids[priceVal].end());
            orderIdIndex[order.orderId] = it;

            uint64_t newTotalQuantity = 0;
            for (const auto& o : bids.at(priceVal))
                newTotalQuantity += o.quantity;

            aether_market_data::OrderDelta delta;
            delta.set_action((wasLevelPresent) ? aether_market_data::DeltaAction::UPDATE : aether_market_data::DeltaAction::ADD);
            delta.set_side(aether::OrderSide::BID);
            delta.set_price(order.price.value());
            delta.set_quantity(newTotalQuantity);

            tradeQueue->push(delta);
        } else {
            bool wasLevelPresent = asks.contains(priceVal);
            asks[priceVal].push_back(order);
            auto it = std::prev(asks[priceVal].end());
            orderIdIndex[order.orderId] = it;

            uint64_t newTotalQuantity = 0;
            for (const auto& o : asks.at(priceVal))
                newTotalQuantity += o.quantity;

            aether_market_data::OrderDelta delta;
            delta.set_action((wasLevelPresent) ? aether_market_data::DeltaAction::UPDATE : aether_market_data::DeltaAction::ADD);
            delta.set_side(aether::OrderSide::ASK);
            delta.set_price(order.price.value());
            delta.set_quantity(newTotalQuantity);

            tradeQueue->push(delta);
        }
    }

    return trades;
}


bool OrderBook::removeOrder(uint64_t orderId)
{
    auto it = orderIdIndex.find(orderId);
    if (it == orderIdIndex.end()) return false;
    auto orderIt = it->second;

    uint64_t p = orderIt->price.value();
    OrderSide side = orderIt->side;

    auto updateAndNotify = [&](auto& book) {
        auto& orderList = book.at(p);

        orderList.erase(orderIt);
        orderIdIndex.erase(it);

        uint64_t newTotalQuantity = 0;
        for (const auto& order : orderList)
            newTotalQuantity += order.quantity;

        aether_market_data::OrderDelta delta;
        delta.set_price(p);
        delta.set_side((side == OrderSide::BID) ? aether::OrderSide::BID : aether::OrderSide::ASK);
        delta.set_quantity(newTotalQuantity);

        if (newTotalQuantity == 0) {
            delta.set_action(aether_market_data::DeltaAction::DELETE);
            book.erase(p);
        } else {
            delta.set_action(aether_market_data::DeltaAction::UPDATE);
        }

        tradeQueue->push(delta);
    };

    if (side == OrderSide::BID)
        updateAndNotify(bids);
    else
        updateAndNotify(asks);

    return true;
}


void OrderBook::processTrade(Order& aggressiveOrder, uint64_t restingOrderId, std::vector<Trade>& trades)
{
    auto& restingOrderIter = orderIdIndex.at(restingOrderId);
    uint32_t tradeQuantity = std::min(aggressiveOrder.quantity, restingOrderIter->quantity);
    trades.emplace_back(Trade{
        .tradeId = nextTradeId++,
        .securityId = aggressiveOrder.securityId,
        .price = restingOrderIter->price.value(),
        .quantity = tradeQuantity,
        .aggressingOrderId = aggressiveOrder.orderId,
        .restingOrderId = restingOrderId,
        .timestamp = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count())
    });

    aggressiveOrder.quantity -= tradeQuantity;
    restingOrderIter->quantity -= tradeQuantity;

    auto listIteratorCopy = restingOrderIter;
    OrderSide side = restingOrderIter->side;
    uint64_t p = restingOrderIter->price.value();

    auto updateAndNotify = [&](auto& book) {
        if (restingOrderIter->quantity == 0) {
            auto& orderList = book.at(p);
            orderList.erase(listIteratorCopy);
            if (orderList.empty())
                book.erase(p);
            orderIdIndex.erase(restingOrderId);
        }

        uint64_t newTotalQuantity = 0;
        if (book.contains(p)) {
            for (const auto& order : book.at(p))
                newTotalQuantity += order.quantity;
        }

        aether_market_data::OrderDelta delta;
        delta.set_action((newTotalQuantity > 0) ? aether_market_data::DeltaAction::UPDATE : aether_market_data::DeltaAction::DELETE);
        delta.set_side((side == OrderSide::BID) ? aether::OrderSide::BID : aether::OrderSide::ASK);
        delta.set_price(p);
        delta.set_quantity(newTotalQuantity);

        tradeQueue->push(delta);
    };

    if (side == OrderSide::BID)
        updateAndNotify(bids);
    else
        updateAndNotify(asks);
}


std::optional<uint64_t> OrderBook::getBestBid() const
{
    if (bids.empty()) return std::nullopt;
    return bids.begin()->first;
}


std::optional<uint64_t> OrderBook::getBestAsk() const
{
    if (asks.empty()) return std::nullopt;
    return asks.begin()->first;
}


size_t OrderBook::getAskCountAt(uint64_t p) const 
{
    auto it = asks.find(p);
    return (it != asks.end()) ? it->second.size() : 0;
}


size_t OrderBook::getBidCountAt(uint64_t p) const 
{
    auto it = bids.find(p);
    return (it != bids.end()) ? it->second.size() : 0;
}


const Order* OrderBook::getTopAskAt(uint64_t p) const 
{
    auto it = asks.find(p);
    if (it == asks.end() || it->second.empty()) {
        return nullptr;
    }
    return &it->second.front();
}


const Order* OrderBook::getTopBidAt(uint64_t p) const 
{
    auto it = bids.find(p);
    if (it == bids.end() || it->second.empty()) {
        return nullptr;
    }
    return &it->second.front();
}


const Order* OrderBook::getLastBidAt(uint64_t p) const 
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
        uint64_t totalShares {0};
        for (const auto& order : value)
            totalShares += order.quantity;
        state.askPriceLevel.emplace_back(key, totalShares);
    }

    for (const auto& [key, value] : bids) {
        uint64_t totalShares {0};
        for (const auto& order : value)
            totalShares += order.quantity;
        state.bidsPriceLevel.emplace_back(key, totalShares);
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