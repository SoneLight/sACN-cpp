#pragma once
#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>
#include <ctime>

namespace sACNcpp 
{
    enum LogLevel
    {
        Debug = 0,
        Info,
        Warning,
        Critical
    };

    class LogInterface
    {
        public:
            virtual void Log(LogLevel loglevel, std::string text) = 0;
    };

    class DefaultLogger : public LogInterface
    {
        public:
            void Log(LogLevel loglevel, std::string text) override
            {
                std::string leveltext;
                
                auto t = std::time(nullptr);
                auto tm = *std::localtime(&t);

                switch(loglevel)
                {
                    case LogLevel::Debug:
                        leveltext = "Debug";
                        break;
                    case LogLevel::Info:
                        leveltext = "Info";
                        break;
                    case LogLevel::Warning:
                        leveltext = "Warning";
                        break;
                    case LogLevel::Critical:
                        leveltext = "Critical";
                        break;
                }

                std::cout << "[" << std::put_time(&tm, "%d-%m-%Y %H-%M-%S") << "][" << leveltext << "] " << text << std::endl;

            }

    };

    class Logger
    {
        public:

            static void Log(LogLevel loglevel, std::string text)
            {
                if(getLogHandlerRef() == nullptr)
                    return;

                getLogHandlerRef()->Log(loglevel, text);
            }

            static void setLogger(LogInterface* logger)
            {
                if(getLogHandlerRef != nullptr)
                    delete getLogHandlerRef();
                getLogHandlerRef() = logger;                
            }

            static LogInterface*& getLogHandlerRef()
            {
                static DefaultLogger defaultLogger;
                static LogInterface* currentLogHandler = &defaultLogger;
                return currentLogHandler;
            }
    };
}