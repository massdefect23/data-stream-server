#include "momentum_server.h"
#include "config.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <libpq-fe.h>

MomentumDataServer::MomentumDataServer(unsigned short port,
                                       const AppConfig& config)
    : BaseServer(port)
    , config_(config)
{
}

MomentumDataServer::~MomentumDataServer() {
    running_ = false;
    if (pollingThread_.joinable()) {
        pollingThread_.join();
    }
}

void MomentumDataServer::start() {
    // Start polling thread
    pollingThread_ = std::thread(&MomentumDataServer::pollMomentumData, this);

    // Run server in current thread
    BaseServer::run();
}

void MomentumDataServer::onNewConnection(std::shared_ptr<BaseSession> session) {
    std::cout << "[MomentumDataServer] New client connected for momentum data.\n";
    // Possibly send an immediate snapshot or historical momentum data
}

void MomentumDataServer::pollMomentumData() {
    // Connect to DB
    std::string connInfo = "host=" + config_.dbHost +
                           " port=" + std::to_string(config_.dbPort) +
                           " dbname=" + config_.dbName +
                           " user=" + config_.dbUser +
                           " password=" + config_.dbPassword;

    PGconn* conn = PQconnectdb(connInfo.c_str());
    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "[MomentumDataServer] Failed to connect to DB: "
                  << PQerrorMessage(conn) << std::endl;
        PQfinish(conn);
        return;
    }
    std::cout << "[MomentumDataServer] Connected to PostgreSQL.\n";

    // Example loop for momentum data
    while (running_) {
        // Example query: maybe you have a "momentum_signals" table
        const char* query = "SELECT token_id, momentum FROM token_momentum LIMIT 5;";
        PGresult* res = PQexec(conn, query);

        if (PQresultStatus(res) == PGRES_TUPLES_OK) {
            int nrows = PQntuples(res);
            for (int i = 0; i < nrows; i++) {
                std::string token_id            = PQgetvalue(res, i, 0);
                std::string momentum            = PQgetvalue(res, i, 1);

                std::string msg = R"({"type":"momentum","token_id":")" + token_id +
                                  R"(","momentum":)" + momentum + "}";
                broadcast(msg);
            }
        } else {
            std::cerr << "[MomentumDataServer] Query error: "
                      << PQerrorMessage(conn) << std::endl;
        }

        PQclear(res);

        // Sleep for some interval (could be different from market data)
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    PQfinish(conn);
    std::cout << "[MomentumDataServer] Disconnected from PostgreSQL.\n";
}