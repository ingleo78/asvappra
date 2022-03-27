#include <chrono>
#include <string>
#include <logger/Logger.h>
#include <logger/ThreadMoniker.h>
#include <threading/TaskThread.h>

static const std::string TAG("TaskThread");
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace threading {
                using namespace logger;
                using namespace std::chrono;
                TaskThread::TaskThread() : m_shuttingDown{false}, m_stop{false}, m_alreadyStarting{false}, m_threadPool{ThreadPool::getDefaultThreadPool()} {}
                TaskThread::~TaskThread() {
                    for (;;) {
                        {
                            std::lock_guard<std::mutex> guard(m_mutex);
                            if (!m_alreadyStarting || m_workerThread == nullptr) {
                                m_shuttingDown = true;
                                return;
                            }
                        }
                        while (m_alreadyStarting) {
                            std::this_thread::yield();
                        }
                        m_stop = true;
                    }
                }
                bool TaskThread::start(std::function<bool()> jobRunner) {
                    if (!jobRunner) {
                        ACSDK_ERROR(LX("startFailed").d("reason", "invalidFunction"));
                        return false;
                    }
                    bool notRunning = false;
                    if (!m_alreadyStarting.compare_exchange_strong(notRunning, true)) {
                        ACSDK_ERROR(LX("startFailed").d("reason", "tooManyThreads"));
                        return false;
                    }
                    m_startTime = steady_clock::now();
                    m_stop = true;
                    std::lock_guard<std::mutex> guard(m_mutex);
                    if (m_shuttingDown) {
                        ACSDK_ERROR(LX("startFailed").d("reason", "shuttingDown"));
                        return false;
                    }
                    m_workerThread = m_threadPool->obtainWorker(m_moniker);
                    m_moniker = m_workerThread->getMoniker();
                    m_workerThread->run([this, jobRunner] {
                        TaskThread::run(jobRunner);
                        return false;
                    });
                    return true;
                }
                void TaskThread::run(std::function<bool()> jobRunner) {
                    std::lock_guard<std::mutex> guard(m_mutex);
                    ACSDK_DEBUG9(LX("startThread")
                                     .d("moniker", m_moniker)
                                     .d("duration", duration_cast<microseconds>(steady_clock::now() - m_startTime).count()));
                    m_stop = false;
                    m_alreadyStarting = false;
                    while (!m_stop && jobRunner());
                    m_threadPool->releaseWorker(std::move(m_workerThread));
                }
            }
        }
    }
}
