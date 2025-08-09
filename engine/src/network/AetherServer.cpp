#include "AetherServer.h"


grpc::Status MatchingEngineImpl::SubmitOrder(ServerContext* context, 
                                                aether::OrderRequest* request, 
                                                aether::OrderConfirmation* orderConfirmation) override
{
    uint64_t new_id = ++orderId;
    uint64_t securityId = request->security_id;
    Order::OrderSide side = request->side;
    Order::OrderType type = request->type;
    uint32_t quantity = request->quantity;
    uint64_t price = request->price;
    uint64_t time = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count())
    Order order = {new_id, securityId, side, type, quantity, price, time};
    std::vector<Trade> trades = orderBook.addOrder(order);
    orderConfirmation->order_id = new_id;
    orderConfirmation->accepted = !trades.empty() : true ? false;
    orderConfirmation->reason = !trades.empty() : "Accepted" ? "No matching orders";
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