#ifndef SERVER_BASE_H
#define SERVER_BASE_H

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>
#include <mutex>
#include <mutex>
#include <vector>
#include <string>

// Forward declaration
class BaseSession;

class BaseServer
{
    public:
        BaseServer(unsigned short port);
        virtual ~BaseServer();

        // Start listening (blocking call)
        void run();

        // Broadcast message to connected clients 
        void broadcast(const std::string& msg);

    protected:
        // For derived classes: override this if needed to handle new connections
        virtual void onNewConnection(std::shared_ptr<BaseSession>) {}

    private:
        void doAccept();

        boost::asio::io_context ioc_;
        boost::asio::ip::tcp::acceptor acceptor_;

        std:: vector<std::shared_ptr<BaseSession>> sessions_;
        std::mutex  sessionsMutex_;

        friend class BaseSession;
};


// Each connected client 
class BaseSession : public std::enable_shared_from_this<BaseSession>
{
    public:
        BaseSession(boost::asio::ip::tcp::socket socket, BaseServer& server);
        virtual ~BaseSession() = default;

        void start();
        void send(const std::string& msg);

    private:
        void onAccept(boost::beast::error_code ec);
        void doRead();
        void onRead(boost::beast::error_code ec, std::size_t bytesTransferred);
        void onWrite(boost::beast::error_code ec, std::size_t bytesTransferred);

    protected:
        boost::beast::websocket::stream<boost::asio::ip::tcp::socket> ws_;
        BaseServer& server_;
        boost::beast::flat_buffer buffer_;
};

#endif // SERVER_BASE_H
