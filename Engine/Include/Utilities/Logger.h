#pragma once
#include <iostream>
#include <string>

#ifndef USING_LOGGER
#define LOG(a)      do { std::cout << "[LOG] "   << a << std::endl; } while(0)
#define LOGERROR(a) do { std::cout << "[ERROR] " << a << std::endl; } while(0)
#endif

namespace engine::utilities
{
    class ScopeLogger
    {
    public:
        static bool Enable;
        ScopeLogger(const std::string& functionName)
            : m_functionName(functionName)
        {
            if(Enable) std::cout << "[ENTER] " << m_functionName << std::endl;
        }

        ~ScopeLogger()
        {
            if(Enable) std::cout << "[EXIT]  " << m_functionName << std::endl;
        }

    private:
        std::string m_functionName;
    };

    inline bool ScopeLogger::Enable = false;


}

// Helper macro: creates a unique logger object per function
//#define LOG_FUNCTION()if(engine::utilities::ScopeLogger::Enable) engine::utilities::ScopeLogger __logger(__func__) // standard C++
#define LOG_FUNCTION()engine::utilities::ScopeLogger __logger(__FUNCSIG__) // MSVC

