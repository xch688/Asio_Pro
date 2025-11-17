#include "boost/asio.hpp"
#include "example/day4/Server.h"
#include "sylar/log.h"

int main(int argc, char* argv[])
{
    Server server(8888);
    if (!server.init()) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "Server init failed";
    }

    server.start();
    return 0;
}