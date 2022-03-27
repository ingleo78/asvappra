#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_TIMING_STOPWATCH_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_TIMING_STOPWATCH_H_

#include <chrono>
#include <mutex>
#include <thread>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace timing {
                class Stopwatch {
                public:
                    Stopwatch();
                    bool start();
                    bool pause();
                    bool resume();
                    void stop();
                    void reset();
                    std::chrono::milliseconds getElapsed();
                private:
                    enum class State {
                        RESET,
                        RUNNING,
                        PAUSED,
                        STOPPED
                    };
                    std::mutex m_mutex;
                    State m_state;
                    std::chrono::steady_clock::time_point m_startTime;
                    std::chrono::steady_clock::time_point m_pauseTime;
                    std::chrono::steady_clock::time_point m_stopTime;
                    std::chrono::milliseconds m_totalTimePaused;
                };
            }
        }
    }
}
#endif