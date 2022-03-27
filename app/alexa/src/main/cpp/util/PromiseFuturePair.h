#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_PROMISEFUTUREPAIR_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_PROMISEFUTUREPAIR_H_

#include <future>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            template <typename Type> class PromiseFuturePair {
            public:
                PromiseFuturePair() : m_future{m_promise.get_future()} {}
                void setValue(Type val) {
                    m_promise.set_value(val);
                }
                bool waitFor(std::chrono::milliseconds timeout) {
                    auto future = m_future;
                    return future.wait_for(timeout) == std::future_status::ready;
                }
                Type getValue() {
                    auto future = m_future;
                    return future.get();
                }
            private:
                std::promise<Type> m_promise;
                std::shared_future<Type> m_future;
            };
            template <> class PromiseFuturePair<void> {
            public:
                PromiseFuturePair() : m_future{m_promise.get_future()} {}
                void setValue() {
                    m_promise.set_value();
                }
                bool waitFor(std::chrono::milliseconds timeout) {
                    auto future = m_future;
                    return future.wait_for(timeout) == std::future_status::ready;
                }
            private:
                std::promise<void> m_promise;
                std::shared_future<void> m_future;
            };
        }
    }
}
#endif