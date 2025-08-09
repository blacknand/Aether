#ifndef SERVER_H
#define SERVER_H

#include "../core/OrderBook.h"
#include "order.grpc.pb.h"

#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

// using grpc::CallbackServerContext;
// using grpc::Server;
// using grpc::ServerBuilder;
// using grpc::Status;
// using routeguide::Feature;
// using routeguide::Point;
// using routeguide::Rectangle;
// using routeguide::RouteGuide;
// using routeguide::RouteNote;
// using routeguide::RouteSummary;
// using std::chrono::system_clock;

#include <iostream>
#include <chrono>
#include <memory>
#include <atomic>
#include <vector>
#include <optional>
#include <fstream>
#include <sstream>
#include <unordered_set>

class MatchingEngineImpl final : public aether::MatchingEngine::Service
{
public:
    grpc::Status SubmitOrder(grpc::ServerContext* context, 
                                aether::OrderRequest* request, 
                                aether::OrderConfirmation* orderConfirmation) override;
    grpc::Status StreamTrades(grpc::ServerContext* context, 
                                aether::StreamRequest* request, 
                                aether::Trade* trade, 
                                grpc::ServerWriter<aether::Trade>* writer) override;
    grpc::Status StreamOrderBook(grpc::ServerContext* context, 
                                    aether::StreamRequest* request, 
                                    aether::OrderBookSnapshot* snapshot,
                                    grpc::ServerWriter<aether::OrderBookSnapshot>* writer) override;
    void RunServer(const std::string& db_path);
private:
    OrderBook orderBook;
    std::atomic<uint64_t> orderId {0};
    std::optional<OrderSide> convertToOrderSide(aether::OrderSide& orderSide);
    std::optional<OrderType> convertToOrderType(aether::OrderSide& orderType);
    bool isValidSecurity(uint64_t securityId&);
    std::unordered_set<uint64_t> securities;
    int loadSecurities();
};

#endif  // SERVER_H