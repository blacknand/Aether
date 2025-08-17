#ifndef ORDER_BOOK_STATE_H
#define ORDER_BOOK_STATE_H

#include "order_management.grpc.pb.h"
#include "PriceLevel.h"

struct OrderBookState
{
    std::vector<PriceLevel> askPriceLevel;
    std::vector<PriceLevel> bidsPriceLevel;
}

#endif  // ORDER_BOOK_STATE_H