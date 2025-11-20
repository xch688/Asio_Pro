//
// Created by xch on 11/20/25.
//

#include "Session.h"
#include "boost/uuid/uuid_generators.hpp"
#include "boost/uuid/uuid_io.hpp"
#include "example/day7/Server.h"
#include "sylar/log.h"

Session::Session(boost::asio::io_context& ctx, Server* server)
    : server_(server)
    , sock_(ctx)
{
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    uuid_ = boost::uuids::to_string(uuid);
}

void Session::start()
{
    memset(data_, 0, sizeof(data_));
    sock_.async_read_some(boost::asio::buffer(data_, MAX_LENGTH),
                          std::bind(&Session::handleRead,
                                    this,
                                    std::placeholders::_1,
                                    std::placeholders::_2,
                                    shared_from_this()));
}

void Session::handleRead(boost::system::error_code ec, std::size_t bytes_transferred,
                         std::shared_ptr<Session> session)
{
    if (ec.value() != 0) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "read error: " << ec.message();
        server_->clearSession(uuid_);
        return;
    }

    SYLAR_LOG_DEBUG(SYLAR_LOG_ROOT()) << "recv: " << std::string(data_, bytes_transferred);
    send(data_, bytes_transferred);
    memset(data_, 0, sizeof(data_));
    sock_.async_read_some(boost::asio::buffer(data_, MAX_LENGTH),
                          std::bind(&Session::handleRead,
                                    this,
                                    std::placeholders::_1,
                                    std::placeholders::_2,
                                    shared_from_this()));
}

void Session::send(char* msg, int len)
{
    std::lock_guard<std::mutex> lock(mtx_);
    sendQueue_.push(std::make_shared<MsgNode>(msg, len));
    if (sendQueue_.size() >= 2) {
        return;
    }

    boost::asio::async_write(
        sock_,
        boost::asio::buffer(msg, len),
        std::bind(&Session::handleWrite, this, std::placeholders::_1, shared_from_this()));
}

void Session::handleWrite(boost::system::error_code ec, std::shared_ptr<Session> self)
{
    if (ec.value() != 0) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "handleWrite error: " << ec.message();
        server_->clearSession(uuid_);
        return;
    }

    std::lock_guard<std::mutex> lock(mtx_);
    sendQueue_.pop();
    if (!sendQueue_.empty()) {
        auto& node = sendQueue_.front();
        boost::asio::async_write(
            sock_,
            boost::asio::buffer(node->data, node->maxLen),
            std::bind(&Session::handleWrite, this, std::placeholders::_1, shared_from_this()));
    }
}