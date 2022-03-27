#include <future>
#include <memory>
#include <thread>
#include <gmock/gmock.h>
#include <sdkinterfaces/MockMessageSender.h>
#include <json/JSONUtils.h>
#include <avs/MessageRequest.h>
#include "PostConnectVerifyGatewaySender.h"

namespace alexaClientSDK {
    namespace avsGatewayManager {
        namespace test {
            using namespace std;
            using namespace testing;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace sdkInterfaces::test;
            using namespace utils;
            using namespace json;
            using namespace jsonUtils;
            static const string EXPECTED_NAMESPACE = "Alexa.ApiGateway";
            static const string EXPECTED_NAME = "VerifyGateway";
            static const string EXPECTED_PAYLOAD = "{}";
            static const int TEST_RETRY_COUNT = 3;
            class PostConnectVerifyGatewaySenderTest : public Test {
            public:
                void SetUp() override;
                void TearDown() override;
            protected:
                bool m_callbackCheck;
                void updateStateCallback(shared_ptr<PostConnectVerifyGatewaySender> postConnectVerifyGatewaySender);
                shared_ptr<PostConnectVerifyGatewaySender> m_postConnectVerifyGatewaySender;
                thread m_mockPostConnectSenderThread;
                shared_ptr<MockMessageSender> m_mockPostConnectSendMessage;
            };
            struct EventData {
                string namespaceString;
                string nameString;
                string payloadString;
            };
            bool parseEventJson(const string& eventJson, EventData* eventData) {
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
                if (eventData.nameString != EXPECTED_NAME || eventData.namespaceString != EXPECTED_NAMESPACE ||
                    eventData.payloadString != EXPECTED_PAYLOAD) {
                    return false;
                }
                return true;
            }
            void PostConnectVerifyGatewaySenderTest::SetUp() {
                m_callbackCheck = false;
                m_mockPostConnectSendMessage = make_shared<NiceMock<MockMessageSender>>();
                auto callback = bind(&PostConnectVerifyGatewaySenderTest::updateStateCallback, this, placeholders::_1);
                m_postConnectVerifyGatewaySender = PostConnectVerifyGatewaySender::create(callback);
            }
            void PostConnectVerifyGatewaySenderTest::TearDown() {
                if (m_mockPostConnectSenderThread.joinable()) m_mockPostConnectSenderThread.join();
            }
            void PostConnectVerifyGatewaySenderTest::updateStateCallback(shared_ptr<PostConnectVerifyGatewaySender> postConnectVerifyGatewaySender) {
                if (postConnectVerifyGatewaySender == m_postConnectVerifyGatewaySender) m_callbackCheck = true;
            }
            TEST_F(PostConnectVerifyGatewaySenderTest, test_creaetWithEmptyCallbackMethod) {
                function<void(const shared_ptr<PostConnectVerifyGatewaySender>&)> callback;
                auto instance = PostConnectVerifyGatewaySender::create(callback);
            }
            TEST_F(PostConnectVerifyGatewaySenderTest, test_performGatewayWhen204IsReceived) {
                auto sendMessageLambda = [this](shared_ptr<MessageRequest> request) {
                    if (m_mockPostConnectSenderThread.joinable()) m_mockPostConnectSenderThread.join();
                    m_mockPostConnectSenderThread = thread([request]() {
                        EXPECT_TRUE(validateEvent(request->getJsonContent()));
                        request->sendCompleted(MessageRequestObserverInterface::Status::SUCCESS_NO_CONTENT);
                    });
                };
                EXPECT_CALL(*m_mockPostConnectSendMessage, sendMessage(_)).WillOnce(Invoke(sendMessageLambda));
                ASSERT_TRUE(m_postConnectVerifyGatewaySender->performOperation(m_mockPostConnectSendMessage));
                ASSERT_TRUE(m_callbackCheck);
            }
            TEST_F(PostConnectVerifyGatewaySenderTest, test_performGatewayWhen200IsReceived) {
                auto sendMessageLambda = [this](shared_ptr<MessageRequest> request) {
                    if (m_mockPostConnectSenderThread.joinable()) m_mockPostConnectSenderThread.join();
                    m_mockPostConnectSenderThread = thread([request]() {
                        EXPECT_TRUE(validateEvent(request->getJsonContent()));
                        request->sendCompleted(MessageRequestObserverInterface::Status::SUCCESS);
                    });
                };
                EXPECT_CALL(*m_mockPostConnectSendMessage, sendMessage(_)).WillOnce(Invoke(sendMessageLambda));
                ASSERT_TRUE(m_postConnectVerifyGatewaySender->performOperation(m_mockPostConnectSendMessage));
                ASSERT_FALSE(m_callbackCheck);
            }
            TEST_F(PostConnectVerifyGatewaySenderTest, test_performGatewayWhen400IsReceived) {
                auto sendMessageLambda = [this](shared_ptr<MessageRequest> request) {
                    if (m_mockPostConnectSenderThread.joinable()) m_mockPostConnectSenderThread.join();
                    m_mockPostConnectSenderThread = thread([request]() {
                        EXPECT_TRUE(validateEvent(request->getJsonContent()));
                        request->sendCompleted(MessageRequestObserverInterface::Status::BAD_REQUEST);
                    });
                };
                EXPECT_CALL(*m_mockPostConnectSendMessage, sendMessage(_)).WillOnce(Invoke(sendMessageLambda));
                ASSERT_FALSE(m_postConnectVerifyGatewaySender->performOperation(m_mockPostConnectSendMessage));
                ASSERT_FALSE(m_callbackCheck);
            }
            TEST_F(PostConnectVerifyGatewaySenderTest, test_performGatewayRetriesWhen503IsReceived) {
                promise<int> retryCountPromise;
                auto sendMessageLambda = [this, &retryCountPromise](shared_ptr<MessageRequest> request) {
                    if (m_mockPostConnectSenderThread.joinable()) m_mockPostConnectSenderThread.join();
                    m_mockPostConnectSenderThread = thread([this, request, &retryCountPromise]() {
                        EXPECT_TRUE(validateEvent(request->getJsonContent()));
                        request->sendCompleted(MessageRequestObserverInterface::Status::SERVER_INTERNAL_ERROR_V2);
                        static int count = 0;
                        count++;
                        if (TEST_RETRY_COUNT == count) {
                            retryCountPromise.set_value(count);
                            thread localThread([this]() { m_postConnectVerifyGatewaySender->abortOperation(); });
                            if (localThread.joinable()) localThread.join();
                        }
                    });
                };
                EXPECT_CALL(*m_mockPostConnectSendMessage, sendMessage(_)).WillRepeatedly(Invoke(sendMessageLambda));
                ASSERT_FALSE(m_postConnectVerifyGatewaySender->performOperation(m_mockPostConnectSendMessage));
                EXPECT_EQ(retryCountPromise.get_future().get(), TEST_RETRY_COUNT);
                ASSERT_FALSE(m_callbackCheck);
            }
        }
    }
}