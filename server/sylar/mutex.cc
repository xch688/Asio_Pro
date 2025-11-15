#include "sylar/mutex.h"

#include <stdexcept>

namespace sylar {

Semaphore::Semaphore(int32_t count)
{
    if (0 != sem_init(&sem_, 0, count)) {
        throw std::logic_error("sem_init error.");
    }
}

Semaphore::~Semaphore()
{
    sem_destroy(&sem_);
}

void Semaphore::wait()
{
    if (sem_wait(&sem_) != 0) {
        throw std::logic_error("sem_wait error.");
    }
}

void Semaphore::notify()
{
    if (sem_post(&sem_) != 0) {
        throw std::logic_error("sem_post error.");
    }
}

}   // namespace sylar


namespace sylar {

Mutex::Mutex()
{
    if (pthread_mutex_init(&mutex_, nullptr) != 0) {
        throw std::logic_error("pthread_mutex_init error");
    }
}

Mutex::~Mutex()
{
    pthread_mutex_destroy(&mutex_);
}

void Mutex::lock()
{
    if (pthread_mutex_lock(&mutex_) != 0) {
        throw std::logic_error("pthread_mutex_lock error.");
    }
}

void Mutex::unlock()
{
    if (pthread_mutex_unlock(&mutex_) != 0) {
        throw std::logic_error("pthread_mutex_unlock error.");
    }
}

}   // namespace sylar




namespace sylar {

RWMutex::RWMutex()
{
    if (pthread_rwlock_init(&mutex_, nullptr) != 0) {
        throw std::logic_error("pthread_rwlock_init error.");
    }
}

RWMutex::~RWMutex()
{
    pthread_rwlock_destroy(&mutex_);
}

void RWMutex::rdlock()
{
    if (pthread_rwlock_rdlock(&mutex_) != 0) {
        throw std::logic_error("pthread_rwlock_rdlock error.");
    }
}

void RWMutex::wrlock()
{
    if (pthread_rwlock_wrlock(&mutex_) != 0) {
        throw std::logic_error("pthread_rwlock_wrlock error.");
    }
}

void RWMutex::unlock()
{
    if (pthread_rwlock_unlock(&mutex_) != 0) {
        throw std::logic_error("pthread_rwlock_unlock error.");
    }
}

}   // namespace sylar



namespace sylar {

Spinlock::Spinlock()
{
    if (pthread_spin_init(&mutex_, 0) != 0) {
        throw std::logic_error("pthread_spin_init error.");
    }
}

Spinlock::~Spinlock()
{
    pthread_spin_destroy(&mutex_);
}

void Spinlock::lock()
{
    if (pthread_spin_lock(&mutex_) != 0) {
        throw std::logic_error("pthread_spin_lock error.");
    }
}

void Spinlock::unlock()
{
    if (pthread_spin_unlock(&mutex_) != 0) {
        throw std::logic_error("pthread_spin_unlock error.");
    }
}

}   // namespace sylar


namespace sylar {

CASLock::CASLock()
{
    mutex_.clear();
}

CASLock::~CASLock() = default;

void CASLock::lock()
{
    while (std::atomic_flag_test_and_set_explicit(&mutex_, std::memory_order_acquire)) {}
}

void CASLock::unlock()
{
    std::atomic_flag_clear_explicit(&mutex_, std::memory_order_release);
}

}   // namespace sylar



namespace sylar {

void RWSpinlock::rdlock()
{
    mutex_.lock_shared();
    isRead_ = true;
}

void RWSpinlock::wrlock()
{
    mutex_.lock();
    isRead_ = false;
}

void RWSpinlock::unlock()
{
    if (isRead_) {
        mutex_.unlock_shared();
    }
    else {
        mutex_.unlock();
    }
}

}   // namespace sylar