#include <future>
#include <memory>
#include <thread>
#include <gmock/gmock.h>
#include <sdkinterfaces/MockContextManager.h>
#include <sdkinterfaces/MockMessageSender.h>
#include <json/JSONUtils.h>
#include "PostConnectSynchronizeStateSender.h"

namespace alexaClientSDK {
    namespace synchronizeStateSender {
        namespace test {
            using namespace std;
            using namespace chrono;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace sdkInterfaces::test;
            using namespace utils;
            using namespace json;
            using namespace jsonUtils;
            using namespace testing;
            static const string TEST_CONTEXT_VALUE = "{}";
            static const string EXPECTED_NAMESPACE = "System";
            static const string EXPECTED_NAME = "SynchronizeState";
            static const string EXPECTED_PAYLOAD = "{}";
            static const ContextRequestToken MOCK_CONTEXT_REQUEST_TOKEN = 1;
            static const int TEST_RETRY_COUNT = 3;
            class PostConnectSynchronizeStateSenderTest : public Test {
            public:
                void SetUp() override;
                void TearDown() override;
            protected:
                shared_ptr<MockContextManager> m_mockContextManager;
                shared_ptr<MockMessageSender> m_mockPostConnectSendMessage;
                thread m_mockContextManagerThread;
                thread m_mockPostConnectSenderThread;
                shared_ptr<PostConnectSynchronizeStateSender> m_postConnectSynchronizeStateSender;
            };
            struct EventData {
                string contextString;
                string namespaceString;
                string nameString;
                string payloadString;
            };
            bool parseEventJson(const string& eventJson, EventData* eventData) {
                if (!retrieveValue(eventJson, "context", &eventData->contextString)) return false;
                string eventString;
                if (!retrieveValue(eventJson, "event", &eventString)) return false;
                string headerString;
                if (!retrieveValue(eventString, "header", &headerString)) return false;
                if (!retrieveValue(headerString, "namespace", &eventData->namespaceString)) return false;
                if (!retrieveValue(headerString, "name", &eventData->nameString)) return false;
                if (!retrieveValue(eventString, "payload", &eventData->payloadString)) return false;
                return true;
            }
            bool validateEvent(const string& eventJson) {
                EventData eventData;
                if (!parseEventJson(eventJson, &eventData)) return false;
                if (eventData.contextString != TEST_CONTEXT_VALUE || eventData.nameString != EXPECTED_NAME ||
                    eventData.namespaceString != EXPECTED_NAMESPACE || eventData.payloadString != EXPECTED_PAYLOAD) {
                    return false;
                }
                return true;
            }
            void PostConnectSynchronizeStateSenderTest::SetUp() {
                m_mockContextManager = make_shared<NiceMock<MockContextManager>>();
                m_mockPostConnectSendMessage = make_shared<NiceMock<MockMessageSender>>();
                m_postConnectSynchronizeStateSender = PostConnectSynchronizeStateSender::create(m_mockContextManager);
            }
            void PostConnectSynchronizeStateSenderTest::TearDown() {
                if (m_mockContextManagerThread.joinable()) m_mockContextManagerThread.join();
                if (m_mockPostConnectSenderThread.joinable()) m_mockPostConnectSenderThread.join();
            }
            TEST_F(PostConnectSynchronizeStateSenderTest, test_createWithNullContextManager) {
                auto instance = PostConnectSynchronizeStateSender::create(nullptr);
                ASSERT_EQ(instance, nullptr);
            }
            TEST_F(PostConnectSynchronizeStateSenderTest, test_getOperationPriority) {
                ASSERT_EQ(m_postConnectSynchronizeStateSender->getOperationPriority(),
                    static_cast<unsigned int>(PostConnectOperationInterface::SYNCHRONIZE_STATE_PRIORITY));
            }
            TEST_F(PostConnectSynchronizeStateSenderTest, test_perfromOperationSendsSynchronizeStateEvent) {
                auto getContextLambda = [this](shared_ptr<ContextRequesterInterface> contextRequester, const string& endpointId,
                                               const milliseconds& timeout) {
                    if (m_mockContextManagerThread.joinable()) m_mockContextManagerThread.join();
                    m_mockContextManagerThread = thread([contextRequester]() { contextRequester->onContextAvailable(TEST_CONTEXT_VALUE); });
                    return MOCK_CONTEXT_REQUEST_TOKEN;
                };
                EXPECT_CALL(*m_mockContextManager, getContext(_, _, _)).WillOnce(Invoke(getContextLambda));
                auto sendMessageLambda = [this](shared_ptr<MessageRequest> request) {
                    if (m_mockPostConnectSenderThread.joinable()) m_mockPostConnectSenderThread.join();
                    m_mockPostConnectSenderThread = thread([request]() {
                        EXPECT_TRUE(validateEvent(request->getJsonContent()));
                        request->sendCompleted(MessageRequestObserverInterface::Status::SUCCESS_NO_CONTENT);
                    });
                };
                EXPECT_CALL(*m_mockPostConnectSendMessage, sendMessage(_)).WillOnce(Invoke(sendMessageLambda));
                ASSERT_TRUE(m_postConnectSynchronizeStateSender->performOperation(m_mockPostConnectSendMessage));
            }
            TEST_F(PostConnectSynchronizeStateSenderTest, test_performOperationRetriesOnContextFailure) {
                promise<int> retryCountPromise;
                auto getContextLambda = [this, &retryCountPromise](shared_ptr<ContextRequesterInterface> contextRequester, const string& endpointId,
                                                                   const milliseconds& timeout) {
                    if (m_mockContextManagerThread.joinable()) m_mockContextManagerThread.join();
                    m_mockContextManagerThread = thread([this, contextRequester, &retryCountPromise]() {
                        contextRequester->onContextFailure(ContextRequestError::STATE_PROVIDER_TIMEDOUT);
                        static int count = 0;
                        count++;
                        if (count == TEST_RETRY_COUNT) {
                            retryCountPromise.set_value(count);
                            m_postConnectSynchronizeStateSender->abortOperation();
                        }
                    });
                    return MOCK_CONTEXT_REQUEST_TOKEN;
                };
                EXPECT_CALL(*m_mockContextManager, getContext(_, _, _)).WillRepeatedly(Invoke(getContextLambda));
                EXPECT_CALL(*m_mockPostConnectSendMessage, sendMessage(_)).Times(0);
                EXPECT_FALSE(m_postConnectSynchronizeStateSender->performOperation(m_mockPostConnectSendMessage));
                EXPECT_EQ(retryCountPromise.get_future().get(), TEST_RETRY_COUNT);
            }
            TEST_F(PostConnectSynchronizeStateSenderTest, test_testPerfromOperationRetriesOnUnsuccessfulResponse) {
                promise<int> retryCountPromise;
                auto getContextLambda = [this](shared_ptr<ContextRequesterInterface> contextRequester, const string& endpointId, const milliseconds& timeout) {
                    if (m_mockContextManagerThread.joinable()) m_mockContextManagerThread.join();
                    m_mockContextManagerThread = thread([contextRequester]() { contextRequester->onContextAvailable(TEST_CONTEXT_VALUE); });
                    return MOCK_CONTEXT_REQUEST_TOKEN;
                };
                EXPECT_CALL(*m_mockContextManager, getContext(_, _, _)).WillRepeatedly(Invoke(getContextLambda));
                auto sendMessageLambda = [this, &retryCountPromise](std::shared_ptr<MessageRequest> request) {
                    if (m_mockPostConnectSenderThread.joinable()) m_mockPostConnectSenderThread.join();
                    m_mockPostConnectSenderThread = thread([this, request, &retryCountPromise]() {
                        ASSERT_TRUE(validateEvent(request->getJsonContent()));
                        request->sendCompleted(MessageRequestObserverInterface::Status::SERVER_INTERNAL_ERROR_V2);
                        static int count = 0;
                        count++;
                        if (count == TEST_RETRY_COUNT) {
                            retryCountPromise.set_value(count);
                            thread localThread([this]() { m_postConnectSynchronizeStateSender->abortOperation(); });
                            if (localThread.joinable()) localThread.join();
                        }
                    });
                };
                EXPECT_CALL(*m_mockPostConnectSendMessage, sendMessage(_)).WillRepeatedly(Invoke(sendMessageLambda));
                EXPECT_FALSE(m_postConnectSynchronizeStateSender->performOperation(m_mockPostConnectSendMessage));
                EXPECT_EQ(retryCountPromise.get_future().get(), TEST_RETRY_COUNT);
            }
            TEST_F(PostConnectSynchronizeStateSenderTest, test_abortOperationWhenContextRequestInProgress) {
                auto getContextLambda = [this](shared_ptr<ContextRequesterInterface> contextRequester,
                                            const string& endpointId,
                                            const milliseconds& timeout) {
                    if (m_mockContextManagerThread.joinable()) m_mockContextManagerThread.join();
                    m_mockContextManagerThread = thread([this]() {
                        this_thread::sleep_for(milliseconds(500));
                        thread localThread([this]() { m_postConnectSynchronizeStateSender->abortOperation(); });
                        if (localThread.joinable()) localThread.join();
                    });
                    return MOCK_CONTEXT_REQUEST_TOKEN;
                };
                EXPECT_CALL(*m_mockContextManager, getContext(_, _, _)).WillRepeatedly(Invoke(getContextLambda));
                EXPECT_CALL(*m_mockPostConnectSendMessage, sendMessage(_)).Times(0);
                EXPECT_FALSE(m_postConnectSynchronizeStateSender->performOperation(m_mockPostConnectSendMessage));
            }
            TEST_F(PostConnectSynchronizeStateSenderTest, test_abortOperationWhenSendMessageInProgress) {
                auto getContextLambda = [this](shared_ptr<ContextRequesterInterface> contextRequester, const string& endpointId, const milliseconds& timeout) {
                    if (m_mockContextManagerThread.joinable()) m_mockContextManagerThread.join();
                    m_mockContextManagerThread = thread([contextRequester]() { contextRequester->onContextAvailable(TEST_CONTEXT_VALUE); });
                    return MOCK_CONTEXT_REQUEST_TOKEN;
                };
                EXPECT_CALL(*m_mockContextManager, getContext(_, _, _)).WillRepeatedly(Invoke(getContextLambda));
                auto sendMessageLambda = [this](shared_ptr<MessageRequest> request) {
                    if (m_mockPostConnectSenderThread.joinable()) m_mockPostConnectSenderThread.join();
                    m_mockPostConnectSenderThread = thread([this, request]() {
                        ASSERT_TRUE(validateEvent(request->getJsonContent()));
                        this_thread::sleep_for(milliseconds(500));
                        thread localThread([this]() { m_postConnectSynchronizeStateSender->abortOperation(); });
                        if (localThread.joinable()) localThread.join();
                    });
                };
                EXPECT_CALL(*m_mockPostConnectSendMessage, sendMessage(_)).WillOnce(Invoke(sendMessageLambda));
                EXPECT_FALSE(m_postConnectSynchronizeStateSender->performOperation(m_mockPostConnectSendMessage));
            }
        }
    }
}
