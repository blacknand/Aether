#ifndef SERVER_H
#define SERVER_H

#include "../core/OrderBook.h"
#include "../utils/BlockingQueue.h"
#include "order_management.grpc.pb.h"
#include "market_data.grpc.pb.h"

#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

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

void RunServer();

class MatchingEngineImpl final : public aether::MatchingEngine::Service
{
public:
    MatchingEngineImpl(std::shared_ptr<OrderBook> orderBook_) : orderBook(orderBook_) {}
    grpc::Status SubmitOrder(grpc::ServerContext* context, 
                                const aether::OrderRequest* request, 
                                aether::OrderConfirmation* orderConfirmation) override;
    grpc::Status StreamTrades(grpc::ServerContext* context, 
                                const aether::StreamRequest* request, 
                                grpc::ServerWriter<aether::Trade>* writer) override;
private:
    std::shared_ptr<OrderBook> orderBook;
    std::atomic<uint64_t> orderId {0};
    // std::unordered_set<uint64_t> securities;
    // const std::string& securitiesPath;
    std::queue<Trade> tradesQ;
    std::condition_variable tradesReady;
    mutable std::mutex streamTradeMut;

    // Helpers
    std::optional<OrderSide> convertToOrderSide(aether::OrderSide& orderSide);
    std::optional<OrderType> convertToOrderType(aether::OrderType& orderType);
    grpc::Status buildErrorResponse(aether::OrderConfirmation* confirmation, 
                                    const std::string& log_msg, 
                                    const std::string& reason);
};


class StreamOrderBookStateImpl final : public aether_market_data::StreamOrderBookState::Service
{
public:
    StreamOrderBookStateImpl(std::shared_ptr<BlockingQueue<aether_market_data::OrderDelta>> blockingQueue, std::shared_ptr<OrderBook> orderBook_) : 
                                                                                                                tradeQueue(blockingQueue),
                                                                                                                orderBook(orderBook_) {}
    grpc::Status StreamOrderBook(grpc::ServerContext* context, 
                                    const aether::StreamRequest* request, 
                                    grpc::ServerWriter<aether_market_data::OrderDelta>* writer) override;

private:
    std::shared_ptr<BlockingQueue<aether_market_data::OrderDelta>> tradeQueue;
    std::shared_ptr<OrderBook> orderBook;
};

#endif  // SERVER_H