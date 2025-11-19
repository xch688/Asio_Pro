#include "session.h"
#include "sylar/log.h"

Server::Server(boost::asio::io_context& ctx, uint16_t port)
    : port_(port)
    , ctx_(ctx)
    , acceptor_(ctx)
{}

void Server::init()
{
    boost::system::error_code ec;
    if (acceptor_.open(boost::asio::ip::tcp::v4(), ec)) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Open acceptor error: " << ec.message();
        return;
    }
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));

    boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address_v4().any(), port_);
    if (acceptor_.bind(ep, ec)) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Bind acceptor error: " << ec.message();
        return;
    }

    if (acceptor_.listen(boost::asio::socket_base::max_connections, ec)) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Listen acceptor error: " << ec.message();
        return;
    }
}


void Server::start_accept()
{
    Session* new_session = new Session(ctx_);
    // 异步接受新的连接
    acceptor_.async_accept(
        new_session->socket(),
        std::bind(&Server::handle_accept, this, new_session, std::placeholders::_1));
}


void Server::handle_accept(Session* new_session, const boost::system::error_code& ec)
{
    if (ec.value() != 0) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Accept error: " << ec.message();
        delete new_session;
        return;
    }

    auto& sock = new_session->socket();
    SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())
        << "accept a new client; ip: " << sock.remote_endpoint().address().to_string()
        << "; port: " << sock.remote_endpoint().port();
    new_session->start();

    start_accept();   // 继续接受下一个连接
}


void Session::start()
{
    memset(data_, 0, sizeof(data_));
    sock_.async_read_some(
        boost::asio::buffer(data_, max_length),
        std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2));
}


void Session::handle_read(const boost::system::error_code& ec, std::size_t bytes_transferred)
{
    if (ec.value() != 0) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Read error: " << ec.message();
        delete this;
        return;
    }

    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "recv data: " << std::string(data_, bytes_transferred);
    boost::asio::async_write(sock_,
                             boost::asio::buffer(data_, bytes_transferred),
                             std::bind(&Session::handle_write, this, std::placeholders::_1));
}


void Session::handle_write(const boost::system::error_code& ec)
{
    if (ec.value() != 0) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Write error: " << ec.message();
        delete this;
        return;
    }

    memset(data_, 0, sizeof(data_));
    sock_.async_read_some(
        boost::asio::buffer(data_, max_length),
        std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2));
}