#ifndef PRICE_LEVEL_H
#define PRICE_LEVEL_H

struct PriceLevel
{
    uint64_t price;
    uint64_t totalShares;
    PriceLevel(uint64_t price_, uint64_t totalShares_) : 
                                        price(std::move(price_)), 
                                        totalShares(std::move(totalShares_)) {}
};

#endif  // PRICE_LEVEL_H