#pragma once

#include "sylar/mutex.h"
#include "sylar/noncopyable.h"

#include <memory>

namespace sylar {

class Thread : public NonCopyable {
public:
    using ptr = std::shared_ptr<Thread>;

public:
    Thread(std::function<void()> cb, std::string name);
    ~Thread();
    void join();

    [[nodiscard]] pid_t Id() const { return id_; }
    [[nodiscard]] const std::string& getName() const { return name_; }

    static const std::string& GetName();
    static void SetName(const std::string& name);
    static Thread* GetThis();

private:
    static void* run(void* arg);

private:
    pid_t id_ = -1;
    pthread_t thread_ = 0;
    std::function<void()> cb_;
    std::string name_;
    Semaphore sem_;
};

}   // namespace sylar