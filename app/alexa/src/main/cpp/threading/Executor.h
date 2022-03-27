#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_THREADING_EXECUTOR_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_THREADING_EXECUTOR_H_

#include <atomic>
#include <condition_variable>
#include <chrono>
#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <utility>
#include "TaskThread.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace threading {
                class Executor {
                public:
                    Executor(const std::chrono::milliseconds& delayExit = std::chrono::milliseconds(1000));
                    ~Executor();
                    template <typename Task, typename... Args> auto submit(Task task, Args&&... args) -> std::future<decltype(task(args...))>;
                    template <typename Task, typename... Args>
                    auto submitToFront(Task task, Args&&... args) -> std::future<decltype(task(args...))>;
                    void waitForSubmittedTasks();
                    void shutdown();
                    bool isShutdown();
                private:
                    using Queue = std::deque<std::function<void()>>;
                    bool runNext();
                    bool hasNext();
                    std::function<void()> pop();
                    template <typename Task, typename... Args> auto pushTo(bool front, Task task, Args&&... args) -> std::future<decltype(task(args...))>;
                    Queue m_queue;
                    bool m_threadRunning;
                    std::chrono::milliseconds m_timeout;
                    std::mutex m_queueMutex;
                    std::atomic_bool m_shutdown;
                    std::condition_variable m_delayedCondition;
                    TaskThread m_taskThread;
                };
                template <typename Task, typename... Args> auto Executor::submit(Task task, Args&&... args) -> std::future<decltype(task(args...))> {
                    bool front = false;
                    return pushTo(front, std::forward<Task>(task), std::forward<Args>(args)...);
                }
                template <typename Task, typename... Args> auto Executor::submitToFront(Task task, Args&&... args) -> std::future<decltype(task(args...))> {
                    bool front = true;
                    return pushTo(front, std::forward<Task>(task), std::forward<Args>(args)...);
                }
                template <typename T> inline static void forwardPromise(std::shared_ptr<std::promise<T>> promise, std::future<T>* future) {
                    promise->set_value(future->get());
                }
                template <> inline void forwardPromise<void>(std::shared_ptr<std::promise<void>> promise, std::future<void>* future) {
                    future->get();
                    promise->set_value();
                }
                template <typename Task, typename... Args> auto Executor::pushTo(bool front, Task task, Args&&... args) -> std::future<decltype(task(args...))> {
                    auto boundTask = std::bind(std::forward<Task>(task), std::forward<Args>(args)...);
                    using PackagedTaskType = std::packaged_task<decltype(boundTask())()>;
                    auto packaged_task = std::make_shared<PackagedTaskType>(boundTask);
                    auto cleanupPromise = std::make_shared<std::promise<decltype(task(args...))>>();
                    auto cleanupFuture = cleanupPromise->get_future();
                    auto translated_task = [packaged_task, cleanupPromise]() mutable {
                        packaged_task->operator()();
                        auto taskFuture = packaged_task->get_future();
                        packaged_task.reset();
                        forwardPromise(cleanupPromise, &taskFuture);
                    };
                    packaged_task.reset();
                    {
                        bool restart = false;
                        std::lock_guard<std::mutex> queueLock{m_queueMutex};
                        if (!m_shutdown) {
                            restart = !m_threadRunning;
                            m_queue.emplace(front ? m_queue.begin() : m_queue.end(), std::move(translated_task));
                        } else {
                            using FutureType = decltype(task(args...));
                            return std::future<FutureType>();
                        }
                        if (restart) {
                            m_taskThread.start(std::bind(&Executor::runNext, this));
                            m_threadRunning = true;
                        }
                    }
                    m_delayedCondition.notify_one();
                    return cleanupFuture;
                }
            }
        }
    }
}
#endif