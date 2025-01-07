#include "market_server.h"
#include "config.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <libpq-fe.h>

MarketDataServer::MarketDataServer(unsigned short port,
                                   const AppConfig& config)
    : BaseServer(port)
    , config_(config)
{
}

MarketDataServer::~MarketDataServer() {
    running_ = false;
    if (pollingThread_.joinable()) {
        pollingThread_.join();
    }
}

void MarketDataServer::start() {
    // Start polling thread
    pollingThread_ = std::thread(&MarketDataServer::pollMarketData, this);

    // Run the server (blocking) in the current thread
    BaseServer::run();
}

void MarketDataServer::onNewConnection(std::shared_ptr<BaseSession> session) {
    std::cout << "[MarketDataServer] New client connected for market data.\n";
    // You could send an immediate snapshot here if desired
}

void MarketDataServer::pollMarketData() {
    // Connect to PostgreSQL
    std::string connInfo = "host=" + config_.dbHost +
                           " port=" + std::to_string(config_.dbPort) +
                           " dbname=" + config_.dbName +
                           " user=" + config_.dbUser +
                           " password=" + config_.dbPassword;

    PGconn* conn = PQconnectdb(connInfo.c_str());
    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "[MarketDataServer] Failed to connect to DB: "
                  << PQerrorMessage(conn) << std::endl;
        PQfinish(conn);
        return;
    }
    std::cout << "[MarketDataServer] Connected to PostgreSQL.\n";

    // Example: poll in a loop
    while (running_) {
        // Example query: get some market data (e.g., last trades or quotes)
        // In real life: you'd use timestamps or conditions for updated rows
        const char* query = R"(
            SELECT token_id, 
                price_usd, 
                fdv_usd, 
                volume_usd_h24, 
                total_reserve_usd, 
                timestamp_utc
            FROM token_market_data 
            ORDER BY timestamp_utc DESC
            LIMIT 1
        )";
                             
        PGresult* res = PQexec(conn, query);

        if (PQresultStatus(res) == PGRES_TUPLES_OK) {
            int nrows = PQntuples(res);
            for (int i = 0; i < nrows; i++) {
                std::string token_id           = PQgetvalue(res, i, 0);
                std::string price              = PQgetvalue(res, i, 1);
                std::string fdv_usd            = PQgetvalue(res, i, 2);
                std::string volume_usd_h24     = PQgetvalue(res, i, 3);
                std::string total_reserve_usd  = PQgetvalue(res, i, 4);
                std::string timestamp_utc      = PQgetvalue(res, i, 5);

                // Construct JSON (or whatever format) to broadcast
                std::string msg = 
                    R"({"type":"market","token_id":")" + token_id +
                    R"(","price_usd":)" + price +
                    R"(,"fdv_usd":)" + fdv_usd +
                    R"(,"volume_usd_h24":)" + volume_usd_h24 +
                    R"(,"total_reserve_usd":)" + total_reserve_usd +
                    R"(,"timestamp_utc":")" + timestamp_utc + 
                    R"("})";
                broadcast(msg);
            }
        } else {
            std::cerr << "[MarketDataServer] Query error: "
                      << PQerrorMessage(conn) << std::endl;
        }

        PQclear(res);

        // Sleep for a bit (e.g. 1 second or 1 minute, depending on frequency)
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    PQfinish(conn);
    std::cout << "[MarketDataServer] Disconnected from PostgreSQL.\n";
}