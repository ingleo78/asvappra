#ifndef ALEXA_CLIENT_SDK_ADSL_TEST_COMMON_MOCKDIRECTIVEHANDLER_H_
#define ALEXA_CLIENT_SDK_ADSL_TEST_COMMON_MOCKDIRECTIVEHANDLER_H_

#include <chrono>
#include <future>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sdkinterfaces/DirectiveHandlerInterface.h>
#include <avs/NamespaceAndName.h>

namespace alexaClientSDK {
    namespace adsl {
        namespace test {
            using namespace std;
            using namespace chrono;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace testing;
            class DirectiveHandlerMockAdapter : public DirectiveHandlerInterface {
            public:
                void preHandleDirective(shared_ptr<AVSDirective> directive, unique_ptr<DirectiveHandlerResultInterface> result) override;
                virtual void preHandleDirective(shared_ptr<AVSDirective> directive, shared_ptr<DirectiveHandlerResultInterface> result) = 0;
            };
            class MockDirectiveHandler : public DirectiveHandlerMockAdapter {
            public:
                static shared_ptr<NiceMock<MockDirectiveHandler>> create(DirectiveHandlerConfiguration config,
                                                                         milliseconds handlingTimeMs = DEFAULT_HANDLING_TIME_MS);
                MockDirectiveHandler(DirectiveHandlerConfiguration config, milliseconds handlingTimeMs);
                ~MockDirectiveHandler();
                void mockHandleDirectiveImmediately(shared_ptr<AVSDirective> directive);
                void mockPreHandleDirective(shared_ptr<AVSDirective> directive, shared_ptr<DirectiveHandlerResultInterface> result);
                bool mockHandleDirective(const string& messageId);
                void mockCancelDirective(const string& messageId);
                void mockOnDeregistered();
                void doHandleDirective(const string& messageId);
                void doPreHandlingFailed(shared_ptr<AVSDirective> directive, shared_ptr<DirectiveHandlerResultInterface> result);
                void doHandlingCompleted();
                bool doHandlingFailed(const string& messageId);
                void shutdown();
                bool waitUntilPreHandling(milliseconds timeout = DEFAULT_DONE_TIMEOUT_MS);
                bool waitUntilHandling(milliseconds timeout = DEFAULT_DONE_TIMEOUT_MS);
                bool waitUntilCanceling(milliseconds timeout = DEFAULT_DONE_TIMEOUT_MS);
                bool waitUntilCompleted(milliseconds timeout = DEFAULT_DONE_TIMEOUT_MS);
                milliseconds m_handlingTimeMs;
                shared_ptr<DirectiveHandlerResultInterface> m_result;
                shared_ptr<AVSDirective> m_directive;
                thread m_doHandleDirectiveThread;
                mutex m_mutex;
                condition_variable m_wakeNotifier;
                bool m_isCompleted;
                bool m_isShuttingDown;
                promise<void> m_preHandlingPromise;
                future<void> m_preHandlingFuture;
                promise<void> m_handlingPromise;
                future<void> m_handlingFuture;
                promise<void> m_cancelingPromise;
                shared_future<void> m_cancelingFuture;
                promise<void> m_completedPromise;
                future<void> m_completedFuture;
                MOCK_METHOD1(handleDirectiveImmediately, void(shared_ptr<AVSDirective>));
                //MOCK_METHOD2(preHandleDirective, void(shared_ptr<AVSDirective>, shared_ptr<DirectiveHandlerResultInterface>));
                MOCK_METHOD1(handleDirective, bool(const string&));
                MOCK_METHOD1(cancelDirective, void(const string&));
                MOCK_METHOD0(onDeregistered, void());
                MOCK_CONST_METHOD0(getConfiguration, DirectiveHandlerConfiguration());
                static const milliseconds DEFAULT_HANDLING_TIME_MS;
                static const milliseconds DEFAULT_DONE_TIMEOUT_MS;
            };
        }
    }
}
#endif