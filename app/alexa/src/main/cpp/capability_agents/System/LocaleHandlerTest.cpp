#include <gtest/gtest.h>
#include <memory>
#include <avs/attachment/MockAttachmentManager.h>
#include <sdkinterfaces/MockExceptionEncounteredSender.h>
#include <sdkinterfaces/MockLocaleAssetsManager.h>
#include <json/JSONGenerator.h>
#include <json/JSONUtils.h>
#include <util/WaitEvent.h>
#include <registration_manager/CustomerDataManager.h>
#include <settings/DeviceSettingsManager.h>
#include <settings/Settings/MockDeviceSettingStorage.h>
#include <settings/Settings/MockSettingEventSender.h>
#include <settings/Types/LocaleWakeWordsSetting.h>
#include "LocaleHandler.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace system {
            namespace test {
                using namespace attachment;
                using namespace json;
                using namespace storage;
                using namespace testing;
                using namespace types;
                using namespace sdkInterfaces::test;
                using namespace settings::test;
                using namespace storage::test;
                using namespace attachment::test;
                constexpr char NAMESPACE[] = "System";
                constexpr char MESSAGE_ID[] = "1";
                static const string LOCALES_PAYLOAD_KEY = "locales";
                static const set<string> TEST_LOCALES = {"en-US"};
                static const set<string> SUPPORTED_WAKE_WORDS = {"ALEXA", "ECHO"};
                static const set<string> SUPPORTED_LOCALES = {"en-CA", "en-US"};
                static const string DEFAULT_LOCALE = "en-CA";
                static const NamespaceAndName SET_WAKE_WORDS{NAMESPACE, "SetLocales"};
                class LocaleHandlerTest : public ::testing::Test {
                public:
                    void SetUp() override;
                    void TearDown() override;
                protected:
                    shared_ptr<LocaleHandler> m_localeHandler;
                    shared_ptr<DeviceSettingsManager> m_deviceSettingsManager;
                    shared_ptr<MockDeviceSettingStorage> m_mockDeviceSettingStorage;
                    shared_ptr<MockSettingEventSender> m_mockLocaleSettingMessageSender;
                    shared_ptr<MockSettingEventSender> m_mockWakeWordSettingMessageSender;
                    shared_ptr<MockLocaleAssetsManager> m_mockAssetsManager;
                    shared_ptr<MockExceptionEncounteredSender> m_mockExceptionEncounteredSender;
                    shared_ptr<LocaleWakeWordsSetting> m_localeSetting;
                };
                void LocaleHandlerTest::SetUp() {
                    auto customerDataManager = make_shared<registrationManager::CustomerDataManager>();
                    m_deviceSettingsManager = make_shared<settings::DeviceSettingsManager>(customerDataManager);
                    m_mockDeviceSettingStorage = make_shared<MockDeviceSettingStorage>();
                    m_mockExceptionEncounteredSender = make_shared<MockExceptionEncounteredSender>();
                    m_mockLocaleSettingMessageSender = make_shared<MockSettingEventSender>();
                    m_mockWakeWordSettingMessageSender = make_shared<NiceMock<MockSettingEventSender>>();
                    m_mockAssetsManager = make_shared<NiceMock<MockLocaleAssetsManager>>();
                    ON_CALL(*m_mockAssetsManager, getSupportedWakeWords(_))
                        .WillByDefault(InvokeWithoutArgs([]()->LocaleAssetsManagerInterface::WakeWordsSets {
                            return {SUPPORTED_WAKE_WORDS};
                        }));
                    ON_CALL(*m_mockAssetsManager, getDefaultSupportedWakeWords())
                        .WillByDefault(InvokeWithoutArgs([]()->LocaleAssetsManagerInterface::WakeWordsSets {
                            return {SUPPORTED_WAKE_WORDS};
                        }));
                    ON_CALL(*m_mockAssetsManager, getSupportedLocales()).WillByDefault(InvokeWithoutArgs([] {
                        return SUPPORTED_LOCALES;
                    }));
                    ON_CALL(*m_mockAssetsManager, getDefaultLocale()).WillByDefault(InvokeWithoutArgs([] { return DEFAULT_LOCALE; }));
                    //ON_CALL(*m_mockAssetsManager, changeAssets(_, _)).WillByDefault(InvokeWithoutArgs([] { return true; }));
                    EXPECT_CALL(*m_mockDeviceSettingStorage, loadSetting("System.locales"))
                        .WillOnce(Return(make_pair(SettingStatus::SYNCHRONIZED, R"(["en-CA"])")));
                    EXPECT_CALL(*m_mockDeviceSettingStorage, loadSetting("SpeechRecognizer.wakeWords"))
                        .WillOnce(Return(make_pair(SettingStatus::SYNCHRONIZED, "")));
                    auto settingSendEventSuccess = [](const string& value) {
                        promise<bool> retPromise;
                        retPromise.set_value(true);
                        return retPromise.get_future();
                    };
                    ON_CALL(*m_mockWakeWordSettingMessageSender, sendChangedEvent(_)).WillByDefault(Invoke(settingSendEventSuccess));
                    ON_CALL(*m_mockWakeWordSettingMessageSender, sendReportEvent(_)).WillByDefault(Invoke(settingSendEventSuccess));
                    ON_CALL(*m_mockLocaleSettingMessageSender, sendChangedEvent(_)).WillByDefault(Invoke(settingSendEventSuccess));
                    ON_CALL(*m_mockLocaleSettingMessageSender, sendReportEvent(_)).WillByDefault(Invoke(settingSendEventSuccess));
                    m_localeSetting = LocaleWakeWordsSetting::create(m_mockLocaleSettingMessageSender,m_mockWakeWordSettingMessageSender,
                                                                     m_mockDeviceSettingStorage,m_mockAssetsManager);
                    m_localeHandler = LocaleHandler::create(m_mockExceptionEncounteredSender, m_localeSetting);
                    ASSERT_NE(m_localeHandler, nullptr);
                }
                void LocaleHandlerTest::TearDown() {}
                TEST_F(LocaleHandlerTest, test_createWithInvalidArgs) {
                    ASSERT_EQ(LocaleHandler::create(nullptr, nullptr), nullptr);
                    ASSERT_EQ(LocaleHandler::create(m_mockExceptionEncounteredSender, nullptr), nullptr);
                    ASSERT_EQ(LocaleHandler::create(nullptr, m_localeSetting), nullptr);
                }
                TEST_F(LocaleHandlerTest, test_setLocalesDirectiveSuccess) {
                    JsonGenerator payloadGenerator;
                    payloadGenerator.addStringArray(LOCALES_PAYLOAD_KEY, TEST_LOCALES);
                    std::promise<bool> messagePromise;
                    EXPECT_CALL(*m_mockLocaleSettingMessageSender,sendReportEvent(jsonUtils::convertToJsonString(set<string>({*TEST_LOCALES.begin()}))))
                        .WillOnce(InvokeWithoutArgs([&messagePromise] {
                            messagePromise.set_value(true);
                            promise<bool> retPromise;
                            retPromise.set_value(true);
                            return retPromise.get_future();
                        }));
                    EXPECT_CALL(*m_mockDeviceSettingStorage, storeSettings(_)).WillRepeatedly(Return(true));
                    //EXPECT_CALL(*m_mockDeviceSettingStorage, updateSettingStatus(_, _)).WillRepeatedly(Return(true));
                    auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(SET_WAKE_WORDS.nameSpace, SET_WAKE_WORDS.name, MESSAGE_ID);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, payloadGenerator.toString(),
                                                                              attachmentManager, "");
                    auto capabilityAgent = static_pointer_cast<CapabilityAgent>(m_localeHandler);
                    capabilityAgent->handleDirectiveImmediately(directive);
                    auto messageFuture = messagePromise.get_future();
                    ASSERT_EQ(messageFuture.wait_for(seconds(5)),future_status::ready);
                }
            }
        }
    }
}