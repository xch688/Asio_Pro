#include "boost/asio.hpp"
#include "sylar/log.h"

// 客户端与服务端的通信端点设置
namespace ItemX1 {

int ClientEndPoint(std::string rawIp, uint16_t port)
{
    boost::system::error_code ec;
    boost::asio::ip::address serAddr = boost::asio::ip::make_address(rawIp, ec);
    if (ec.value() != 0) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "Error: " << ec.message();
        return -1;
    }

    boost::asio::ip::tcp::endpoint ep(serAddr, port);

    return 0;
}

int CreateServerEndPoint(uint16_t port)
{
    boost::asio::ip::address addr = boost::asio::ip::address_v4().any();
    boost::asio::ip::tcp::endpoint ep(addr, port);

    return 0;
}

}   // namespace ItemX1
// namespace ItemX1


namespace ItemX2 {

void CreateClientSocket()
{
    boost::asio::io_context ctx;
    boost::asio::ip::tcp::socket sock(ctx);

    boost::system::error_code ec;
    sock.open(boost::asio::ip::tcp::v4(), ec);
    if (ec.value() != 0) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "Error: " << ec.message();
    }
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "Creating client socket";
}

void createServerSocket()
{
    boost::asio::io_context ctx;
    boost::asio::ip::tcp::acceptor acceptor(ctx);

    boost::system::error_code ec;
    acceptor.open(boost::asio::ip::tcp::v4(), ec);
    if (ec.value() != 0) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "Error: " << ec.message();
    }
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "Creating server socket";
}

}   // namespace ItemX2


namespace ItemX3 {

void serverbindEndPoint(uint16_t port)
{
    boost::asio::io_context cxt;
    boost::asio::ip::tcp::socket acceptor(cxt);
    boost::system::error_code ec;
    if (acceptor.open(boost::asio::ip::tcp::v4(), ec)) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "Error: " << ec.message();
        return;
    };

    boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address_v4().any(), port);
    if (acceptor.bind(ep, ec)) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "Error: " << ec.message();
        return;
    }
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "Bind socket success";
}

void clientBindEndPoint(const std::string& rawIp, uint16_t port)
{
    boost::asio::io_context ctx;
    boost::asio::ip::tcp::socket sock(ctx);
    boost::system::error_code ec;
    if (sock.open(boost::asio::ip::tcp::v4(), ec)) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "Error: " << ec.message();
        return;
    }

    boost::asio::ip::address addr = boost::asio::ip::make_address(rawIp, ec);
    if (ec.value() != 0) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "Error: " << ec.message();
        return;
    }
    boost::asio::ip::tcp::endpoint ep(addr, port);
    if (sock.connect(ep, ec)) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "Error: " << ec.message();
        return;
    }
}

}   // namespace ItemX3


namespace ItemX4 {

void start(uint16_t port)
{
    boost::asio::io_context ctx;
    boost::asio::ip::tcp::acceptor acceptor(ctx);
    boost::system::error_code ec;
    if (acceptor.open(boost::asio::ip::tcp::v4(), ec)) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "Error: " << ec.message();
        return;
    }
    acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));

    boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address_v4().any(), port);
    if (acceptor.bind(ep, ec)) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "Error: " << ec.message();
        return;
    }

    if (acceptor.listen(boost::asio::socket_base::max_connections, ec)) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "Error: " << ec.message();
        return;
    }

    boost::asio::ip::tcp::endpoint peer;
    boost::asio::ip::tcp::socket sock = acceptor.accept(peer, ec);
    if (ec.value() == 0) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT())
            << "peer ip: " << peer.address().to_string() << "; port: " << peer.port();
    }
}

}   // namespace ItemX4


int main(int argc, char* argv[])
{
    // ItemX1::ClientEndPoint("172.17.0.2", 8080);
    // ItemX1::CreateServerEndPoint(8080);
    // ItemX2::CreateClientSocket();
    // ItemX2::createServerSocket();
    // ItemX3::serverbindEndPoint(8080);
    // ItemX3::clientBindEndPoint("172.17.0.2", 8080);
    ItemX4::start(8888);

    return 0;
}
