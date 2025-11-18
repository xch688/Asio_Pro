#include "boost/asio.hpp"
#include "sylar/log.h"

#include <functional>
#include <queue>

namespace ItemX1 {

constexpr int RECV_SIZE = 1024;
struct MsgNode
{
public:
    MsgNode(const char* msg, int totalLen)
        : totalLen_(totalLen)
        , curLen_(0)
    {
        msg_ = new char[totalLen];
        memset(msg_, 0, totalLen);
    }

    explicit MsgNode(const int totalLen)
        : totalLen_(totalLen)
        , curLen_(0)
    {
        msg_ = new char[totalLen];
        memset(msg_, 0, totalLen);
    }

    ~MsgNode()
    {
        delete[] msg_;
        msg_ = nullptr;
    }

public:
    char* msg_;
    std::size_t totalLen_;
    std::size_t curLen_;
};


class Session {
public:
    void writeToSocketErr(const std::string& buf);
    void writeToSocketErrCallback(boost::system::error_code ec, std::size_t bytes_transferred,
                                  std::shared_ptr<MsgNode>);

private:
    std::shared_ptr<MsgNode> sendNode_;
    std::shared_ptr<boost::asio::ip::tcp::socket> sock_;
};

void Session::writeToSocketErr(const std::string& buf)
{
    sendNode_ = std::make_shared<MsgNode>(buf.c_str(), buf.size());
    sock_->async_write_some(boost::asio::buffer(buf.data(), buf.size()),
                            std::bind(&Session::writeToSocketErrCallback,
                                      this,
                                      std::placeholders::_1,
                                      std::placeholders::_2,
                                      sendNode_));
}


void Session::writeToSocketErrCallback(boost::system::error_code ec, std::size_t bytes_transferred,
                                       std::shared_ptr<MsgNode> sendNode)
{
    if (ec) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "write error: " << ec.message();
        return;
    }

    if (bytes_transferred + sendNode->curLen_ < sendNode->totalLen_) {
        sendNode->curLen_ += bytes_transferred;
        sock_->async_write_some(boost::asio::buffer(sendNode->msg_ + sendNode->curLen_,
                                                    sendNode->totalLen_ - sendNode->curLen_),
                                std::bind(&Session::writeToSocketErrCallback,
                                          this,
                                          std::placeholders::_1,
                                          std::placeholders::_2,
                                          sendNode_));
    }
}

}   // namespace ItemX1








namespace ItemX2 {

constexpr int RECV_SIZE = 1024;
struct MsgNode
{
public:
    MsgNode(const char* msg, int totalLen)
        : totalLen_(totalLen)
        , curLen_(0)
    {
        msg_ = new char[totalLen];
        memset(msg_, 0, totalLen);
    }

    explicit MsgNode(const int totalLen)
        : totalLen_(totalLen)
        , curLen_(0)
    {
        msg_ = new char[totalLen];
        memset(msg_, 0, totalLen);
    }

    ~MsgNode()
    {
        delete[] msg_;
        msg_ = nullptr;
    }

public:
    char* msg_;
    std::size_t totalLen_;
    std::size_t curLen_;
};


class Session {
public:
    void writeToSocket(const std::string& buf);
    void writeToSocketCallback(boost::system::error_code ec, std::size_t bytes_transferred);

private:
    bool sendPending_;
    std::queue<std::shared_ptr<MsgNode>> sendQueue_;
    std::shared_ptr<boost::asio::ip::tcp::socket> sock_;
};

void Session::writeToSocket(const std::string& buf)
{
    sendQueue_.emplace(new MsgNode(buf.data(), buf.size()));
    // 若当前正在发送数据，则直接返回，等待当前发送完成后的回调继续发送
    if (sendPending_) {
        return;
    }

    sock_->async_write_some(
        boost::asio::buffer(buf.data(), buf.size()),
        std::bind(
            &Session::writeToSocketCallback, this, std::placeholders::_1, std::placeholders::_2));
    sendPending_ = true;
}


void Session::writeToSocketCallback(boost::system::error_code ec, std::size_t bytes_transferred)
{
    if (ec.value() != 0) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "write error: " << ec.message();
        return;
    }

    auto& curNode = sendQueue_.front();
    curNode->curLen_ += bytes_transferred;
    // 当前Node还存在数据没有发送完成，则应该继续该Node的数据发送
    if (curNode->curLen_ < curNode->totalLen_) {
        sock_->async_write_some(boost::asio::buffer(curNode->msg_ + curNode->curLen_,
                                                    curNode->totalLen_ - curNode->curLen_),
                                std::bind(&Session::writeToSocketCallback,
                                          this,
                                          std::placeholders::_1,
                                          std::placeholders::_2));
        return;
    }

    sendQueue_.pop();
    // 所有的数据发送完成，则结束数据的发送
    if (sendQueue_.empty()) {
        sendPending_ = false;
        return;
    }

    auto& nextNode = sendQueue_.front();
    sock_->async_write_some(
        boost::asio::buffer(nextNode->msg_, curNode->totalLen_),
        std::bind(
            &Session::writeToSocketCallback, this, std::placeholders::_1, std::placeholders::_2));
}

}   // namespace ItemX2


namespace ItemX3 {

constexpr int RECV_SIZE = 1024;
struct MsgNode
{
public:
    MsgNode(const char* msg, int totalLen)
        : totalLen_(totalLen)
        , curLen_(0)
    {
        msg_ = new char[totalLen];
        memset(msg_, 0, totalLen);
    }

    explicit MsgNode(const int totalLen)
        : totalLen_(totalLen)
        , curLen_(0)
    {
        msg_ = new char[totalLen];
        memset(msg_, 0, totalLen);
    }

