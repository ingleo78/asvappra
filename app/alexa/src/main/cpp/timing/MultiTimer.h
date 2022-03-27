#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_TIMING_MULTITIMER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_TIMING_MULTITIMER_H_

#include <cstdint>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <map>
#include <mutex>
#include <stdint.h>
#include <threading/Executor.h>
#include <threading/TaskThread.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace timing {
                class MultiTimer {
                public:
                    using Token = uint64_t;
                    MultiTimer();
                    ~MultiTimer();
                    Token submitTask(const std::chrono::milliseconds& delay, std::function<void()> task);
                    void cancelTask(Token token);
                private:
                    bool executeTimer();
                    bool hasNextLocked(std::unique_lock<std::mutex>& lock);
                    using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;
                    std::condition_variable m_waitCondition;
                    std::mutex m_waitMutex;
                    threading::TaskThread m_timerThread;
                    std::multimap<TimePoint, Token> m_timers;
                    std::map<Token, std::pair<TimePoint, std::function<void()>>> m_tasks;
                    bool m_isRunning;
                    bool m_isBeingDestroyed;
                    Token m_nextToken;
                };
            }
        }
    }
}
#endif