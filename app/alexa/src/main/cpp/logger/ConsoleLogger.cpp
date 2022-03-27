#include <cstdio>
#include <ctime>
#include <iostream>
#include <mutex>
#include "util/CoutMutex.h"
#include "ConsoleLogger.h"
#include "LoggerUtils.h"
#include "ThreadMoniker.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace logger {
                static const std::string CONFIG_KEY_DEFAULT_LOGGER = "consoleLogger";
                std::shared_ptr<Logger> ConsoleLogger::instance() {
                    static std::shared_ptr<Logger> singleConsoleLogger = std::shared_ptr<ConsoleLogger>(new ConsoleLogger);
                    return singleConsoleLogger;
                }
                void ConsoleLogger::emit(Level level, std::chrono::system_clock::time_point time, const char* threadMoniker, const char* text) {
                    if (m_coutMutex) {
                        std::lock_guard<std::mutex> lock(*m_coutMutex);
                        std::cout << m_logFormatter.format(level, time, threadMoniker, text) << std::endl;
                    }
                }
                ConsoleLogger::ConsoleLogger() : Logger(Level::UNKNOWN), m_coutMutex{getCoutMutex()} {
                    #ifdef DEBUG
                        setLevel(Level::DEBUG9);
                    #else
                        setLevel(Level::INFO);
                    #endif
                    init(configuration::ConfigurationNode::getRoot()[CONFIG_KEY_DEFAULT_LOGGER]);
                }
                std::shared_ptr<Logger> getConsoleLogger() { return ConsoleLogger::instance(); }
            }
        }
    }
}
