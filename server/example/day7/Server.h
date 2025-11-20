#pragma once

#include "Session.h"
#include "boost/asio.hpp"
#include <map>

class Server {
public:
    Server(boost::asio::io_context& ctx, uint16_t port);

    bool init();
    void startAccept();
    void clearSession(std::string uuid);

private:
    void handleAccept(std::shared_ptr<Session> new_session, const boost::system::error_code& ec);

private:
    uint16_t port_ = 0;
    boost::asio::io_context& ctx_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::map<std::string, std::shared_ptr<Session>> sessions_;
};