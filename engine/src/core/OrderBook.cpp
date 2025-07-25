#include "OrderBook.h"

bool OrderBook::addOrder(const Order& order) 
{
    switch (order.side) {
        case OrderSide::BUY:
            while (order.quantity > 0 && !asks.empty() && order.price >= asks.begin()->first) {
                std::queue<Order>& restingSellOrders = asks.begin()->second;
                for (; !restingSellOrders.empty(); restingSellOrders.pop()) {
                    Order& restingOrder = restingSellOrders.front();
                    uint32_t tradeQuantity = ;
                    Trade newTrade;
                    newTrade.tradeId = 
                    newTrade.securityId = 
                    newTrade.price = order.price;
                    newTrade.quantity = order.quantity;
                    newTrade.aggressingOrderId =
                    newTrade.restingOrderId = 
                    newTrade.timestamp = 

                    order.quantity -= newTrade.quantity;
                    restingOrder.quantity -= newTrade.quantity;
                }

                if (asks.begin().empty())
                    asks.erase(order.price);
            }

            if (order.quantity != 0) {
                switch (order.type) {
                    case OrderType::LIMIT:
                        bids.insert(order.price, order);
                    case OrderType::MARKET:
                        std::cout << "ORDER REJECTED ???? " << std::endl;
                    default:
                        break;
                }
            }
        case OrderSide::SELL:
            // Sell
            break;
        default:
            break;
    }
}


bool OrderBook::removeOrder(uint64_t orderId)
{

}


std::optional<price> OrderBook::getBestBid() const
{

}