#pragma once

#include <memory>

namespace sylar {

namespace {

template<typename T, typename X, int N> T& GetInstanceX()
{
    static T v;
    return v;
}

template<typename T, typename X, int N> std::shared_ptr<T> GetInstancePtr()
{
    static std::shared_ptr<T> v = std::make_shared<T>();
    return v;
}

}   // namespace


template<typename T, typename X = void, int N = 0> class Singleton {
public:
    static T* GetInstance()
    {
        // static T v;
        // return &v;
        return &GetInstanceX<T, X, N>();
    }
};

template<typename T, typename X = void, int N = 0> class SingletonPtr {
public:
    static std::shared_ptr<T> GetInstance()
    {
        // static std::shared_ptr<T> v = std::make_shared<T>();
        // return v;
        return GetInstancePtr<T, X, N>();
    }
};

}   // namespace sylar
