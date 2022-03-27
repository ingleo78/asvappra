#include <gtest/gtest.h>
#include <avs/attachment/MockAttachmentManager.h>
#include <sdkinterfaces/MockAVSConnectionManager.h>
#include <sdkinterfaces/MockDirectiveHandlerResult.h>
#include <sdkinterfaces/MockExceptionEncounteredSender.h>
#include <sdkinterfaces/MockMessageSender.h>
#include <sdkinterfaces/SystemTimeZoneInterface.h>
#include <json/JSONUtils.h>
#include <memory/Memory.h>
#include <util/WaitEvent.h>
#include <settings/DeviceSettingsManager.h>
#include <settings/Settings/MockSetting.h>
#include "TimeZoneHandler.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace system {
            namespace test {
                using namespace attachment;
                using namespace memory;
                using namespace testing;
                using namespace sdkInterfaces::test;
                using namespace settings::test;
                using namespace attachment::test;
                constexpr char NAMESPACE[] = "System";
                constexpr char SET_TIMEZONE_DIRECTIVE_NAME[] = "SetTimeZone";
                constexpr char MESSAGE_ID[] = "1";
                static const auto TEST_TIMEOUT = seconds(5);
                static const string TIMEZONE_NEW_YORK = "America/New_York";
                static const string TIMEZONE_DEFAULT = "Etc/GMT";
                static const string TIMEZONE_NEW_YORK_JSON = R"(")" + TIMEZONE_NEW_YORK + R"(")";
                static const string TIMEZONE_PAYLOAD_NEW_YORK =
                    "{"
                        R"("timeZone":)" + TIMEZONE_NEW_YORK_JSON +
                    "}";
                class TimeZoneHandlerTest : public Test {
                public:
                    void SetUp() override;
                    void TearDown() override;
                protected:
                    shared_ptr<MockSetting<TimeZoneSetting::ValueType>> m_mockTzSetting;
                    shared_ptr<MockExceptionEncounteredSender> m_mockExceptionEncounteredSender;
                    shared_ptr<TimeZoneHandler> m_timeZoneHandler;
                    unique_ptr<StrictMock<MockDirectiveHandlerResult>> m_mockDirectiveHandlerResult;
                };
                void TimeZoneHandlerTest::SetUp() {
                    m_mockExceptionEncounteredSender = make_shared<MockExceptionEncounteredSender>();
                    m_mockDirectiveHandlerResult = make_unique<StrictMock<MockDirectiveHandlerResult>>();
                    m_mockTzSetting = make_shared<MockSetting<TimeZoneSetting::ValueType>>(TIMEZONE_DEFAULT);
                    m_timeZoneHandler = TimeZoneHandler::create(m_mockTzSetting, m_mockExceptionEncounteredSender);
                    ASSERT_NE(m_timeZoneHandler, nullptr);
                }
                void TimeZoneHandlerTest::TearDown() {
                }
                TEST_F(TimeZoneHandlerTest, test_createWithoutTimezoneSetting) {
                    shared_ptr<TimeZoneHandler> nullHandler = TimeZoneHandler::create(nullptr, m_mockExceptionEncounteredSender);
                    ASSERT_EQ(nullHandler, nullptr);
                }
                TEST_F(TimeZoneHandlerTest, test_createWithoutExceptionSender) {
                    shared_ptr<TimeZoneHandler> nullHandler = TimeZoneHandler::create(m_mockTzSetting, nullptr);
                    ASSERT_EQ(nullHandler, nullptr);
                }
                TEST_F(TimeZoneHandlerTest, test_handleSetTimeZoneDirective) {
                    auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE, SET_TIMEZONE_DIRECTIVE_NAME, MESSAGE_ID);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, TIMEZONE_PAYLOAD_NEW_YORK, attachmentManager, "");
                    WaitEvent waitEvent;
                    EXPECT_CALL(*m_mockTzSetting, setAvsChange(TIMEZONE_NEW_YORK)).WillOnce(InvokeWithoutArgs([&waitEvent] {
                        waitEvent.wakeUp();
                        return true;
                    }));
                    auto timeZoneCA = static_pointer_cast<CapabilityAgent>(m_timeZoneHandler);
                    timeZoneCA->preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    timeZoneCA->handleDirective(MESSAGE_ID);
                    ASSERT_TRUE(waitEvent.wait(TEST_TIMEOUT));
                }
                TEST_F(TimeZoneHandlerTest, DISABLED_settingCallbackFails) {
                    auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE, SET_TIMEZONE_DIRECTIVE_NAME, MESSAGE_ID);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, TIMEZONE_PAYLOAD_NEW_YORK, attachmentManager, "");
                    avsCommon::utils::WaitEvent waitEvent;
                    EXPECT_CALL(*m_mockTzSetting, setAvsChange(TIMEZONE_NEW_YORK)).WillOnce(Return(false));
                    EXPECT_CALL(*m_mockExceptionEncounteredSender, sendExceptionEncountered(_, _, _))
                        .WillOnce(InvokeWithoutArgs([&waitEvent] { waitEvent.wakeUp(); }));
                    auto timeZoneCA = static_pointer_cast<CapabilityAgent>(m_timeZoneHandler);
                    timeZoneCA->preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    timeZoneCA->handleDirective(MESSAGE_ID);
                    ASSERT_TRUE(waitEvent.wait(TEST_TIMEOUT));
                }
            }
        }
    }
}