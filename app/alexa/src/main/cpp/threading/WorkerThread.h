#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_THREADING_WORKERTHREAD_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_THREADING_WORKERTHREAD_H_

#include <atomic>
#include <condition_variable>
#include <functional>
#include <list>
#include <string>
#include <thread>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace threading {
                class WorkerThread {
                public:
                    WorkerThread();
                    ~WorkerThread();
                    void run(std::function<bool()> workFunc);
                    void cancel();
                    std::string getMoniker() const;
                private:
                    void runInternal();
                    const std::string m_moniker;
                    std::atomic<bool> m_stop;
                    std::atomic<bool> m_cancel;
                    std::thread m_thread;
                    std::function<bool()> m_workerFunc;
                    std::mutex m_mutex;
                    std::condition_variable m_workReady;
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_THREADING_WORKERTHREAD_H_
