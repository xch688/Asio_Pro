#include "sylar/thread.h"
#include "sylar/log.h"

namespace sylar {

static thread_local Thread* t_thread = nullptr;
static thread_local std::string t_thread_name = "UNKNOW";

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

const std::string& Thread::GetName()
{
    return t_thread_name;
}

void Thread::SetName(const std::string& name)
{
    if (name.empty()) {
        return;
    }

    if (t_thread) {
        t_thread->name_ = name;
    }

    t_thread_name = name;
}

Thread* Thread::GetThis()
{
    return t_thread;
}


Thread::Thread(std::function<void()> cb, std::string name)
    : cb_(cb)
    , name_(std::move(name))
{
    if (name_.empty()) {
        name_ = "UNKNOW";
    }

    if (const int rt = pthread_create(&thread_, nullptr, &Thread::run, this)) {
        SYLAR_LOG_ERROR(g_logger) << "pthread_create thread fail, rt=" << rt << " name=" << name_;
        throw std::logic_error("create thread fail");
    }

    sem_.wait();
}

Thread::~Thread()
{
    if (thread_) {
        pthread_detach(thread_);
    }
}

void Thread::join()
{
    if (thread_) {
        if (const int rt = pthread_join(thread_, nullptr)) {
            SYLAR_LOG_ERROR(g_logger) << "pthread_join thread fail, rt=" << rt << " name=" << name_;
            throw std::logic_error("join thread fail");
        }

        // 防止线程再次被Join或者detach
        thread_ = 0;
    }
}




void* Thread::run(void* arg)
{
    Thread* self = static_cast<Thread*>(arg);
    t_thread = self;
    t_thread_name = self->name_;

    self->id_ = sylar::GetThreadId();
    pthread_setname_np(pthread_self(), self->name_.substr(0, 15).c_str());

    std::function<void()> cb;
    cb.swap(self->cb_);
    self->sem_.notify();

    cb();

    return nullptr;
}


}   // namespace sylar