## Matching Engine Core Logic


### The Add Order Functionality


### OrderBook
`std::map` is used for automatically handling price priority. Within each `std::map`, a `std::queue` is used to automatically handle time priority.


- `std::map<price, std::queue<Order>> asks`, the begining iterator always points to the lowest price (the best offer to sell).
- `std::map<price, std::queue<Order>, std::greater<price>> bids`, the beginning iterator always points to the highest price (the best offer to buy).

