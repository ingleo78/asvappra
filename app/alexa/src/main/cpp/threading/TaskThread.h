#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_THREADING_TASKTHREAD_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_THREADING_TASKTHREAD_H_

#include <atomic>
#include <chrono>
#include <functional>
#include <string>
#include <thread>
#include "ThreadPool.h"
#include "WorkerThread.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace threading {
                class TaskThread {
                public:
                    TaskThread();
                    ~TaskThread();
                    bool start(std::function<bool()> jobRunner);
                private:
                    void run(std::function<bool()> jobRunner);
                    std::chrono::steady_clock::time_point m_startTime;
                    std::mutex m_mutex;
                    bool m_shuttingDown;
                    std::atomic_bool m_stop;
                    std::atomic_bool m_alreadyStarting;
                    std::string m_moniker;
                    std::shared_ptr<ThreadPool> m_threadPool;
                    std::unique_ptr<WorkerThread> m_workerThread;
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_THREADING_TASKTHREAD_H_
