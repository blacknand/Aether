#include "OrderBook.h"

bool OrderBook::addOrder(Order& order) 
{
    static uint32_t nextTradeId = 1;        // TODO: Improve this, probably not static
    switch (order.side) {
        case OrderSide::BUY:
            while (order.quantity > 0 && !asks.empty() && order.price >= asks.begin()->first) {
                std::queue<Order>& restingSellOrders = asks.begin()->second;
                while (order.quantity > 0 && !restingSellOrders.empty()) {
                    Order& restingOrder = restingSellOrders.front();
                    uint32_t tradeQuantity = std::min(order.quantity, restingOrder.quantity);
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
                        restingSellOrders.pop();
                }

                if (asks.begin()->second.empty())
                    asks.erase(asks.begin());
            }

            if (order.quantity != 0) {
                switch (order.type) {
                    case OrderType::LIMIT:
                        bids[order.price].push(order);
                        break;
                    case OrderType::MARKET:
                        return false;
                    default:
                        break;
                }
            }
            break;
        case OrderSide::SELL:
            while (order.quantity > 0 && !bids.empty() && order.price <= bids.begin()->first) {
                std::queue<Order>& restingBidOrders = bids.begin()->second;
                while (order.quantity > 0 && !restingBidOrders.empty()) {
                    Order& restingOrder = restingBidOrders.front();
                    uint32_t tradeQuantity = std::min(order.quantity, restingOrder.quantity);
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
                        restingBidOrders.pop();
                }

                if (bids.begin()->second.empty())
                    bids.erase(bids.begin());
            }

            // NOTE: could probably rewrite this core logic into a helper function instead

            if (order.quantity != 0) {
                switch (order.type) {
                    case OrderType::LIMIT:
                        asks[order.price].push(order);
                        break;
                    case OrderType::MARKET:
                        return false;
                    default:
                        break;
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

}


std::optional<price> OrderBook::getBestBid() const
{

}