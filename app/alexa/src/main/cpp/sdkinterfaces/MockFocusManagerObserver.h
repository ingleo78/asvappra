#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKFOCUSMANAGEROBSERVER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKFOCUSMANAGEROBSERVER_H_

#include <gmock/gmock.h>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include "FocusManagerObserverInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace test {
                class MockFocusManagerObserver : public FocusManagerObserverInterface {
                public:
                    MockFocusManagerObserver();
                    MOCK_METHOD2(onFocusChanged, void(const std::string& channelName, avs::FocusState newFocus));
                    void expectFocusChange(const std::string& channelName, avs::FocusState newFocus);
                    bool waitForFocusChanges(std::chrono::milliseconds timeout = std::chrono::milliseconds::zero());
                private:
                    size_t m_expects;
                    std::mutex m_mutex;
                    std::condition_variable m_conditionVariable;
                };
                inline MockFocusManagerObserver::MockFocusManagerObserver() : m_expects{0} {}
                inline void MockFocusManagerObserver::expectFocusChange(const std::string& channelName, avs::FocusState newFocus) {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    EXPECT_CALL(*this, onFocusChanged(testing::StrEq(channelName), newFocus))
                        .WillOnce(testing::InvokeWithoutArgs([this] {
                            std::lock_guard<std::mutex> lock(m_mutex);
                            --m_expects;
                            m_conditionVariable.notify_all();
                        }));
                    ++m_expects;
                }
                inline bool MockFocusManagerObserver::waitForFocusChanges(std::chrono::milliseconds timeout) {
                    std::unique_lock<std::mutex> lock(m_mutex);
                    return m_conditionVariable.wait_for(lock, timeout, [this] { return !m_expects; });
                }
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKFOCUSMANAGEROBSERVER_H_
