#pragma once

namespace core
{
    template <typename T>
    class Singleton
    {
    protected:
        Singleton() = default;
        ~Singleton() = default;

    public:
        Singleton(const Singleton&) = delete;
        Singleton& operator=(const Singleton&) = delete;

        static T& Instance()
        {
            static T sInstance;
            return sInstance;
        }
    };
}

