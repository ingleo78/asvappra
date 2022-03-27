#include <memory>
#include <functional>
#include <logger/Logger.h>
#include <logger/ThreadMoniker.h>
#include <memory/Memory.h>
#include <threading/ThreadPool.h>

static const std::string TAG("ThreadPool");
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace threading {
                using namespace std;
                using namespace logger;
                static ThreadPool SINGLETON_THREAD_POOL{};
                ThreadPool::ThreadPool(size_t maxThreads) : m_maxPoolThreads{maxThreads}, m_created{0}, m_obtained{0}, m_releasedToPool{0}, m_releasedFromPool{0} {}
                ThreadPool::~ThreadPool() {
                    std::lock_guard<mutex> lock(m_queueMutex);
                    m_workerQueue.clear();
                }
                unique_ptr<WorkerThread> ThreadPool::obtainWorker(string optionalMoniker) {
                    std::lock_guard<mutex> lock(m_queueMutex);
                    ACSDK_DEBUG9(LX("obtainWorker")
                                     .d("created", m_created)
                                     .d("obtained", m_obtained)
                                     .d("releasedToPool", m_releasedToPool)
                                     .d("releasedFromPool", m_releasedFromPool)
                                     .d("outstanding", m_obtained - (m_releasedToPool + m_releasedFromPool)));
                    m_obtained++;
                    unique_ptr<WorkerThread> ret;
                    if (m_workerQueue.empty()) {
                        m_created++;
                        ret = memory::make_unique<WorkerThread>();
                    } else {
                    #ifdef THREAD_AFFINITY
                        bool containsMoniker = false;
                        if (!optionalMoniker.empty()) {
                            containsMoniker = m_workerMap.count(optionalMoniker) > 0;
                        }
                        auto workerIterator = containsMoniker ? m_workerMap[optionalMoniker] : m_workerQueue.begin();
                    #else
                        auto workerIterator = m_workerQueue.begin();
                    #endif
                        ret = std::move(*workerIterator);
                        m_workerQueue.erase(workerIterator);
                    #ifdef THREAD_AFFINITY
                        std::string moniker = (*workerIterator)->getMoniker();
                        m_workerMap.erase(moniker);
                    #endif
                    }
                    return ret;
                }
                void ThreadPool::releaseWorker(std::unique_ptr<WorkerThread> workerThread) {
                    std::lock_guard<mutex> lock(m_queueMutex);
                    if (m_workerQueue.size() >= m_maxPoolThreads) {
                        m_workerQueue.pop_front();
                        m_releasedFromPool++;
                    } else m_releasedToPool++;
                    m_workerQueue.push_back(std::move(workerThread));
                #ifdef THREAD_AFFINITY
                    std::string moniker = workerThread->getMoniker();
                    m_workerMap[moniker] = --m_workerQueue.end();
                #endif
                }
                void ThreadPool::setMaxThreads(size_t maxThreads) {
                    std::lock_guard<mutex> lock(m_queueMutex);
                    m_maxPoolThreads = maxThreads <= 0 ? 1 : maxThreads;
                    while (m_workerQueue.size() > m_maxPoolThreads) {
                        m_workerQueue.pop_front();
                    }
                }
                uint32_t ThreadPool::getMaxThreads() {
                    std::lock_guard<mutex> lock(m_queueMutex);
                    return m_maxPoolThreads;
                }
                void ThreadPool::getStats(uint64_t& threadsCreated, uint64_t& threadsObtained, uint64_t& threadsReleasedToPool, uint64_t& threadsReleasedFromPool) {
                    std::lock_guard<mutex> lock(m_queueMutex);
                    threadsCreated = m_created;
                    threadsObtained = m_obtained;
                    threadsReleasedToPool = m_releasedToPool;
                    threadsReleasedFromPool = m_releasedFromPool;
                }
                std::shared_ptr<ThreadPool> ThreadPool::getDefaultThreadPool() {
                    static std::mutex singletonMutex;
                    static std::weak_ptr<ThreadPool> weakTPRef;
                    auto sharedTPRef = weakTPRef.lock();
                    if (!sharedTPRef) {
                        std::lock_guard<mutex> lock(singletonMutex);
                        sharedTPRef = weakTPRef.lock();
                        if (!sharedTPRef) {
                            sharedTPRef = std::make_shared<ThreadPool>();
                            weakTPRef = sharedTPRef;
                        }
                    }
                    return sharedTPRef;
                }
            }
        }
    }
}