    ~MsgNode()
    {
        delete[] msg_;
        msg_ = nullptr;
    }

public:
    char* msg_;
    std::size_t totalLen_;
    std::size_t curLen_;
};

class Session {
public:
    void writeAllToSocket(const std::string& buf);
    void writeAllToSocketCallback(boost::system::error_code ec, std::size_t bytes_transferred);

private:
    bool sendPending_;
    std::queue<std::shared_ptr<MsgNode>> sendQueue_;
    std::shared_ptr<boost::asio::ip::tcp::socket> sock_;
};

void Session::writeAllToSocket(const std::string& buf)
{
    sendQueue_.emplace(new MsgNode(buf.data(), buf.size()));
    // 若当前正在发送数据，则直接返回，等待当前发送完成后的回调继续发送
    if (sendPending_) {
        return;
    }

    sock_->async_send(boost::asio::buffer(buf.data(), buf.size()),
                      std::bind(&Session::writeAllToSocketCallback,
                                this,
                                std::placeholders::_1,
                                std::placeholders::_2));
    sendPending_ = true;
}

void Session::writeAllToSocketCallback(boost::system::error_code ec, std::size_t bytes_transferred)
{
    if (ec.value() != 0) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "write error: " << ec.message();
        return;
    }

    sendQueue_.pop();
    if (sendQueue_.empty()) {
        sendPending_ = false;
        return;
    }

    auto& nexNode = sendQueue_.front();
    sock_->async_send(boost::asio::buffer(nexNode->msg_, nexNode->totalLen_),
                      std::bind(&Session::writeAllToSocketCallback,
                                this,
                                std::placeholders::_1,
                                std::placeholders::_2));
}


}   // namespace ItemX3



namespace ItemX4 {

constexpr int RECV_SIZE = 1024;
struct MsgNode
{
public:
    MsgNode(const char* msg, int totalLen)
        : totalLen_(totalLen)
        , curLen_(0)
    {
        msg_ = new char[totalLen];
        memset(msg_, 0, totalLen);
    }

    explicit MsgNode(const int totalLen)
        : totalLen_(totalLen)
        , curLen_(0)
    {
        msg_ = new char[totalLen];
        memset(msg_, 0, totalLen);
    }

    ~MsgNode()
    {
        delete[] msg_;
        msg_ = nullptr;
    }

public:
    char* msg_;
    std::size_t totalLen_;
    std::size_t curLen_;
};

class Session {
public:
    void readFromSocket();
    void readFromSocketCallback(boost::system::error_code ec, std::size_t bytes_transferred);

private:
    bool recvPending_;
    std::shared_ptr<MsgNode> recvNode_;
    std::shared_ptr<boost::asio::ip::tcp::socket> sock_;
};

void Session::readFromSocket()
{
    if (recvPending_) {
        return;
    }

    recvNode_ = std::make_shared<MsgNode>(RECV_SIZE);
    sock_->async_read_some(
        boost::asio::buffer(recvNode_->msg_, recvNode_->totalLen_),
        std::bind(
            &Session::readFromSocketCallback, this, std::placeholders::_1, std::placeholders::_2));
    recvPending_ = true;
}

void Session::readFromSocketCallback(boost::system::error_code ec, std::size_t bytes_transferred)
{
    if (ec.value() != 0) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "read error: " << ec.message();
        return;
    }

    recvNode_->curLen_ += bytes_transferred;
    // 若当前节点未接收完成，则继续后续的数据发送
    if (recvNode_->curLen_ < recvNode_->totalLen_) {
        sock_->async_read_some(boost::asio::buffer(recvNode_->msg_ + recvNode_->curLen_,
                                                   recvNode_->totalLen_ - recvNode_->curLen_),
                               std::bind(&Session::readFromSocketCallback,
                                         this,
                                         std::placeholders::_1,
                                         std::placeholders::_2));
        return;
    }

    recvPending_ = false;
    recvNode_.reset();
}

}   // namespace ItemX4



namespace ItemX5 {

constexpr int RECV_SIZE = 1024;
struct MsgNode
{
public:
    MsgNode(const char* msg, int totalLen)
        : totalLen_(totalLen)
        , curLen_(0)
    {
        msg_ = new char[totalLen];
        memset(msg_, 0, totalLen);
    }

    explicit MsgNode(const int totalLen)
        : totalLen_(totalLen)
        , curLen_(0)
    {
        msg_ = new char[totalLen];
        memset(msg_, 0, totalLen);
    }

    ~MsgNode()
    {
        delete[] msg_;
        msg_ = nullptr;
    }

public:
    char* msg_;
    std::size_t totalLen_;
    std::size_t curLen_;
};

class Session {
public:
    void readAllFromSocket();
    void readAllFromSocketCallback(boost::system::error_code ec, std::size_t bytes_transferred);

private:
    bool recvPending_;
    std::shared_ptr<MsgNode> recvNode_;
    std::shared_ptr<boost::asio::ip::tcp::socket> sock_;
};

void Session::readAllFromSocket()
{
    if (recvPending_) {
        return;
    }

    recvNode_ = std::make_shared<MsgNode>(RECV_SIZE);
    sock_->async_receive(boost::asio::buffer(recvNode_->msg_, recvNode_->totalLen_),
                         std::bind(&Session::readAllFromSocketCallback,
                                   this,
                                   std::placeholders::_1,
                                   std::placeholders::_2));
    recvPending_ = true;
}

void Session::readAllFromSocketCallback(boost::system::error_code ec, std::size_t bytes_transferred)
{
    if (ec.value() != 0) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "read error: " << ec.message();
        return;
    }

    // 若当前节点投递到逻辑线程中，则继续后续的数据接收
    recvPending_ = false;
    recvNode_.reset();
}

}   // namespace ItemX5


int main(int argc, char* argv[]) {}