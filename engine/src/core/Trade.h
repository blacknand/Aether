#ifndef TRADE_H
#define TRADE_H

#include <optional>

struct Trade
{
    uint64_t tradeId;
    uint64_t securityId;
    std::optional<uint64_t> price;
    uint32_t quantity;
    uint64_t aggressingOrderId;
    uint64_t restingOrderId;
    uint64_t timestamp;
};

#endif // TRADE_H