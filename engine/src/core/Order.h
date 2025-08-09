#ifndef ORDER_H
#define ORDER_H

#include <optional>

enum class OrderSide { BID, ASK };
enum class OrderType { LIMIT, MARKET };

struct Order
{
    uint64_t orderId;
    uint64_t securityId;
    OrderSide side;
    OrderType type;
    std::optional<uint64_t> price;
    uint32_t quantity;
    uint64_t timestamp;
};

#endif // ORDER_H