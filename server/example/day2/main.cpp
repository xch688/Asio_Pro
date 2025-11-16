#include "sylar/log.h"

#include "boost/asio.hpp"

namespace ItemX1 {

void test()
{
    std::string str = "Hello, Boost.Asio!";
    boost::asio::const_buffer asio_buf(str.data(), str.size());
    std::vector<boost::asio::const_buffer> asio_vec;
    asio_vec.emplace_back(asio_buf);

    // 简化版发送，为演示调用API接口
    boost::asio::io_context ctx;
    boost::asio::ip::tcp::socket sock(ctx);
    sock.send(asio_vec);
}

}   // namespace ItemX1


namespace ItemX2 {

void test()
{
    // 简化版发送，为演示调用API接口
    boost::asio::io_context ctx;
    boost::asio::ip::tcp::socket sock(ctx);
    sock.send(boost::asio::buffer("Hello, Boost.Asio!"));
}

}   // namespace ItemX2


namespace ItemX3 {

void test()
{
    const auto data = "Hello, Boost.Asio!";
    std::size_t size = strlen(data);
    // 简化版发送，为演示调用API接口
    boost::asio::io_context ctx;
    boost::asio::ip::tcp::socket sock(ctx);
    sock.send(boost::asio::buffer(data, size));
}

}   // namespace ItemX3


namespace ItemX4 {

void test()
{
    boost::asio::streambuf buf;
    std::ostream os(&buf);   // 流绑定
    os << "Hello, Boost.Asio!";

    std::string msg;
    std::iostream ios(&buf);   // 流绑定
    ios >> msg;
    SYLAR_LOG_DEBUG(SYLAR_LOG_ROOT()) << "msg = " << msg;

    os << "Success";
    ios >> msg;
    SYLAR_LOG_DEBUG(SYLAR_LOG_ROOT()) << "msg = " << msg;
    ios >> msg;
    if (!ios.fail()) {
        SYLAR_LOG_DEBUG(SYLAR_LOG_ROOT()) << "msg = " << msg;
    }
}

}   // namespace ItemX4

int main(int argc, char* argv[])
{
    ItemX4::test();

    return 0;
}