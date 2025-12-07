#pragma once
#include <functional>
#include <iostream>

namespace utilities
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
