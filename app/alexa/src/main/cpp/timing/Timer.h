#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_TIMING_TIMER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_TIMING_TIMER_H_

#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <thread>
#include <logger/LoggerUtils.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace timing {
                using namespace std;
                using namespace logger;
                using namespace chrono;
                class Timer {
                public:
                    static const size_t FOREVER = 0;
                    enum class PeriodType {
                        ABSOLUTE,
                        RELATIVE
                    };
                    Timer();
                    ~Timer();
                    static size_t getForever() {
                        return FOREVER;
                    }
                    static std::string getTag() {
                        return "Timer";
                    }
                    template <typename Rep, typename Period, typename Task, typename... Args>
                    bool start(
                        const std::chrono::duration<Rep, Period>& delay,
                        const std::chrono::duration<Rep, Period>& period,
                        PeriodType periodType,
                        size_t maxCount,
                        Task task,
                        Args&&... args);
                    template <typename Rep, typename Period, typename Task, typename... Args> bool start(const duration<Rep, Period>& period, PeriodType periodType, size_t maxCount, Task task, Args&&... args);
                    template <typename Rep, typename Period, typename Task, typename... Args>
                    auto start(const duration<Rep, Period>& delay, Task task, Args&&... args)->future<decltype(task(args...))>;
                    void stop();
                    bool isActive() const;
                private:
                    bool activate();
                    template <typename Rep, typename Period> void callTask(duration<Rep, Period> delay, duration<Rep, Period> period, PeriodType periodType, size_t maxCount,
                                               function<void()> task);
                    condition_variable m_waitCondition;
                    mutex m_waitMutex;
                    mutex m_joinMutex;
                    thread m_thread;
                    atomic<bool> m_running;
                    bool m_stopping;
                };
                template <typename Rep, typename Period, typename Task, typename... Args> bool Timer::start(const duration<Rep, Period>& delay, const duration<Rep, Period>& period, PeriodType periodType,
                                                            size_t maxCount, Task task, Args&&... args) {
                    if (delay < duration<Rep, Period>::zero()) {
                        acsdkError(LogEntry(Timer::getTag(), "startFailed").d("reason", "negativeDelay"));
                        return false;
                    }
                    if (period < duration<Rep, Period>::zero()) {
                        acsdkError(LogEntry(Timer::getTag(), "startFailed").d("reason", "negativePeriod"));
                        return false;
                    }
                    if (!activate()) {
                        acsdkError(LogEntry(Timer::getTag(), "startFailed").d("reason", "timerAlreadyActive"));
                        return false;
                    }
                    if (m_thread.joinable()) m_thread.join();
                    using BoundTaskType = decltype(bind(forward<Task>(task), forward<Args>(args)...));
                    auto boundTask = make_shared<BoundTaskType>(bind(std::forward<Task>(task), std::forward<Args>(args)...));
                    auto translatedTask = [boundTask]() { boundTask->operator()(); };
                    m_thread = thread{std::bind(&Timer::callTask<Rep, Period>, this, delay, period, periodType, maxCount, translatedTask)};
                    return true;
                }
                template <typename Rep, typename Period, typename Task, typename... Args> bool Timer::start(const duration<Rep, Period>& period, PeriodType periodType, size_t maxCount,
                                                            Task task, Args&&... args) {
                    return start(period, period, periodType, maxCount, std::forward<Task>(task), std::forward<Args>(args)...);
                }
                template <typename Rep, typename Period, typename Task, typename... Args> auto Timer::start(const duration<Rep, Period>& delay, Task task, Args&&... args)->future<decltype(task(args...))> {
                    if (!activate()) {
                        acsdkError(LogEntry(Timer::getTag(), "startFailed").d("reason", "timerAlreadyActive"));
                        using FutureType = decltype(task(args...));
                        return future<FutureType>();
                    }
                    if (m_thread.joinable()) m_thread.join();
                    auto boundTask = bind(std::forward<Task>(task), forward<Args>(args)...);
                    using PackagedTaskType = packaged_task<decltype(boundTask())()>;
                    auto packagedTask = make_shared<PackagedTaskType>(boundTask);
                    auto translatedTask = [packagedTask]() { packagedTask->operator()(); };
                    static const size_t once = 1;
                    m_thread = thread{bind(&Timer::callTask<Rep, Period>, this, delay, delay, PeriodType::ABSOLUTE, once, translatedTask)};
                    return packagedTask->get_future();
                }
                template <typename Rep, typename Period> void Timer::callTask(duration<Rep, Period> delay, duration<Rep, Period> period, PeriodType periodType,
                    size_t maxCount,
                    function<void()> task) {
                    auto now = steady_clock::now();
                    bool offSchedule = false;
                    for (size_t count = 0; maxCount == FOREVER || count < maxCount; ++count) {
                        auto waitTime = (0 == count) ? delay : period;
                        {
                            std::unique_lock<std::mutex> lock(m_waitMutex);
                            if (m_waitCondition.wait_until(lock, now + waitTime, [this]() { return m_stopping; })) {
                                m_stopping = false;
                                m_running = false;
                                return;
                            }
                        }
                        switch (periodType) {
                            case PeriodType::ABSOLUTE:
                                now += waitTime;
                                if (!offSchedule) task();
                                if (now + period < std::chrono::steady_clock::now()) offSchedule = true;
                                else offSchedule = false;
                                break;
                            case PeriodType::RELATIVE:
                                task();
                                now = std::chrono::steady_clock::now();
                                break;
                        }
                    }
                    m_stopping = false;
                    m_running = false;
                }
            }
        }
    }
}
#endif