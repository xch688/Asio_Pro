#include "sylar/util.h"
#include "sylar/log.h"

#include <cstdlib>
#include <cxxabi.h>
#include <filesystem>
#include <fstream>
#include <libunwind.h>
#include <sstream>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

namespace sylar {

pid_t GetThreadId()
{
    return syscall(SYS_gettid);
}

uint32_t GetFiberId()
{
    // todo: 获取协程ID
    return 11;
}

std::string BacktraceToString()
{
    unw_cursor_t cursor;
    unw_context_t context;
    unw_getcontext(&context);
    unw_init_local(&cursor, &context);

    std::stringstream ss;
    while (unw_step(&cursor) > 0) {
        unw_word_t offset, pc;
        unw_get_reg(&cursor, UNW_REG_IP, &pc);
        if (pc == 0) {
            break;
        }
        ss << "0x" << std::hex << pc << ":";

        char sym[256];
        if (unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) != 0) {
            return "fail to obtain backtrace";
        }

        char* nameptr = sym;
        int status;
        char* demangled = abi::__cxa_demangle(sym, nullptr, nullptr, &status);
        if (status == 0) {
            nameptr = demangled;
        }
        ss << "(" << nameptr << "+0x" << offset << ")\n";
        std::free(demangled);
    }

    return ss.str();
}

}   // namespace sylar



namespace sylar {

bool FSUtil::OpenForWrite(std::ofstream& ofs, const std::string& filename,
                          std::ios_base::openmode mode)
{
    ofs.open(filename.c_str(), mode);
    if (!ofs.is_open()) {
        std::string path = std::filesystem::path(filename).parent_path().string();
        if (!path.empty() && !std::filesystem::exists(path)) {
            std::filesystem::create_directories(path);
        }

        ofs.open(filename.c_str(), mode);
    }

    return ofs.is_open();
}


void FSUtil::ListAllFile(std::vector<std::string>& files, const std::string& path,
                         const std::string& subfix)
{
    if (access(path.c_str(), 0) != 0) {
        return;
    }

    for (auto& entry : std::filesystem::recursive_directory_iterator(path)) {
        if (entry.is_regular_file() && entry.path().extension() == ".yml") {
            files.push_back(entry.path());
        }
    }
}

}   // namespace sylar