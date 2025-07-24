#ifndef TRADE_H
#define TRADE_H

struct Trade
{
    uint64_t tradeId;
    uint64_t securityId;
    double price;
    uint32_t quantity;
    uint64_t aggressingOrderId;
    uint64_t restingOrderId;
    uint64_t timestamp;
};

#endif // TRADE_H