#include "AetherServer.h"


grpc::Status MatchingEngineImpl::SubmitOrder(ServerContext* context, 
                                                aether::OrderRequest* request, 
                                                aether::OrderConfirmation* orderConfirmation) override
{
    uint64_t new_id = ++orderId;
    uint64_t securityId = request->security_id;
    Order::OrderSide side = convertToOrderSide(request->side);
    Order::OrderType type = convertToOrderType(request->type);
    uint32_t quantity = request->quantity;
    uint64_t price = request->price;
    uint64_t time = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count());

    if (quantity <= 0) {
        std::cout << "[gRPC-SERVER] An order with an invalid non-positive quantity of " << quantity << " has been placed. Rejecting order." << std::endl;
        orderConfirmation->accepted = false;
        orderConfirmation->reason = "An order with an invalid non-positive quantity of " << quantity << " has been placed.";
        return grpc::Status::(grpc::StatusCode::INVALID_ARGUMENT, "Invalid quantity.");
    } 

    if (type == Order::OrderType::LIMIT && price <= 0) {
        std::cout << "[gRPC-SERVER] A limit order with an invalid non-positive price of " << price << " has been placed. Rejecting order." << std::endl;
        orderConfirmation->accepted = false;
        orderConfirmation->reason = "A limit order with an invalid non-positive price of " << price << " has been placed.";
        return grpc::Status::(grpc::StatusCode::INVALID_ARGUMENT, "Invalid price for LIMIT order.");
    }

    if (type == OrderType::MARKET && price != 0) {
        std::cout << "[gRPC-SERVER] A market order with a specified price has been placed. Rejecting order." << std::endl;
        orderConfirmation->accepted = false;
        orderConfirmation->reason = "A market order with a specified price has been placed.";
        return grpc::Status::(grpc::StatusCode::INVALID_ARGUMENT, "Market order cannot have a specified price.");
    }

    if (!isValidSecurity(securityId)) {
        std::cout << "[gRPC-SERVER] Invalid security ID. Rejecting order." << std::endl;
        orderConfirmation->accepted = false;
        orderConfirmation->reason = "Invalid security ID. Rejecting order.";
        return grpc::Status::(grpc::StatusCode::INVALID_ARGUMENT, "Invalid security ID.");
    }

    Order order = {new_id, securityId, side, type, quantity, price, time};
    std::vector<Trade> trades = orderBook.addOrder(order);
    orderConfirmation->order_id = new_id;
    orderConfirmation->accepted = true;
    orderConfirmation->reason = "Valid order. All checks passed.";
    return grpc::Status::OK;
}


grpc::Status MatchingEngineImpl::StreamTrades(ServerContext* context, 
                                                aether::StreamRequest* request, 
                                                aether::Trade* trade,
                                                grpc::ServerWriter<aether::Trade>* Writer) override
{
    return grpc::Status::OK;
}


grpc::Status MatchingEngineImpl::StreamOrderBook(ServerContext* context, 
                                                    aether::StreamRequest* request, 
                                                    aether::OrderBookSnapshot* snapshot,
                                                    grpc::ServerWriter<aether::OrderBookSnapshot>* Writer) override
{
    return grpc::Status::OK;
}


void MatchingEngineImpl::RunServer(const std::string& db_path)
{
    std::string server_address("0.0.0.0:50051");    // NOTE: will probably need to change
    MatchingEngineImpl service(db_path);
    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "gRPC Aether Server listening on " << server_address << std::endl;
    server->Wait();
}


std::optional<OrderSide> MatchingEngineImpl::convertToOrderSide(aether::OrderSide& orderSide)
{
    Order::OrderSide res;
    if (orderSide == aether::OrderSide::BID) {
        res = Order::OrderSide::BID;
    } else if (orderSide == aether::OrderSide::ASK) {
        res = Order::OrderSide::ASK;
    } else {
        std::cout << "aether::OrderSide::SIDE_UNKWON" << std::endl;
        return;
    }
    return res;
}


std::optional<OrderType> MatchingEngineImpl::convertToOrderType(aether::OrderSide& orderType)
{

}


bool isValidSecurity(uint64_t& securityId) 
{

}