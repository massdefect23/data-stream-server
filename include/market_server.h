#ifndef MARKET_SERVER_H
#define MARKET_SERVER_H

#include "server_base.h"
#include "config.h"
#include <string>
#include <thread>
#include <atomic>

struct AppConfig;

class MarketDataServer : public BaseServer {
public:
    MarketDataServer(unsigned short port,
                     const AppConfig& config);
    ~MarketDataServer();

    // Start the server and also start polling DB in a separate thread
    void start();

protected:
    // Called whenever a new client connects
    void onNewConnection(std::shared_ptr<BaseSession> session) override;

private:
    void pollMarketData();

private:
    AppConfig config_;
    std::thread pollingThread_;
    std::atomic<bool> running_{true};
};

#endif // MARKET_SERVER_H