#include "example/day6/session.h"
#include "sylar/log.h"

int main(int argc, char* argv[])
{
    try {
        boost::asio::io_context ctx;
        Server server(ctx, 8888);
        server.init();
        server.start_accept();
        ctx.run();
    }
    catch (std::exception& ex) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Exception: " << ex.what();
    }

    return 0;
}