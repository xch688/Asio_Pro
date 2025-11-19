#pragma once

#include "boost/asio.hpp"

class Session {
public:
    explicit Session(boost::asio::io_context& ctx)
        : sock_(ctx)
        , data_{0}
    {}

    void start();
    boost::asio::ip::tcp::socket& socket() { return sock_; }

private:
    void handle_read(const boost::system::error_code& ec, std::size_t bytes_transferred);
    void handle_write(const boost::system::error_code& ec);

private:
    boost::asio::ip::tcp::socket sock_;
    enum
    {
        max_length = 1024
    };
    char data_[max_length];
};


class Server {
public:
    Server(boost::asio::io_context& ctx, uint16_t port);
    void init();
    void start_accept();

private:
    void handle_accept(Session* new_session, const boost::system::error_code& error);

private:
    uint16_t port_;
    boost::asio::io_context& ctx_;
    boost::asio::ip::tcp::acceptor acceptor_;
};