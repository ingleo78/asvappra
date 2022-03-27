#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_WAITEVENT_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_WAITEVENT_H_

#include <chrono>
#include <condition_variable>
#include <mutex>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            class WaitEvent {
            public:
                WaitEvent();
                void wakeUp();
                bool wait(const std::chrono::milliseconds& timeout);
                void reset();
            private:
                std::condition_variable m_condition;
                std::mutex m_mutex;
                bool m_wakeUpTriggered;
            };
        }
    }
}
#endif