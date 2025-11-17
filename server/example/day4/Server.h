#pragma once
#include <boost/asio.hpp>
#include <cstdint>
#include <set>

class Server {
public:
    explicit Server(uint16_t port);
    ~Server();
    bool init();
    void start();

private:
    void threadFunc(boost::asio::ip::tcp::socket sock);

private:
    uint16_t port_;
    boost::asio::io_context ctx_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::set<std::shared_ptr<std::thread>> threads;
};