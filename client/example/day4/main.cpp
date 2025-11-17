#include "boost/asio.hpp"
#include "sylar/log.h"

int main(int argc, char* argv[])
{
    boost::asio::io_context ctx;
    boost::asio::ip::tcp::socket sock(ctx);
    boost::system::error_code ec;
    if (sock.open(boost::asio::ip::tcp::v4(), ec)) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "open socket error: " << ec.message();
        return -1;
    }

    boost::asio::ip::tcp::endpoint ep(boost::asio::ip::tcp::v4(), 8888);
    if (sock.connect(ep, ec)) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "connect socket error: " << ec.message();
        return -1;
    }

    std::string msg = "Hello World!";
    sock.send(boost::asio::buffer(msg.data(), msg.size()), 0, ec);
    if (ec) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "send error: " << ec.message();
        return -1;
    }

    char buf[128] = {0};
    std::size_t bytesRead = sock.read_some(boost::asio::buffer(buf, 128), ec);
    if (ec) {
        if (ec == boost::asio::error::eof) {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "connection closed: " << ec.message();
            sock.close();
            return -1;
        }
        else {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "read error: " << ec.message();
            return -1;
        }
    }
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "recv: " << std::string(buf, bytesRead);

    return 0;
}
