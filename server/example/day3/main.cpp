#include "sylar/log.h"

#include <boost/asio.hpp>
#include <optional>

namespace ItemX1 {

bool writeSome(boost::asio::ip::tcp::socket& sock)
{
    std::string str = "hello world!";
    std::size_t totalBytesWritten = 0;
    boost::system::error_code ec;
    while (totalBytesWritten < str.length()) {
        totalBytesWritten += sock.write_some(
            boost::asio::buffer(str.data() + totalBytesWritten, str.length() - totalBytesWritten),
            ec);
        if (ec) {
            if (ec == boost::asio::error::connection_reset ||
                ec == boost::asio::error::broken_pipe) {
                SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Connection closed by peer";
            }
            else {
                SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Write error: " << ec.message();
            }
            return false;
        }
    }

    return true;
}

bool sendAll(boost::asio::ip::tcp::socket& sock)
{
    std::string str = "hello world!";
    boost::system::error_code ec;
    sock.send(boost::asio::buffer(str.data(), str.length()), 0, ec);
    if (ec) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Send error: " << ec.message();
        return false;
    }

    return true;
}

bool writeAll(boost::asio::ip::tcp::socket& sock)
{
    std::string str = "hello world!";
    boost::system::error_code ec;
    boost::asio::write(sock, boost::asio::buffer(str.data(), str.length()), ec);
    if (ec) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Send error: " << ec.message();
        return false;
    }

    return true;
}

}   // namespace ItemX1


namespace ItemX2 {

std::optional<std::string> readSome(boost::asio::ip::tcp::socket& sock, std::size_t length)
{
    std::optional<std::string> str = std::nullopt;
    str->resize(length);
    std::size_t totalBytesRead = 0;
    while (totalBytesRead < length) {
        std::size_t bytesRead =
            sock.read_some(boost::asio::buffer(str->data(), length - totalBytesRead));
        if (bytesRead <= 0) {
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "Connection closed by peer";
            return str;
        }
    }

    return str;
}

bool receiveAll(boost::asio::ip::tcp::socket& sock)
{
    char buf[128] = {0};
    boost::system::error_code ec;
    sock.receive(boost::asio::buffer(buf, 128), 0, ec);
    if (ec) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Receive error: " << ec.message();
        return false;
    }

    return true;
}

bool writeAll(boost::asio::ip::tcp::socket& sock)
{
    char buf[128] = {0};
    boost::system::error_code ec;
    boost::asio::read(sock, boost::asio::buffer(buf, 128), ec);
    if (ec) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Receive error: " << ec.message();
        return false;
    }

    return true;
}

bool readUntil(boost::asio::ip::tcp::socket& sock)
{
    boost::asio::streambuf buf;
    boost::system::error_code ec;
    boost::asio::read_until(sock, buf, '\n', ec);
    if (ec) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "Read error: " << ec.message();
        return false;
    }

    return true;
}

}   // namespace ItemX2

int main(int argc, char* argv[])
{
    return 0;
}