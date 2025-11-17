//
// Created by xch on 11/17/25.
//

#include "Server.h"
#include "sylar/log.h"

Server::Server(uint16_t port)
    : port_(port)
    , ctx_()
    , acceptor_(ctx_)
{}

Server::~Server()
{
    for (auto& thr : threads) {
        if (thr->joinable()) {
            thr->join();
        }
    }
}

bool Server::init()
{
    boost::system::error_code ec;
    if (acceptor_.open(boost::asio::ip::tcp::v4(), ec)) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "acceptor open error: " << ec.message();
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
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "acceptor listen error: " << ec.message();
        return false;
    }

    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "Server init on port " << port_ << " success";

    return true;
}

void Server::start()
{
    while (true) {
        boost::asio::ip::tcp::endpoint peer;
        boost::system::error_code ec;
        boost::asio::ip::tcp::socket clientSock = acceptor_.accept(peer, ec);
        if (ec) {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "accept error: " << ec.message();
            continue;
        }
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT())
            << "accept a new client; ip: " << peer.address().to_string()
            << "; port: " << peer.port();

        auto thr = std::make_shared<std::thread>(&Server::threadFunc, this, std::move(clientSock));
        threads.insert(thr);
    }
}

void Server::threadFunc(boost::asio::ip::tcp::socket sock)
{
    try {
        while (true) {
            char data[1024] = {0};
            boost::system::error_code ec;
            size_t byteRead = sock.read_some(boost::asio::buffer(data, 1024), ec);
            if (ec) {
                if (ec == boost::asio::error::eof) {
                    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "Connection closed by peer";
                    sock.close();
                    break;
                }
                else {
                    throw boost::system::system_error(ec);
                }
            }

            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "recv: " << std::string(data, byteRead);
            sock.send(boost::asio::buffer(data, byteRead), 0, ec);
            if (ec) {
                if (ec == boost::asio::error::eof) {
                    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "Connection closed by peer";
                    sock.close();
                    break;
                }
                else {
                    throw boost::system::system_error(ec);
                }
            }
        }
    }
    catch (std::exception& ex) {
        SYLAR_LOG_FATAL(SYLAR_LOG_ROOT()) << "Server threadFunc exception: " << ex.what();
    }
}