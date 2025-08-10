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
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

void RunServer(const std::string& db_path);

class MatchingEngineImpl final : public aether::MatchingEngine::Service
{
public:
    MatchingEngineImpl(const std::string& db_path) : dbPath(db_path) {}
    grpc::Status SubmitOrder(grpc::ServerContext* context, 
                                const aether::OrderRequest* request, 
                                aether::OrderConfirmation* orderConfirmation) override;
    grpc::Status StreamTrades(grpc::ServerContext* context, 
                                const aether::StreamRequest* request, 
                                grpc::ServerWriter<aether::Trade>* writer) override;
    grpc::Status StreamOrderBook(grpc::ServerContext* context, 
                                    const aether::StreamRequest* request, 
                                    grpc::ServerWriter<aether::OrderBookSnapshot>* writer) override;
private:
    OrderBook orderBook;
    std::atomic<uint64_t> orderId {0};
    std::unordered_set<uint64_t> securities;
    const std::string& dbPath;
    std::queue<Trade> trades;
    std::condition_variable dataCond;
    mutable std::mutex mut;
    std::unique_lock lock{mut, std::defer_lock};

    // Helpers
    std::optional<OrderSide> convertToOrderSide(aether::OrderSide& orderSide);
    std::optional<OrderType> convertToOrderType(aether::OrderType& orderType);
    bool isValidSecurity(uint64_t& securityId);
    int loadSecurities();
    grpc::Status buildErrorResponse(aether::OrderConfirmation* confirmation, 
                                    const std::string& log_msg, 
                                    const std::string& reason);
};

#endif  // SERVER_H