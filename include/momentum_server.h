#ifndef MOMENTUM_SERVER_H
#define MOMENTUM_SERVER_H

#include "server_base.h"
#include "config.h"
#include <thread>
#include <atomic>

struct AppConfig;

class MomentumDataServer : public BaseServer {
public:
    MomentumDataServer(unsigned short port,
                       const AppConfig& config);
    ~MomentumDataServer();

    void start();

protected:
    void onNewConnection(std::shared_ptr<BaseSession> session) override;

private:
    void pollMomentumData();

private:
    AppConfig config_;
    std::thread pollingThread_;
    std::atomic<bool> running_{true};
};

#endif // MOMENTUM_SERVER_H