## High level architecture

## Core Data Structures

## Price-Time Priority Algorithm
The Price-Time Priority is also known as just FIFO. It is goverened by a strict, two-tiered hierachy of rules that are applied in sequence:
1. **Price Priority:** The primary sorting criteria is price. Orders with better prices gain precedence over all orders with worse price. 
    - For *bid orders*, a higher price is considered better.
    - For *ask orders*, a lower price is considered better.


2. **Time priority**: If multiple orders exist at the same price level, the secondary sorting criterion is time. The order that was recieved by the exchange first gets precedence over later orders at the same price. The "time" component is typically a high-precision timestamp (nanoseconds) assigned by the exchange the moment an order is recieved and accepted by the system.

When a new aggressing order enters the system, the matching engine immediately attemps to match it against existing passive orders on the other side of the LOB. A trade is possible if the prices "cross" -- if an incoming bid (buy) order's price is `>=` the best ask (sell) price, or if an incoming ask order's price is `<=` the best bid price. The matching process then follows a determistic sequential set of steps (it is the same for both ask and bid, just mirror them), I cover a bid here:

1. **Identify Counterpart:** The engine looks at the ask side of the LOB and identifies the best price level; the one with the lowest price.
2. **Check Price:** Since it is a market order, its implicit price is effectively infinite (because the implication is tha the trader is willing to pay *anything* for the securities), so it will cross with any available ask. For a limit buy order, the engine would check if its limit is `>=` the best ask price.
3. **Determine Trade Quantity:** The engine takes the first order in the queue at that best ask price (the one with time priority). The trade quantity is the minimum of the aggressing order's remaning quantity and the passive order's quantity: `tradeQuantity = min(agressorQuantity, passiveQuantity)`
4. **Generate Trade Execution:** A trade record is created, capturing the taker's ID, the maker's ID, the trade price, and the trade quantity. This record is then sent to clearing and settlement systems and published as market data.
5. **Update Quantities:** The quantites of both the aggressing order and the passive order are decremented by the trade quantity.
6. **Update Book State:** 
    - If the passive order's quantity is now 0, it is full filled and removed from the LOB.
    - If the passive order was only partially filled, it remains in its position at the head of its price-level queue, retaining its time priority.
7. **Repeat or Rest:**
    - if the aggressing order's quantity is now 0, it is fully filled and the matching process for this order concludes.
    - If the aggressing order still has remaining quantity, the process repeats from step 3, either with the next order in the queue at the current price level, or if that level is depleted, with the first resting order at the next-best price level. This process of an aggressive order consuming liquidity across multiple price levels is known as "walking the book".

### Limit Order Book (LOB)
The LOB is a real-time dynamic data structure for all orders of a particular financial instrument. It provides a completely transparent view for an asset at various price points. The LOB is composed of two distinct sides:


- **The Bid Side (Buy Orders)**: This side lists all pending bid (buy)orders. These orders are sorted in descending order by price, meaning the bid with the highest price is at the top of the list. The highest-priced bid is the *best bid*.
- **The Ask Side (Sell Orders)**: This side lists all pending ask (sell) orders. These orders are sorted in ascending order, meaning the order with the lowest price is at the top of the list. The lowest-priced ask is know as the *best offer*.

Several key characteristics of the market can be derived directly from the LOB:


- **Best Bid and Offer (BBO)**: The highest bid and lowest ask at any given momemnt in time.
- **Spread**: The difference between the best ask and the best bid. A narrow spread generally indicates high liquidity and market efficiency. 
- **Market Depth**: The quantity of shares available at different price points on both the bid and ask side. This gives traders insight into how much volume can be traded without significantly impacting the price of the security.


## Alternative Models (Pro-Rata)
Pro-Rate gives first priority to price. However for orders at the same price level, it allocates incoming volume proportionally based on the size of the resting orders rathern than by time of arrival. For example, if a 1000 share buy order arrives to match against a price level with two sell orders, one for 800 shares and one for 200 then the 800 share order would recieve 80% of the fill and the 200 share order would recieve 20%.

## Key design decisions