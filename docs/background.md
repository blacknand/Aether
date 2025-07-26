# Trading background, terminology and a high-level conceptual overview
This document aims to cover the prerequisite knowledge required to understand the equity matching engine. This involves knowing the terminology, trading operations, conditions and concepts.
## Essential order terminology: Market, Limit and TIF conditions
A matching engine must process serveral types of orders. 
### Market vs. Limit Orders
- **Limit Order:** An instruction to buy or sell a specified quantity of an asset at a specified price or better. A buy limit order specifies the *maximum price* a trader is willing to pay, while a sell limit order specifies the *minimum price* a trader is willing to accept. Limit orders provide traders with precise control over the execution price; however the execution of the trade is not guranteed. If the market price never reaches the limit, the order will not be filled. When a limit order cannot be matched immediately, it is placed on the LOB where it "rests" and adds liquidity to the market. Trades who place limit orders are referred to as "makers".
- **Market Order:** A market order is an instruction to buy or sell a specified quantity of an asset immediately at the best available price in the market. Market orders prioritise speed and certianty of execution over price control. In volatile markets, market orders can suffer from slippage where the average excecution price is worse than the price quoted at the moment of order submission. Market orders consume liquidity from the LOB and are placed by "takers".

### Time in Force (TIF) Conditions
TIF conditions are instructions attatched to an order that define its lifetime in the market. Some common TIF conitions are:
1. **Good-Till-Cancelled (GTC):** The order remains active in the LOB until it is either fully executed or explicilty cancelled by the trader. This is typically the default for an order.
2. **Immediate-or-Cancel (IOC):** This condition requires that all or part of the order be executed immediately upon submission. Any portion of the order that cannot be filled instantly against resting orders in the book is cancelled and removed. This allows for partial fills.
3. **Fill-or-Kill (FOK):** This is a stricter condition that requires the *entire* order to be executed immediately. If the full quantity cannot be matched against resting orders at the specified limit price or better, the entire order is cancelled. No partial fills are permitted.

## Essential trading terminology
- **Liquidity:** How quickly an asset can be converted into cash without significantly affecting its market price. You can think of it as on a spectrum, an asset with high liquidity or is "very liquid" means you can sell it almost immediately for its **full** market value. If it is an illiquid asset then it takes a long time to sell, or if you do sell it then you will need to do so at a very steep discount to get cash quickly.
- **Aggressing Order**: An incoming order that is executable immediately, it actively seeks out and "hits" a resting order on the opposite side of the book. This happens when the order's price instruction *crosses the spread*
    - For a **bid**, it becomes aggressive if its limit price is at or above the current best ask.
    - For a **ask**, it becomes aggressive if its limit price is at or below the current best bid.
Because these orders remove orders from the book, they are said to be **"taking liquidity"**.
- **Passive Order**: An order that does not execute immediately and instead rests on the CLOB, waiting for a future order to trade against it. It is passive because it does not **cross the spread**. 
    - A bid order with a price below the best ask.
    - A ask order with a price above the best bid.
By sitting on the CLOB, these orders provide trading oppurtunities for others and therefore **"make liquidity"** or add depth tot eh market and are the trader therefore placing a passive order is the "maker".