#pragma once

#include <boost/asio.hpp>
#include <queue>

#define MAX_LENGTH 1024

class Server;

struct MsgNode
{
public:
    MsgNode(char* msg, int len)
    {
        data = new char[len];
        memcpy(data, msg, len);
    }

    ~MsgNode() { delete[] data; }

public:
    int curLen;
    int maxLen;
    char* data;
};

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(boost::asio::io_context& ctx, Server* server);

    boost::asio::ip::tcp::socket& getSocket() { return sock_; }
    std::string& getUUid() { return uuid_; }

    void start();
    void send(char* msg, int len);

private:
    void handleRead(boost::system::error_code ec, std::size_t bytes_transferred,
                    std::shared_ptr<Session> session);
    void handleWrite(boost::system::error_code ec, std::shared_ptr<Session> self);

private:
    std::string uuid_;
    Server* server_;
    boost::asio::ip::tcp::socket sock_;

    char data_[MAX_LENGTH];
    std::mutex mtx_;
    std::queue<std::shared_ptr<MsgNode>> sendQueue_;
};
