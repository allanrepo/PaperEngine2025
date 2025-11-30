#pragma once
#include <iostream>
#include <string>

#ifndef USING_LOGGER
#define LOG(a)      do { std::cout << "[LOG] "   << a << std::endl; } while(0)
#define LOGERROR(a) do { std::cout << "[ERROR] " << a << std::endl; } while(0)
#endif

namespace utilities
{
    class Logger
    {
    public:
        static bool Enable;
        Logger(const std::string& functionName)
            : m_functionName(functionName)
        {
            std::cout << "[ENTER] " << m_functionName << std::endl;
        }

        ~Logger()
        {
            std::cout << "[EXIT]  " << m_functionName << std::endl;
        }

    private:
        std::string m_functionName;
    };

    inline bool Logger::Enable = false;


}

// Helper macro: creates a unique logger object per function
#define LOG_FUNCTION()if(utilities::Logger::Enable) utilities::Logger __logger(__FUNCTION__)

