#pragma once
#include <functional>
#include <iostream>
#include <random>
#include <type_traits>
#include <algorithm>

namespace engine::utilities
{
	// A utility class that allows you to register a callback that will be called when the object goes out of scope.
	// This is useful for cleanup tasks that should happen automatically when the object is destroyed.
	// has option to dismiss the callback before it is called
	class OnOutOfScope
	{
    private:
        std::function<void()> callback;
        bool active;

    public:
        explicit OnOutOfScope(std::function<void()> cb)
            : callback(std::move(cb)), active(true)
        {
        }

        ~OnOutOfScope()
        {
            if (active && callback) callback();
        }

        void dismiss() 
        { 
            active = false; 
        }
	};

    //// TODO: test this on different types like int
    //template<typename T = float>
    //float Random(T min = 0, T max = 1)
    //{
    //    static std::random_device rd;  // seeds once
    //    static std::mt19937 gen(rd()); // Mersenne Twister engine
    //    std::uniform_real_distribution<T> dist(min, max);
    //    return dist(gen);
    //}	

    template<typename T = float>
    T Random(T min = T{ 0 }, T max = T{ 1 })
    {
        if (min > max)
        {
            std::swap(min, max);
        }

        static std::random_device rd;
        static std::mt19937 gen(rd());

        if constexpr (std::is_integral_v<T>)
        {
            std::uniform_int_distribution<T> dist(min, max);
            return dist(gen);
        }
        else if constexpr (std::is_floating_point_v<T>)
        {
            std::uniform_real_distribution<T> dist(min, max);
            return dist(gen);
        }
    }

    //struct Text
    //{
    //    static std::wstring ToWide(const std::string& str)
    //    {
    //        int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(),
    //            (int)str.size(), nullptr, 0);

    //        std::wstring wstr(size_needed, 0);

    //        MultiByteToWideChar(CP_UTF8, 0, str.c_str(),
    //            (int)str.size(), &wstr[0], size_needed);

    //        return wstr;
    //    }
    //};
}
