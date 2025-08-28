#include "AetherServer.h"


grpc::Status MatchingEngineImpl::SubmitOrder(grpc::ServerContext* context, 
                                                const aether::OrderRequest* request, 
                                                aether::OrderConfirmation* orderConfirmation) 
{
    uint64_t new_id = ++orderId;
    uint64_t securityId = request->security_id();
    aether::OrderSide aetherSide = request->side();
    aether::OrderType aetherType = request->type();
    std::optional<OrderSide> side = convertToOrderSide(aetherSide);
    std::optional<OrderType> type = convertToOrderType(aetherType);
    uint32_t quantity = request->quantity();
    std::optional<uint64_t> price = request->price();
    uint64_t time = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count());

    if (!side.has_value() || !type.has_value()) 
        return buildErrorResponse(orderConfirmation, "[gRPC-SERVER] Invalid order side or invalid order type provided. Rejecting order.",
                                    "Invalid order side or invalid order type.");

    if (type.has_value() && type.value() == OrderType::LIMIT && !price.has_value()) 
        return buildErrorResponse(orderConfirmation, "[gRPC-SERVER] Limit order with no price provided. Rejecting order.",
                                    "A limit order must have a price provided.");

    if (quantity <= 0) 
        return buildErrorResponse(orderConfirmation, "[gRPC-SERVER] An order with an invalid non-positive quantity of " + std::to_string(quantity) + " has been placed. Rejecting order.",
                                    "An order with an invalid non-positive quantity of " + std::to_string(quantity) + " has been placed.");

    if (type.value() == OrderType::LIMIT && price.value() <= 0) 
        return buildErrorResponse(orderConfirmation, "[gRPC-SERVER] A limit order with an invalid non-positive price of " + std::to_string(price.value()) + " has been placed. Rejecting order.",
                                    "A limit order with an invalid non-positive price of " + std::to_string(price.value()) + " has been placed.");

    if (type == OrderType::MARKET && price.has_value())
        return buildErrorResponse(orderConfirmation, "[gRPC-SERVER] A market order with a specified price has been placed. Rejecting order.",
                                    "A market order with a specified price has been placed.");

    if (!orderBook->isValidSecurity(securityId)) 
        return buildErrorResponse(orderConfirmation, "[gRPC-SERVER] Invalid security ID. Rejecting order.",
                                    "Invalid security ID.");

    Order order{
        .orderId = new_id, 
        .securityId = securityId, 
        .side = side.value(), 
        .type = type.value(), 
        .quantity = quantity, 
        .price = price, 
        .timestamp = time};
    std::optional<std::vector<Trade>> trades = orderBook->addOrder(order);

    if (!trades.has_value())
        return buildErrorResponse(orderConfirmation, "[gRPC-SERVER] No trades to stream. Rejecting order.",
                                    "No trades to stream.");
    
    std::unique_lock lock{streamTradeMut};
    for (const auto& trade : trades.value())
        tradesQ.push(trade);
    lock.unlock();
    tradesReady.notify_one();
    
    orderConfirmation->set_order_id(new_id);
    orderConfirmation->set_accepted(true);
    orderConfirmation->set_reason("Valid order. All checks passed.");
    return grpc::Status::OK;
}


grpc::Status MatchingEngineImpl::StreamTrades(grpc::ServerContext* context, 
                                                const aether::StreamRequest* request, 
                                                grpc::ServerWriter<aether::Trade>* writer) 
{
    while (!context->IsCancelled()) {
        std::unique_lock lock{streamTradeMut};
        tradesReady.wait(lock, [this]{ return !tradesQ.empty(); });
        Trade& trade = tradesQ.front();
        tradesQ.pop();
        lock.unlock();

        aether::Trade aetherTrade;
        aetherTrade.set_trade_id(trade.tradeId);
        aetherTrade.set_security_id(trade.securityId);
        aetherTrade.set_price(trade.price.value());
        aetherTrade.set_quantity(trade.quantity);
        aetherTrade.set_timestamp_ns(trade.timestamp);

        writer->Write(aetherTrade);
    }

    std::cout << "[gRPC-SERVER] Client disconnected from the trades stream." << std::endl;
    return grpc::Status::OK;
}


