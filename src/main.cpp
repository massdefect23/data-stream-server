#include <iostream>
#include <thread>
#include "config.h"
#include "market_server.h"
#include "momentum_server.h"

int main(int argc, char* argv[]) {
    // 1. Load .env config
    std::string envFile = ".env";
    if (argc > 1) {
        envFile = argv[1];
    }
    AppConfig config = loadConfig(envFile);

    // 2. Create server objects
    MarketDataServer marketServer(config.marketDataPort, config);
    MomentumDataServer momentumServer(config.momentumDataPort, config);

    // 3. Start each server in its own thread
    std::thread marketThread([&marketServer]() {
        marketServer.start(); // blocks
    });

    std::thread momentumThread([&momentumServer]() {
        momentumServer.start(); // blocks
    });

    std::cout << "[Main] Both servers are running in separate threads.\n"
              << "Market data port: " << config.marketDataPort << "\n"
              << "Momentum data port: " << config.momentumDataPort << "\n";

    // 4. Wait for them to finish (they never will in this example)
    marketThread.join();
    momentumThread.join();

    return 0;
}