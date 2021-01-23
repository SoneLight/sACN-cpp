#pragma once
#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>
#include <ctime>

namespace sACNcpp 
{
    /**
     * @brief Log level to use
     * 
     */
    enum LogLevel
    {
        Debug = 0,
        Info,
        Warning,
        Critical
    };

    /**
     * @brief An interface for all loggers
     * 
     */
    class LogInterface
    {
        public:
            virtual void Log(LogLevel loglevel, std::string text) = 0;
    };

    /**
     * @brief A default (ostream) logger implementing the LogInterface
     * 
     */
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

    /**
     * @brief The static logger used in this package.
     * 
     * You can set you own logger implementing the LogInterface
     * using the setLogger method.
     */
    class Logger
    {
        public:

            /**
             * @brief Log the log entry to the currently set logging handler.
             * 
             * @param loglevel loglevel of the entry
             * @param text text of the entry
             */
            static void Log(LogLevel loglevel, std::string text)
            {
                if(getLogHandlerRef() == nullptr)
                    return;

                getLogHandlerRef()->Log(loglevel, text);
            }

            /**
             * @brief Set the Logger to use
             * 
             * @param logger Logger to use, implementing LogInterface
             */
            static void setLogger(LogInterface* logger)
            {
                getLogHandlerRef() = logger;                
            }

        private:

            /**
             * @brief Used to statically store the Logging handler
             * 
             * @return LogInterface*& 
             */
            static LogInterface*& getLogHandlerRef()
            {
                static DefaultLogger defaultLogger;
                static LogInterface* currentLogHandler = &defaultLogger;
                return currentLogHandler;
            }
    };
}