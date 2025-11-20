//
// Created by xch on 11/20/25.
//

#include "Server.h"
#include "sylar/log.h"

Server::Server(boost::asio::io_context& ctx, uint16_t port)
    : port_(port)
    , ctx_(ctx)
    , acceptor_(ctx)
{}


bool Server::init()
{
    boost::system::error_code ec;
    if (acceptor_.open(boost::asio::ip::tcp::v4(), ec)) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "acceptor open error: " << ec.message();
        return false;
    }
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    boost::asio::ip::address addr = boost::asio::ip::address_v4().any();
    boost::asio::ip::tcp::endpoint ep(addr, port_);
    if (acceptor_.bind(ep, ec)) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "acceptor bind error: " << ec.message();
        return false;
    }

    if (acceptor_.listen(boost::asio::socket_base::max_listen_connections, ec)) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "acceptor listen error: " << ec.message();
        return false;
    }
    return true;
}


void Server::startAccept()
{
    std::shared_ptr<Session> new_session = std::make_shared<Session>(ctx_, this);
    acceptor_.async_accept(
        new_session->getSocket(),
        std::bind(&Server::handleAccept, this, new_session, std::placeholders::_1));
}

void Server::handleAccept(std::shared_ptr<Session> new_session, const boost::system::error_code& ec)
{
    if (ec.value() == 0) {
        SYLAR_LOG_DEBUG(SYLAR_LOG_ROOT())
            << "Accept a new client, ip : "
            << new_session->getSocket().remote_endpoint().address().to_string()
            << "; port: " << new_session->getSocket().remote_endpoint().port();
        new_session->start();
    }
    else {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "accept failed: " << ec.message();
    }

    // 再次监听客户端的连接
    startAccept();
}

void Server::clearSession(std::string uuid)
{
    sessions_.erase(uuid);
}