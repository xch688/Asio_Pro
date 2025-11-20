#include "example/day7/Server.h"
#include "sylar/log.h"

int main(int argc, char* argv[])
{
    try {
        boost::asio::io_context ctx;
        Server server(ctx, 8888);
        if (!server.init()) {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "failed to initialize";
            return -1;
        }
        server.startAccept();
        ctx.run();
    }
    catch (std::exception& ex) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "exception: " << ex.what();
    }
}