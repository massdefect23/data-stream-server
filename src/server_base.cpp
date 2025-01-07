#include "server_base.h"
#include <iostream>
#include <boost/asio/dispatch.hpp>

//---------------------------
// BaseServer
//---------------------------
BaseServer::BaseServer(unsigned short port)
    : ioc_()
    , acceptor_(ioc_, {boost::asio::ip::tcp::v4(), port})
{
    std::cout << "[BaseServer] Listening on port " << port << std::endl;
}

BaseServer::~BaseServer() {
    ioc_.stop();
}

void BaseServer::run() {
    doAccept();
    ioc_.run();
}

void BaseServer::doAccept() {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket){
            if (!ec) {
                auto session = std::make_shared<BaseSession>(std::move(socket), *this);
                {
                    std::lock_guard<std::mutex> lock(sessionsMutex_);
                    sessions_.push_back(session);
                }
                // Let derived classes know about new session if they want
                onNewConnection(session);

                session->start();
            }
            // Continue accepting new clients
            doAccept();
        }
    );
}

void BaseServer::broadcast(const std::string& msg) {
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    for (auto& session : sessions_) {
        session->send(msg);
    }
}

//---------------------------
// BaseSession
//---------------------------
BaseSession::BaseSession(boost::asio::ip::tcp::socket socket, BaseServer& server)
    : ws_(std::move(socket))
    , server_(server)
{}

void BaseSession::start() {
    ws_.async_accept(
        [self = shared_from_this()](boost::beast::error_code ec) {
            self->onAccept(ec);
        }
    );
}

void BaseSession::onAccept(boost::beast::error_code ec) {
    if(ec) {
        std::cerr << "[BaseSession] Accept error: " << ec.message() << std::endl;
        return;
    }
    doRead();
}

void BaseSession::doRead() {
    auto self = shared_from_this();
    ws_.async_read(
        buffer_,
        [self](boost::beast::error_code ec, std::size_t bytesTransferred) {
            self->onRead(ec, bytesTransferred);
        }
    );
}

void BaseSession::onRead(boost::beast::error_code ec, std::size_t bytesTransferred) {
    boost::ignore_unused(bytesTransferred);

    if(ec == boost::beast::websocket::error::closed) {
        // Client closed the connection
        return;
    }

    if(ec) {
        std::cerr << "[BaseSession] Read error: " << ec.message() << std::endl;
        return;
    }

    // Optionally echo the message back or handle it differently
    ws_.text(true);
    ws_.async_write(
        buffer_.data(),
        [self = shared_from_this()](boost::beast::error_code writeEc, std::size_t){
            self->onWrite(writeEc, 0);
        }
    );
}

void BaseSession::onWrite(boost::beast::error_code ec, std::size_t bytesTransferred) {
    boost::ignore_unused(bytesTransferred);
    if(ec) {
        std::cerr << "[BaseSession] Write error: " << ec.message() << std::endl;
        return;
    }

    // Clear buffer
    buffer_.consume(buffer_.size());

    // Keep reading messages
    doRead();
}

void BaseSession::send(const std::string& msg) {
    auto self = shared_from_this();
    boost::asio::dispatch(ws_.get_executor(),
        [self, msg]() {
            self->ws_.text(true);
            self->ws_.async_write(
                boost::asio::buffer(msg),
                [self](boost::beast::error_code ec, std::size_t /*bytes*/) {
                    if(ec) {
                        std::cerr << "[BaseSession] Broadcast write error: "
                                  << ec.message() << std::endl;
                    }
                }
            );
        }
    );
}