void RunServer(const std::string& securitiesPath)
{
    // TODO: Output to std::cout when a user connects to the server,
    // and perhaps log when the user submits any kind of request to the server too
    std::string server_address("0.0.0.0:50051");    // NOTE: will probably need to change

    auto blockingQueue = std::make_shared<BlockingQueue<aether_market_data::OrderDelta>>();
    auto orderBook = std::make_shared<OrderBook>(blockingQueue);

    orderBook->loadSecurities(securitiesPath);

    MatchingEngineImpl matchingEngineService(orderBook);
    StreamOrderBookStateImpl streamOrderBookService(blockingQueue, orderBook); 
    grpc::ServerBuilder builder;

    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&matchingEngineService);
    builder.RegisterService(&streamOrderBookService);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());

    std::cout << "[gRPC-SERVER] gRPC Aether Server listening on " << server_address << std::endl;
    server->Wait();
}


std::optional<OrderSide> MatchingEngineImpl::convertToOrderSide(aether::OrderSide& orderSide)
{
    OrderSide res;
    if (orderSide == aether::OrderSide::BID) {
        res = OrderSide::BID;
    } else if (orderSide == aether::OrderSide::ASK) {
        res = OrderSide::ASK;
    } else {
        std::cout << "[gRPC-SERVER] aether::OrderSide::SIDE_UNKWON" << std::endl;
        return std::nullopt;
    }
    return res;
}


std::optional<OrderType> MatchingEngineImpl::convertToOrderType(aether::OrderType& orderType)
{
    OrderType res;
    if (orderType == aether::OrderType::MARKET) {
        res = OrderType::MARKET;
    } else if (orderType == aether::OrderType::LIMIT) {
        res = OrderType::LIMIT;
    } else {
        std::cout << "[gRPC-SERVER] aether::OrderType::TYPE_UNKOWN" << std::endl;
        return std::nullopt;
    }
    return res;
}


grpc::Status MatchingEngineImpl::buildErrorResponse(aether::OrderConfirmation* confirmation, 
                                const std::string& log_msg, 
                                const std::string& reason)
{
    std::cout << log_msg << std::endl;
    confirmation->set_accepted(false);
    confirmation->set_reason(reason);
    return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, reason);
}


grpc::Status StreamOrderBookStateImpl::StreamOrderBook(grpc::ServerContext* context, 
                                const aether::StreamRequest* request, 
                                grpc::ServerWriter<aether_market_data::OrderDelta>* writer) 
{
    OrderBookState bookState = orderBook->getSnapshot(); 
    for (int i = 0; i < bookState.askPriceLevel.size(); i++) {
        aether_market_data::OrderDelta delta;
        delta.set_action(aether_market_data::DeltaAction::ADD);
        delta.set_side(aether::OrderSide::ASK);
        delta.set_price(bookState.askPriceLevel[i].price);
        delta.set_quantity(bookState.askPriceLevel[i].totalShares);

        writer->Write(delta);
    }

    for (int i = 0; i < bookState.bidsPriceLevel.size(); i++) {
        aether_market_data::OrderDelta delta;
        delta.set_action(aether_market_data::DeltaAction::ADD);
        delta.set_side(aether::OrderSide::BID);
        delta.set_price(bookState.bidsPriceLevel[i].price);
        delta.set_quantity(bookState.bidsPriceLevel[i].totalShares);

        writer->Write(delta);
    }

    while (!context->IsCancelled()) {
        aether_market_data::OrderDelta delta = tradeQueue->pop();
        writer->Write(delta);
    }

    return grpc::Status::OK;
}