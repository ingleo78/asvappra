#include <algorithm>
#include <timing/MultiTimer.h>
#include <logger/Logger.h>
#include <timing/Timer.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace timing {
                static const std::string TAG("MultiTimer");
                #define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)
                static const std::chrono::milliseconds GRACE_PERIOD{500};
                MultiTimer::MultiTimer() : m_isRunning{false}, m_isBeingDestroyed{false}, m_nextToken{0} {}
                MultiTimer::~MultiTimer() {
                    std::unique_lock<std::mutex> lock{m_waitMutex};
                    m_timers.clear();
                    m_tasks.clear();
                    m_isBeingDestroyed = true;
                    if (m_isRunning) {
                        lock.unlock();
                        m_waitCondition.notify_all();
                    }
                }
                MultiTimer::Token MultiTimer::submitTask(const std::chrono::milliseconds& delay, std::function<void()> task) {
                    std::unique_lock<std::mutex> lock{m_waitMutex};
                    auto token = m_nextToken++;
                    TimePoint timePoint = std::chrono::steady_clock::now() + delay;
                    m_timers.insert({timePoint, token});
                    m_tasks.insert({token, {timePoint, task}});
                    if (!m_isRunning) {
                        m_isRunning = true;
                        m_timerThread.start(std::bind(&MultiTimer::executeTimer, this));
                    } else {
                        if (m_timers.begin()->second == token) {
                            lock.unlock();
                            m_waitCondition.notify_one();
                        }
                    }
                    return token;
                }
                void MultiTimer::cancelTask(Token token) {
                    std::unique_lock<std::mutex> lock{m_waitMutex};
                    auto taskIt = m_tasks.find(token);
                    if (taskIt != m_tasks.end()) {
                        auto& timePoint = taskIt->second.first;
                        bool isNext = m_timers.begin()->first == timePoint;
                        auto timerIt = std::find(m_timers.begin(), m_timers.end(), decltype(m_timers)::value_type(timePoint, token));
                        m_timers.erase(timerIt);
                        m_tasks.erase(token);
                        if (isNext) {
                            lock.unlock();
                            m_waitCondition.notify_one();
                        }
                    }
                }
                bool MultiTimer::executeTimer() {
                    std::unique_lock<std::mutex> lock{m_waitMutex};
                    while (!m_timers.empty()) {
                        auto now = std::chrono::steady_clock::now();
                        auto nextIt = m_timers.begin();
                        auto& nextTime = nextIt->first;
                        if (nextTime > now) {
                            auto waitTime = std::chrono::duration_cast<std::chrono::milliseconds>(nextTime - now);
                            m_waitCondition.wait_for(lock, waitTime, [this] { return m_isBeingDestroyed; });
                        } else {
                            auto taskIt = m_tasks.find(nextIt->second);
                            if (taskIt != m_tasks.end()) {
                                auto& task = taskIt->second.second;
                                task();
                                m_tasks.erase(taskIt);
                            }
                            m_timers.erase(nextIt);
                        }
                    }
                    return hasNextLocked(lock);
                }
                bool MultiTimer::hasNextLocked(std::unique_lock<std::mutex>& lock) {
                    m_waitCondition.wait_for(lock, GRACE_PERIOD, [this] { return (!m_tasks.empty()) || m_isBeingDestroyed; });
                    m_isRunning = (!m_isBeingDestroyed) && !m_tasks.empty();
                    return m_isRunning;
                }
            }
        }
    }
}
