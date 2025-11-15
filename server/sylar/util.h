#pragma once

#include <cxxabi.h>
#include <ios>
#include <stdint.h>
#include <string>
#include <vector>

namespace sylar {

pid_t GetThreadId();
uint32_t GetFiberId();

template<typename T> const char* TypeToName()
{
    static const char* s_name = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
    return s_name;
}

std::string BacktraceToString();

class FSUtil {
public:
    static bool OpenForWrite(std::ofstream& ofs, const std::string& filename,
                             std::ios_base::openmode mode);
    static void ListAllFile(std::vector<std::string>& files, const std::string& path,
                            const std::string& subfix);
};

}   // namespace sylar
