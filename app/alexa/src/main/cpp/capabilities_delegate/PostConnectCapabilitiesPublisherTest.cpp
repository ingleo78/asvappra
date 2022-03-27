#include <memory>
#include <thread>
#include <gmock/gmock.h>
#include <sdkinterfaces/MockMessageSender.h>
#include <sdkinterfaces/PostConnectOperationInterface.h>
#include <json/JSONUtils.h>
#include "PostConnectCapabilitiesPublisher.h"
#include "MockAuthDelegate.h"
#include "MockDiscoveryEventSender.h"
#include "MockCapabilitiesObserver.h"

namespace alexaClientSDK {
    namespace capabilitiesDelegate {
        namespace test {
            using namespace testing;
            using namespace avs;
            using namespace utils;
            using namespace json;
            using namespace sdkInterfaces::test;
            using namespace json::jsonUtils;
            class PostConnectCapabilitiesPublisherTest : public Test {
            public:
                void SetUp() override;
                void TearDown() override;
            protected:
                shared_ptr<MockMessageSender> m_mockPostConnectSendMessage;
                shared_ptr<MockDiscoveryEventSender> m_mockDiscoveryEventSender;
                shared_ptr<PostConnectCapabilitiesPublisher> m_postConnectCapabilitiesPublisher;
            };
            void PostConnectCapabilitiesPublisherTest::SetUp() {
                m_mockPostConnectSendMessage = make_shared<StrictMock<MockMessageSender>>();
                //m_mockDiscoveryEventSender = make_shared<StrictMock<MockDiscoveryEventSender>>();
                m_postConnectCapabilitiesPublisher = PostConnectCapabilitiesPublisher::create(m_mockDiscoveryEventSender);
            }
            void PostConnectCapabilitiesPublisherTest::TearDown() {
                EXPECT_CALL(*m_mockDiscoveryEventSender, stop()).Times(1);
            }
            TEST_F(PostConnectCapabilitiesPublisherTest, test_createWithInvalidParams) {
                auto instance = PostConnectCapabilitiesPublisher::create(nullptr);
                ASSERT_EQ(instance, nullptr);
            }
            TEST_F(PostConnectCapabilitiesPublisherTest, test_getPostConnectOperationPriority) {
                ASSERT_EQ(static_cast<unsigned int>(PostConnectOperationInterface::ENDPOINT_DISCOVERY_PRIORITY),
                     m_postConnectCapabilitiesPublisher->getOperationPriority());
            }
            TEST_F(PostConnectCapabilitiesPublisherTest, test_performOperationWithInvalidPostConnectSender) {
                ASSERT_FALSE(m_postConnectCapabilitiesPublisher->performOperation(nullptr));
            }
            TEST_F(PostConnectCapabilitiesPublisherTest, test_performOperationSendsDiscoveryEvents) {
                EXPECT_CALL(*m_mockDiscoveryEventSender, sendDiscoveryEvents(_)).WillOnce(Return(true));
                ASSERT_TRUE(m_postConnectCapabilitiesPublisher->performOperation(m_mockPostConnectSendMessage));
            }
            TEST_F(PostConnectCapabilitiesPublisherTest, test_performOperationFailsWhenCalledTwice) {
                EXPECT_CALL(*m_mockDiscoveryEventSender, sendDiscoveryEvents(_)).WillOnce(Return(true));
                ASSERT_TRUE(m_postConnectCapabilitiesPublisher->performOperation(m_mockPostConnectSendMessage));
                ASSERT_FALSE(m_postConnectCapabilitiesPublisher->performOperation(m_mockPostConnectSendMessage));
            }
            TEST_F(PostConnectCapabilitiesPublisherTest, test_performOperationFailsWhenSendDiscoveryEventsFails) {
                EXPECT_CALL(*m_mockDiscoveryEventSender, sendDiscoveryEvents(_)).WillOnce(Return(false));
                ASSERT_FALSE(m_postConnectCapabilitiesPublisher->performOperation(m_mockPostConnectSendMessage));
            }
            TEST_F(PostConnectCapabilitiesPublisherTest, test_abortStopsSendDiscoveryEvents) {
                EXPECT_CALL(*m_mockDiscoveryEventSender, stop()).Times(1);
                m_postConnectCapabilitiesPublisher->abortOperation();
            }
        }
    }
}