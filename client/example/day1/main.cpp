#include "boost/asio.hpp"
#include "sylar/log.h"

// 客户端与服务端的通信端点设置
namespace ItemX1 {
void start(const std::string& raw_ip, uint16_t port)
{
    boost::asio::io_context ctx;
    boost::asio::ip::tcp::socket sock(ctx);
    boost::system::error_code ec;
    if (sock.open(boost::asio::ip::tcp::v4(), ec)) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Error: " << ec.message();
        return;
    }

    boost::asio::ip::address address = boost::asio::ip::make_address_v4(raw_ip);
    boost::asio::ip::tcp::endpoint ep(address, port);
    if (sock.connect(ep, ec)) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Error: " << ec.message();
        return;
    }
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "Connect to " << raw_ip << ":" << port << " success";
}
}   // namespace ItemX1
// namespace ItemX1


int main(int argc, char* argv[])
{
    ItemX1::start("172.17.0.2", 8888);
    return 0;
}
