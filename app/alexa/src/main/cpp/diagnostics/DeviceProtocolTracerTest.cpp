#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <json/JSONUtils.h>
#include "DeviceProtocolTracer.h"

namespace alexaClientSDK {
    namespace diagnostics {
        namespace test {
            class DeviceProtocolTracerTest : public ::testing::Test {
            public:
                void SetUp() override;
                void TearDown() override;
                std::shared_ptr<DeviceProtocolTracer> m_deviceProtocolTracer;
            };
            void DeviceProtocolTracerTest::SetUp() {
                m_deviceProtocolTracer = DeviceProtocolTracer::create();
            }

            void DeviceProtocolTracerTest::TearDown() {
                m_deviceProtocolTracer->setProtocolTraceFlag(false);
                m_deviceProtocolTracer->clearTracedMessages();
            }
            TEST_F(DeviceProtocolTracerTest, test_ifProtocolTracingIsDisabledByDefault) {
                m_deviceProtocolTracer->setMaxMessages(100);
                m_deviceProtocolTracer->receive("contextId1", "Directive1");
                m_deviceProtocolTracer->receive("contextId1", "Directive1");
                m_deviceProtocolTracer->traceEvent("Event1");
                ASSERT_EQ(m_deviceProtocolTracer->getProtocolTrace(), "[]");
            }
            TEST_F(DeviceProtocolTracerTest, test_protocolTraceWithTraceFlagDisabled) {
                m_deviceProtocolTracer->setProtocolTraceFlag(false);
                m_deviceProtocolTracer->setMaxMessages(100);
                m_deviceProtocolTracer->receive("contextId1", "Directive1");
                m_deviceProtocolTracer->receive("contextId1", "Directive1");
                m_deviceProtocolTracer->traceEvent("Event1");
                ASSERT_EQ(m_deviceProtocolTracer->getProtocolTrace(), "[]");
            }
            TEST_F(DeviceProtocolTracerTest, test_protocolTraceWithTraceFlagEnabled) {
                m_deviceProtocolTracer->setProtocolTraceFlag(true);
                m_deviceProtocolTracer->setMaxMessages(100);
                m_deviceProtocolTracer->receive("contextId1", "Directive1");
                m_deviceProtocolTracer->receive("contextId2", "Directive2");
                m_deviceProtocolTracer->traceEvent("Event1");
                ASSERT_EQ(m_deviceProtocolTracer->getProtocolTrace(), "[Directive1,Directive2,Event1]");
            }
            TEST_F(DeviceProtocolTracerTest, test_ifProtocolTracingTracesOneMessageByDefault) {
                m_deviceProtocolTracer->setProtocolTraceFlag(true);
                m_deviceProtocolTracer->receive("contextId1", "Directive1");
                m_deviceProtocolTracer->receive("contextId1", "Directive2");
                m_deviceProtocolTracer->traceEvent("Event1");
                ASSERT_EQ(m_deviceProtocolTracer->getProtocolTrace(), "[Directive1]");
            }
            TEST_F(DeviceProtocolTracerTest, test_clearTracedMessages) {
                m_deviceProtocolTracer->setProtocolTraceFlag(true);
                m_deviceProtocolTracer->setMaxMessages(100);
                m_deviceProtocolTracer->receive("contextId1", "Directive1");
                m_deviceProtocolTracer->receive("contextId2", "Directive2");
                m_deviceProtocolTracer->traceEvent("Event1");
                ASSERT_EQ(m_deviceProtocolTracer->getProtocolTrace(), "[Directive1,Directive2,Event1]");
                m_deviceProtocolTracer->clearTracedMessages();
                ASSERT_EQ(m_deviceProtocolTracer->getProtocolTrace(), "[]");
            }
            TEST_F(DeviceProtocolTracerTest, test_maxTracedMessageLimit) {
                m_deviceProtocolTracer->setProtocolTraceFlag(true);
                m_deviceProtocolTracer->setMaxMessages(100);
                for (int i = 1; i <= 100; ++i) {
                    m_deviceProtocolTracer->receive("contextId", "Directive" + std::to_string(i));
                    m_deviceProtocolTracer->traceEvent("Event" + std::to_string(i));
                }
                auto messageListJsonString = m_deviceProtocolTracer->getProtocolTrace();
                for (int i = 1; i <= 50; ++i) {
                    ASSERT_TRUE(messageListJsonString.find("Directive" + std::to_string(i)) != std::string::npos);
                    ASSERT_TRUE(messageListJsonString.find("Event" + std::to_string(i)) != std::string::npos);
                }
                for (int i = 51; i <= 100; ++i) {
                    ASSERT_TRUE(messageListJsonString.find("Directive" + std::to_string(i)) == std::string::npos);
                    ASSERT_TRUE(messageListJsonString.find("Event" + std::to_string(i)) == std::string::npos);
                }
            }
            TEST_F(DeviceProtocolTracerTest, test_maxMessagesGettersSetters) {
                const unsigned int expectedVal = m_deviceProtocolTracer->getMaxMessages() + 1;
                ASSERT_TRUE(m_deviceProtocolTracer->setMaxMessages(expectedVal));
                ASSERT_EQ(expectedVal, m_deviceProtocolTracer->getMaxMessages());
            }
            TEST_F(DeviceProtocolTracerTest, test_setMaxMessagesFailsIfSmallerThanStoredMessages) {
                const unsigned int numMessages = 10;
                ASSERT_TRUE(m_deviceProtocolTracer->setMaxMessages(numMessages));
                m_deviceProtocolTracer->setProtocolTraceFlag(true);
                for (unsigned int i = 0; i < numMessages; ++i) {
                    m_deviceProtocolTracer->receive("contextId", "Directive" + std::to_string(i));
                    m_deviceProtocolTracer->traceEvent("Event" + std::to_string(i));
                }
                ASSERT_FALSE(m_deviceProtocolTracer->setMaxMessages(numMessages - 1));
                ASSERT_EQ(numMessages, m_deviceProtocolTracer->getMaxMessages());
            }
        }
    }
}