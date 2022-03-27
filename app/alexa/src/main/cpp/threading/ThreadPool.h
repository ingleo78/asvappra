#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_THREADING_THREADPOOL_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_THREADING_THREADPOOL_H_

#include <condition_variable>
#include <list>
#ifdef THREAD_AFFINITY
#include <map>
#endif
#include <string>
#include "WorkerThread.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace threading {
                static const size_t DEFAULT_MAX_THREAD_POOL_THREADS = 20;
                class ThreadPool {
                public:
                    ThreadPool(size_t maxThreads = DEFAULT_MAX_THREAD_POOL_THREADS);
                    ~ThreadPool();
                    std::unique_ptr<WorkerThread> obtainWorker(std::string optionalMoniker = "");
                    void releaseWorker(std::unique_ptr<WorkerThread> workerThread);
                    void setMaxThreads(size_t maxThreads);
                    uint32_t getMaxThreads();
                    void getStats(
                        uint64_t& threadsCreated,
                        uint64_t& threadsObtained,
                        uint64_t& threadsReleasedToPool,
                        uint64_t& threadsReleasedFromPool);
                    static std::shared_ptr<ThreadPool> getDefaultThreadPool();
                private:
                #ifdef THREAD_AFFINITY
                    std::map<std::string, std::list<std::unique_ptr<WorkerThread>>::iterator> m_workerMap;
                #endif
                    std::list<std::unique_ptr<WorkerThread>> m_workerQueue;
                    size_t m_maxPoolThreads;
                    uint64_t m_created;
                    uint64_t m_obtained;
                    uint64_t m_releasedToPool;
                    uint64_t m_releasedFromPool;
                    std::mutex m_queueMutex;
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_THREADING_THREADPOOL_H_
