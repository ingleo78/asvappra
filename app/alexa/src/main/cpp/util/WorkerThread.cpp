#include <logger/ThreadMoniker.h>
#include <logger/Logger.h>
#include <threading/ThreadPool.h>
#include <threading/WorkerThread.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace threading {
                using namespace std;
                using namespace logger;
                WorkerThread::WorkerThread() : m_moniker{ThreadMoniker::generateMoniker()}, m_stop{false}, m_cancel{false} {
                    m_thread = thread{bind(&WorkerThread::runInternal, this)};
                }
                WorkerThread::~WorkerThread() {
                    m_stop = true;
                    unique_lock<mutex> lock(m_mutex);
                    lock.unlock();
                    m_workReady.notify_one();
                    if (m_thread.joinable()) m_thread.join();
                }
                string WorkerThread::getMoniker() const {
                    return m_moniker;
                }
                void WorkerThread::cancel() {
                    m_cancel = true;
                }
                void WorkerThread::run(function<bool()> workFunc) {
                    lock_guard<std::mutex> lock(m_mutex);
                    m_cancel = false;
                    m_workerFunc = move(workFunc);
                    m_workReady.notify_one();
                }
                void WorkerThread::runInternal() {
                    ThreadMoniker::setThisThreadMoniker(m_moniker);
                    unique_lock<std::mutex> lock(m_mutex);
                    do {
                        while (!m_workerFunc && !m_stop) m_workReady.wait(lock);
                        while (!m_stop && !m_cancel && m_workerFunc());
                        m_workerFunc = nullptr;
                    } while (!m_stop);
                }
            }
        }
    }
}
