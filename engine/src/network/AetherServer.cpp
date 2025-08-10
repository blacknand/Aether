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

    if (!side.has_value() || !type.has_value()) {
        std::cout << "[gRPC-SERVER] Invalid order side or invalid order type provided. Rejecting order." << std::endl;
        orderConfirmation->set_accepted(false);
        orderConfirmation->set_reason("Invalid order side or invalid order type.");
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Invalid order side or invalid order type.");
    }

    if (type.has_value() && type.value() == OrderType::LIMIT && !price.has_value()) {
        std::cout << "[gRPC-SERVER] Limit order with no price provided. Rejecting order." << std::endl;
        orderConfirmation->set_accepted(false);
        orderConfirmation->set_reason("A limit order must have a price provided.");
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Invalid limit order with no price");
    }

    if (quantity <= 0) {
        std::cout << "[gRPC-SERVER] An order with an invalid non-positive quantity of " << quantity << " has been placed. Rejecting order." << std::endl;
        orderConfirmation->set_accepted(false);
        orderConfirmation->set_reason("An order with an invalid non-positive quantity of " + std::to_string(quantity) + " has been placed.");
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Invalid quantity.");
    } 

    if (type.value() == OrderType::LIMIT && price.value() <= 0) {
        std::cout << "[gRPC-SERVER] A limit order with an invalid non-positive price of " << price.value() << " has been placed. Rejecting order." << std::endl;
        orderConfirmation->set_accepted(false);
        orderConfirmation->set_reason("A limit order with an invalid non-positive price of " + std::to_string(price.value()) + " has been placed.");
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Invalid price for LIMIT order.");
    }

    if (type == OrderType::MARKET && price.has_value()) {
        std::cout << "[gRPC-SERVER] A market order with a specified price has been placed. Rejecting order." << std::endl;
        orderConfirmation->set_accepted(false);
        orderConfirmation->set_reason("A market order with a specified price has been placed.");
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Market order cannot have a specified price.");
    }

    if (!isValidSecurity(securityId)) {
        std::cout << "[gRPC-SERVER] Invalid security ID. Rejecting order." << std::endl;
        orderConfirmation->set_accepted(false);
        orderConfirmation->set_reason("Invalid security ID. Rejecting order.");
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Invalid security ID.");
    }

    Order order{
        .orderId = new_id, 
        .securityId = securityId, 
        .side = side.value(), 
        .type = type.value(), 
        .quantity = quantity, 
        .price = price, 
        .timestamp = time};
    std::optional<std::vector<Trade>> trades = orderBook.addOrder(order);
    orderConfirmation->set_order_id(new_id);
    orderConfirmation->set_accepted(true);
    orderConfirmation->set_reason("Valid order. All checks passed.");
    return grpc::Status::OK;
}


grpc::Status MatchingEngineImpl::StreamTrades(grpc::ServerContext* context, 
                                                const aether::StreamRequest* request, 
                                                grpc::ServerWriter<aether::Trade>* Writer) 
{
    return grpc::Status::OK;
}


grpc::Status MatchingEngineImpl::StreamOrderBook(grpc::ServerContext* context, 
                                                    const aether::StreamRequest* request, 
                                                    grpc::ServerWriter<aether::OrderBookSnapshot>* Writer) 
{
    return grpc::Status::OK;
}


void RunServer(const std::string& db_path)
{
    std::string server_address("0.0.0.0:50051");    // NOTE: will probably need to change
    MatchingEngineImpl service(db_path);
    service.loadSecurities();
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "gRPC Aether Server listening on " << server_address << std::endl;
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


bool MatchingEngineImpl::isValidSecurity(uint64_t& securityId) 
{
    return securities.contains(securityId);
}


int MatchingEngineImpl::loadSecurities()
{
    std::ifstream file("../../../engine_config/securities.csv");
    if (!file.is_open()) {
        std::cerr << "[gRPC-SERVER] Could not open securities.csv file" << std::endl;
        return -1;
    }

    std::string line;
    while (std::getline(file, line)) {
        size_t commaPos = line.find(',');
        if (commaPos != std::string::npos) {
            uint64_t securityId = std::stoull(line.substr(0, commaPos));
            securities.insert(securityId);
        }
    }
    file.close();
    return 0;
}