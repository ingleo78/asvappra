#include <mutex>
#include <thread>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <avs/attachment/MockAttachmentManager.h>
#include <avs/AVSMessageHeader.h>
#include <avs/SpeakerConstants/SpeakerConstants.h>
#include <sdkinterfaces/Audio/MockAlertsAudioFactory.h>
#include <sdkinterfaces/MockAVSConnectionManager.h>
#include <sdkinterfaces/MockContextManager.h>
#include <sdkinterfaces/MockDirectiveHandlerResult.h>
#include <sdkinterfaces/MockExceptionEncounteredSender.h>
#include <sdkinterfaces/MockFocusManager.h>
#include <sdkinterfaces/MockSpeakerManager.h>
#include <memory/Memory.h>
#include <metrics/MockMetricRecorder.h>
#include <util/WaitEvent.h>
#include <settings/DeviceSettingsManager.h>
#include <settings/Settings/MockSetting.h>
#include <settings/Types/AlarmVolumeRampTypes.h>
#include "AlertsCapabilityAgent.h"

namespace alexaClientSDK {
    namespace acsdkAlerts {
        namespace test {
            using namespace std;
            using namespace chrono;
            using namespace avsCommon;
            using namespace acsdkAlertsInterfaces;
            using namespace avs;
            using namespace speakerConstants;
            using namespace attachment;
            using namespace attachment::test;
            using namespace sdkInterfaces;
            using namespace audio;
            using namespace audio::test;
            using namespace sdkInterfaces::test;
            using namespace utils;
            using namespace memory;
            using namespace metrics;
            using namespace metrics::test;
            using namespace certifiedSender;
            using namespace registrationManager;
            using namespace renderer;
            using namespace storage;
            using namespace settings;
            using namespace settings::test;
            using namespace settings::types;
            using namespace testing;
            constexpr int MAX_WAIT_TIME_MS = 200;
            constexpr char SET_VOLUME_DIRECTIVE_NAME[] = "SetVolume";
            static constexpr char SET_ALARM_VOLUME_RAMP_DIRECTIVE_NAME[] = "SetAlarmVolumeRamp";
            static constexpr char ALERTS_NAMESPACE[] = "Alerts";
            constexpr char SET_VOLUME_NAMESPACE_NAME[] = "Alerts";
            constexpr char MESSAGE_ID[] = "1";
            constexpr int TEST_VOLUME_VALUE = 33;
            constexpr int HIGHER_VOLUME_VALUE = 100;
            constexpr int LOWER_VOLUME_VALUE = 50;
            static const auto TEST_TIMEOUT = seconds(5);
            static const string VOLUME_PAYLOAD = "{" R"("volume":)" + to_string(TEST_VOLUME_VALUE) + """}";
            static const string VOLUME_PAYLOAD_ABOVE_MAX = "{" R"("volume":)" + to_string(AVS_SET_VOLUME_MAX + 1) + """}";
            static const string VOLUME_PAYLOAD_BELOW_MIN = "{" R"("volume":)" + to_string(AVS_SET_VOLUME_MIN - 1) + """}";
            static const string ALARM_VOLUME_RAMP_PAYLOAD_ENABLED = R"({"alarmVolumeRamp":"ASCENDING"})";
            static const string ALARM_VOLUME_RAMP_PAYLOAD_INVALID = R"({"ascendingAlarm":"ASCENDING"})";
            static const string ALARM_VOLUME_RAMP_JSON_NAME = R"("name":"AlarmVolumeRampReport")";
            static const string ALARM_VOLUME_RAMP_JSON_VALUE = R"("ASCENDING")";
            class StubAlertStorage : public AlertStorageInterface {
            public:
                bool createDatabase() override {
                    return true;
                }
                bool open() override {
                    return true;
                }
                void close() override {
                }
                bool store(std::shared_ptr<Alert>) override {
                    return true;
                }
                bool load(std::vector<std::shared_ptr<Alert>>*, std::shared_ptr<settings::DeviceSettingsManager>) override {
                    return true;
                }
                bool modify(std::shared_ptr<Alert>) override {
                    return true;
                }
                bool erase(std::shared_ptr<Alert>) override {
                    return true;
                }
                bool clearDatabase() override {
                    return true;
                }
                bool bulkErase(const std::list<std::shared_ptr<Alert>>&) override {
                    return true;
                }
            };
            class MockAlertStorage : public AlertStorageInterface {
            public:
                MOCK_METHOD0(createDatabase, bool());
                MOCK_METHOD0(open, bool());
                MOCK_METHOD0(close, void());
                MOCK_METHOD1(store, bool(shared_ptr<Alert>));
                //MOCK_METHOD2(load, bool(vector<shared_ptr<Alert>>*, shared_ptr<DeviceSettingsManager>));
                MOCK_METHOD1(modify, bool(shared_ptr<Alert>));
                MOCK_METHOD1(erase, bool(shared_ptr<Alert>));
                MOCK_METHOD1(bulkErase, bool(const list<shared_ptr<Alert>>&));
                MOCK_METHOD0(clearDatabase, bool());
            };
            class StubRenderer : public RendererInterface {
                void start(shared_ptr<RendererObserverInterface> observer, function<pair<unique_ptr<istream>, const MediaType>()> audioFactory,
                           bool alarmVolumeRampEnabled, const vector<string>& urls, int loopCount, milliseconds loopPause, bool startWithPause) override {}
            };
            class MockRenderer : public RendererInterface {
            public:
                MOCK_METHOD7(start, void(shared_ptr<RendererObserverInterface> observer, function<std::pair<unique_ptr<istream>, const MediaType>()>, bool,
                             const vector<string>&, int, milliseconds, bool));
                MOCK_METHOD0(stop, void());
            };
            class StubMessageStorage : public MessageStorageInterface {
            public:
                bool createDatabase() override {
                    return true;
                }
                bool open() override {
                    return true;
                }
                void close() override {
                }
                bool store(const std::string& message, int* id) override {
                    return true;
                }
                bool store(const std::string& message, const std::string& uriPathExtension, int* id) override {
                    return true;
                }
                bool load(std::queue<StoredMessage>* messageContainer) override {
                    return true;
                }
                bool erase(int messageId) override {
                    return true;
                }

                bool clearDatabase() override {
                    return true;
                }
            };
            class TestMessageSender : public MessageSenderInterface {
            public:
                void sendMessage(shared_ptr<MessageRequest> request) override {
                    unique_lock<mutex> lock(m_mutex);
                    if (m_nextMessagePromise) {
                        m_nextMessagePromise->set_value(request);
                        m_nextMessagePromise.reset();
                    }
                    lock.unlock();
                    request->sendCompleted(MessageRequestObserverInterface::Status::SUCCESS);
                }
                future<shared_ptr<MessageRequest>> getNextMessage() {
                    lock_guard<mutex> lock(m_mutex);
                    m_nextMessagePromise = make_shared<promise<shared_ptr<MessageRequest>>>();
                    return m_nextMessagePromise->get_future();
                }
            private:
                shared_ptr<promise<shared_ptr<MessageRequest>>> m_nextMessagePromise;
                mutex m_mutex;
            };
            class AlertsCapabilityAgentTest : public Test {
            protected:
                void testStartAlertWithContentVolume(int8_t speakerVolume, int8_t alertsVolume, const string& otherChannel, bool shouldResultInSetVolume);
                void SetUp() override;
                void TearDown() override;
                shared_ptr<AlertsCapabilityAgent> m_alertsCA;
                shared_ptr<CertifiedSender> m_certifiedSender;
                shared_ptr<TestMessageSender> m_mockMessageSender;
                shared_ptr<StubMessageStorage> m_messageStorage;
                shared_ptr<MockAVSConnectionManager> m_mockAVSConnectionManager;
                shared_ptr<MockFocusManager> m_mockFocusManager;
                shared_ptr<MockSpeakerManager> m_speakerManager;
                shared_ptr<MockExceptionEncounteredSender> m_exceptionSender;
                shared_ptr<MockContextManager> m_contextManager;
                shared_ptr<MockAlertStorage> m_alertStorage;
                shared_ptr<MockAlertsAudioFactory> m_alertsAudioFactory;
                shared_ptr<MockRenderer> m_renderer;
                shared_ptr<MockSetting<AlarmVolumeRampSetting::ValueType>> m_mockAlarmVolumeRampSetting;
                shared_ptr<CustomerDataManager> m_customerDataManager;
                unique_ptr<StrictMock<MockDirectiveHandlerResult>> m_mockDirectiveHandlerResult;
                shared_ptr<DeviceSettingsManager> m_settingsManager;
                shared_ptr<MetricRecorderInterface> m_metricRecorder;
                mutex m_mutex;
            };
            void AlertsCapabilityAgentTest::SetUp() {
                m_metricRecorder = make_shared<NiceMock<MockMetricRecorder>>();
                m_mockMessageSender = make_shared<TestMessageSender>();
                m_mockAVSConnectionManager = make_shared<NiceMock<MockAVSConnectionManager>>();
                m_mockFocusManager = make_shared<NiceMock<MockFocusManager>>();
                m_speakerManager = make_shared<NiceMock<MockSpeakerManager>>();
                m_exceptionSender = make_shared<NiceMock<MockExceptionEncounteredSender>>();
                m_contextManager = make_shared<NiceMock<MockContextManager>>();
                //m_alertStorage = make_shared<NiceMock<MockAlertStorage>>();
                m_alertsAudioFactory = make_shared<NiceMock<MockAlertsAudioFactory>>();
                m_renderer = make_shared<NiceMock<MockRenderer>>();
                m_customerDataManager = make_shared<CustomerDataManager>();
                m_messageStorage = make_shared<StubMessageStorage>();
                m_mockDirectiveHandlerResult = make_unique<StrictMock<MockDirectiveHandlerResult>>();
                m_settingsManager = make_shared<DeviceSettingsManager>(make_shared<CustomerDataManager>());
                m_mockAlarmVolumeRampSetting = make_shared<MockSetting<AlarmVolumeRampSetting::ValueType>>(getAlarmVolumeRampDefault());
                ASSERT_TRUE(m_settingsManager->addSetting<DeviceSettingsIndex::ALARM_VOLUME_RAMP>(m_mockAlarmVolumeRampSetting));
                ON_CALL(*(m_speakerManager.get()), getSpeakerSettings(_, _)).WillByDefault(Invoke([](ChannelVolumeInterface::Type,
                        SpeakerInterface::SpeakerSettings*) {
                            promise<bool> promise;
                            promise.set_value(true);
                            return promise.get_future();
                        }));
                ON_CALL(*(m_speakerManager.get()), setVolume(_, _, _)).WillByDefault(Invoke([](ChannelVolumeInterface::Type, int8_t,
                        const SpeakerManagerInterface::NotificationProperties&) {
                            std::promise<bool> promise;
                            promise.set_value(true);
                            return promise.get_future();
                        }));
                ON_CALL(*(m_alertStorage), createDatabase()).WillByDefault(Return(true));
                ON_CALL(*(m_alertStorage), open()).WillByDefault(Return(true));
                ON_CALL(*(m_alertStorage), store(_)).WillByDefault(Return(true));
                //ON_CALL(*(m_alertStorage), load(_, _)).WillByDefault(Return(true));
                ON_CALL(*(m_alertStorage), modify(_)).WillByDefault(Return(true));
                ON_CALL(*(m_alertStorage), erase(_)).WillByDefault(Return(true));
                ON_CALL(*(m_alertStorage), bulkErase(_)).WillByDefault(Return(true));
                ON_CALL(*(m_alertStorage), clearDatabase()).WillByDefault(Return(true));
                m_certifiedSender = CertifiedSender::create(m_mockMessageSender,m_mockAVSConnectionManager,m_messageStorage,m_customerDataManager);
                m_alertsCA = AlertsCapabilityAgent::create(m_mockMessageSender,m_mockAVSConnectionManager,m_certifiedSender,
                                               m_mockFocusManager, m_speakerManager, m_contextManager,
                                     m_exceptionSender, m_alertStorage, m_alertsAudioFactory,
                                                m_renderer, m_customerDataManager, m_mockAlarmVolumeRampSetting,
                                              m_settingsManager, m_metricRecorder);
                std::static_pointer_cast<ConnectionStatusObserverInterface>(m_certifiedSender)->onConnectionStatusChanged(ConnectionStatusObserverInterface::Status::CONNECTED,
                        ConnectionStatusObserverInterface::ChangedReason::ACL_CLIENT_REQUEST);
            }
            void AlertsCapabilityAgentTest::TearDown() {
                m_certifiedSender->shutdown();
                m_certifiedSender.reset();
                m_alertsCA->shutdown();
                m_alertsCA.reset();
                m_mockMessageSender.reset();
                m_mockAVSConnectionManager.reset();
                m_mockFocusManager.reset();
                m_speakerManager.reset();
                m_exceptionSender.reset();
                m_contextManager.reset();
                m_alertStorage.reset();
                m_alertsAudioFactory.reset();
                m_renderer.reset();
                m_customerDataManager.reset();
                m_messageStorage.reset();
            }
            void AlertsCapabilityAgentTest::testStartAlertWithContentVolume(
                int8_t speakerVolume,
                int8_t alertsVolume,
                const string& otherChannel,
                bool shouldResultInSetVolume) {
                ON_CALL(*(m_speakerManager.get()), getSpeakerSettings(_, _))
                    .WillByDefault(Invoke([speakerVolume, alertsVolume](ChannelVolumeInterface::Type type, SpeakerInterface::SpeakerSettings* settings) {
                                       if (type == ChannelVolumeInterface::Type::AVS_SPEAKER_VOLUME) settings->volume = speakerVolume;
                                       else settings->volume = alertsVolume;
                                       settings->mute = false;
                                       promise<bool> promise;
                                       promise.set_value(true);
                                       return promise.get_future();
                                   }));
                condition_variable waitCV;
                ON_CALL(*(m_speakerManager.get()), setVolume(_, _, _))
                    .WillByDefault(Invoke([&waitCV](ChannelVolumeInterface::Type, int8_t, const SpeakerManagerInterface::NotificationProperties&) {
                                       waitCV.notify_all();
                                       promise<bool> promise;
                                       promise.set_value(true);
                                       return promise.get_future();
                                   }));
                EXPECT_CALL(*(m_speakerManager.get()), setVolume(ChannelVolumeInterface::Type::AVS_ALERTS_VOLUME, _, _))
                    .Times(shouldResultInSetVolume ? 1 : 0);
                SpeakerInterface::SpeakerSettings speakerSettings{speakerVolume, false};
                speakerSettings.mute = false;
                m_alertsCA->onSpeakerSettingsChanged(SpeakerManagerObserverInterface::Source::LOCAL_API,ChannelVolumeInterface::Type::AVS_SPEAKER_VOLUME,
                                                     speakerSettings);
                speakerSettings.volume = alertsVolume;
                m_alertsCA->onSpeakerSettingsChanged(SpeakerManagerObserverInterface::Source::LOCAL_API, ChannelVolumeInterface::Type::AVS_ALERTS_VOLUME,
                                                     speakerSettings);
                m_alertsCA->onFocusChanged(otherChannel, FocusState::BACKGROUND);
                m_alertsCA->onAlertStateChange("", "", AlertObserverInterface::State::STARTED, "");
                unique_lock<std::mutex> ulock(m_mutex);
                waitCV.wait_for(ulock, milliseconds(MAX_WAIT_TIME_MS));
            }
            TEST_F(AlertsCapabilityAgentTest, test_localAlertVolumeChangeNoAlert) {
                SpeakerInterface::SpeakerSettings speakerSettings;
                speakerSettings.volume = TEST_VOLUME_VALUE;
                speakerSettings.mute = false;
                m_alertsCA->onSpeakerSettingsChanged(SpeakerManagerObserverInterface::Source::LOCAL_API, ChannelVolumeInterface::Type::AVS_ALERTS_VOLUME,
                                                     speakerSettings);
                auto future = m_mockMessageSender->getNextMessage();
                ASSERT_EQ(future.wait_for(milliseconds(MAX_WAIT_TIME_MS)), future_status::ready);
                string content = future.get()->getJsonContent();
                ASSERT_TRUE(content.find("\"name\":\"VolumeChanged\"") != string::npos);
            }
            TEST_F(AlertsCapabilityAgentTest, testTimer_localAlertVolumeChangeAlertPlaying) {
                m_alertsCA->onAlertStateChange("", "", AlertObserverInterface::State::STARTED, "");
                auto future = m_mockMessageSender->getNextMessage();
                ASSERT_EQ(future.wait_for(std::chrono::milliseconds(MAX_WAIT_TIME_MS)), future_status::ready);
                string content = future.get()->getJsonContent();
                ASSERT_TRUE(content.find("\"name\":\"AlertStarted\"") != string::npos);
                SpeakerInterface::SpeakerSettings speakerSettings;
                speakerSettings.volume = TEST_VOLUME_VALUE;
                m_alertsCA->onSpeakerSettingsChanged(SpeakerManagerObserverInterface::Source::LOCAL_API,ChannelVolumeInterface::Type::AVS_ALERTS_VOLUME,
                                                     speakerSettings);
                ASSERT_EQ(m_mockMessageSender->getNextMessage().wait_for(milliseconds(MAX_WAIT_TIME_MS)),future_status::timeout);
            }
            TEST_F(AlertsCapabilityAgentTest, test_avsAlertVolumeChangeNoAlert) {
                auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                auto avsMessageHeader = make_shared<AVSMessageHeader>(SET_VOLUME_NAMESPACE_NAME, SET_VOLUME_DIRECTIVE_NAME, MESSAGE_ID);
                shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, VOLUME_PAYLOAD,attachmentManager,"");
                EXPECT_CALL(*(m_speakerManager.get()), setVolume(ChannelVolumeInterface::Type::AVS_ALERTS_VOLUME, TEST_VOLUME_VALUE, _)).Times(1);
                static_pointer_cast<CapabilityAgent>(m_alertsCA)->preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                static_pointer_cast<CapabilityAgent>(m_alertsCA)->handleDirective(MESSAGE_ID);
                auto future = m_mockMessageSender->getNextMessage();
                ASSERT_EQ(future.wait_for(milliseconds(MAX_WAIT_TIME_MS)), future_status::ready);
                string content = future.get()->getJsonContent();
                ASSERT_TRUE(content.find("\"name\":\"VolumeChanged\"") != std::string::npos);
            }
            TEST_F(AlertsCapabilityAgentTest, test_avsAlertVolumeChangeAlertPlaying) {
                auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                auto avsMessageHeader = make_shared<AVSMessageHeader>(SET_VOLUME_NAMESPACE_NAME, SET_VOLUME_DIRECTIVE_NAME, MESSAGE_ID);
                shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, VOLUME_PAYLOAD, attachmentManager, "");
                EXPECT_CALL(*(m_speakerManager.get()), setVolume(ChannelVolumeInterface::Type::AVS_ALERTS_VOLUME, TEST_VOLUME_VALUE, _)).Times(1);
                m_alertsCA->onAlertStateChange("", "", AlertObserverInterface::State::STARTED, "");
                auto future = m_mockMessageSender->getNextMessage();
                ASSERT_EQ(future.wait_for(std::chrono::milliseconds(MAX_WAIT_TIME_MS)), future_status::ready);
                string content = future.get()->getJsonContent();
                ASSERT_TRUE(content.find("\"name\":\"AlertStarted\"") != std::string::npos);
                static_pointer_cast<CapabilityAgent>(m_alertsCA)->preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
                static_pointer_cast<CapabilityAgent>(m_alertsCA)->handleDirective(MESSAGE_ID);
                future = m_mockMessageSender->getNextMessage();
                ASSERT_EQ(future.wait_for(std::chrono::milliseconds(MAX_WAIT_TIME_MS)), std::future_status::ready);
                content = future.get()->getJsonContent();
                ASSERT_TRUE(content.find("\"name\":\"VolumeChanged\"") != std::string::npos);
            }
            TEST_F(AlertsCapabilityAgentTest, test_startAlertWithContentChannelLowerVolume) {
                testStartAlertWithContentVolume(LOWER_VOLUME_VALUE, HIGHER_VOLUME_VALUE, FocusManagerInterface::CONTENT_CHANNEL_NAME, false);
            }
            TEST_F(AlertsCapabilityAgentTest, test_startAlertWithContentChannelHigherVolume) {
                testStartAlertWithContentVolume(HIGHER_VOLUME_VALUE, LOWER_VOLUME_VALUE, FocusManagerInterface::CONTENT_CHANNEL_NAME, true);
            }
            TEST_F(AlertsCapabilityAgentTest, test_startAlertWithCommsChannelLowerVolume) {
                testStartAlertWithContentVolume(LOWER_VOLUME_VALUE, HIGHER_VOLUME_VALUE, FocusManagerInterface::COMMUNICATIONS_CHANNEL_NAME, false);
            }
            TEST_F(AlertsCapabilityAgentTest, test_startAlertWithCommsChannelHigherVolume) {
                testStartAlertWithContentVolume(HIGHER_VOLUME_VALUE, LOWER_VOLUME_VALUE, FocusManagerInterface::COMMUNICATIONS_CHANNEL_NAME, true);
            }
            TEST_F(AlertsCapabilityAgentTest, test_startAlertWithDialogChannelLowerVolume) {
                testStartAlertWithContentVolume(LOWER_VOLUME_VALUE, HIGHER_VOLUME_VALUE, FocusManagerInterface::DIALOG_CHANNEL_NAME, false);
            }
            TEST_F(AlertsCapabilityAgentTest, test_startAlertWithDialogChannelHigherVolume) {
                testStartAlertWithContentVolume(HIGHER_VOLUME_VALUE, LOWER_VOLUME_VALUE, FocusManagerInterface::DIALOG_CHANNEL_NAME, false);
            }
            TEST_F(AlertsCapabilityAgentTest, test_invalidVolumeValuesMax) {
                EXPECT_CALL(*(m_speakerManager.get()), setVolume(ChannelVolumeInterface::Type::AVS_ALERTS_VOLUME, AVS_SET_VOLUME_MAX, _)).Times(1);
                condition_variable waitCV;
                ON_CALL(*(m_speakerManager.get()), setVolume(ChannelVolumeInterface::Type::AVS_ALERTS_VOLUME, AVS_SET_VOLUME_MAX, _))
                    .WillByDefault(Invoke([&waitCV](ChannelVolumeInterface::Type, int8_t, const SpeakerManagerInterface::NotificationProperties&) {
                                       waitCV.notify_all();
                                       promise<bool> promise;
                                       promise.set_value(true);
                                       return promise.get_future();
                                   }));
                auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                auto avsMessageHeader = make_shared<AVSMessageHeader>(SET_VOLUME_NAMESPACE_NAME, SET_VOLUME_DIRECTIVE_NAME, MESSAGE_ID);
                shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, VOLUME_PAYLOAD_ABOVE_MAX, attachmentManager, "");
                static_pointer_cast<CapabilityAgent>(m_alertsCA)->preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                static_pointer_cast<CapabilityAgent>(m_alertsCA)->handleDirective(MESSAGE_ID);
                unique_lock<std::mutex> ulock(m_mutex);
                waitCV.wait_for(ulock, milliseconds(MAX_WAIT_TIME_MS));
            }
            TEST_F(AlertsCapabilityAgentTest, test_invalidVolumeValuesMin) {
                EXPECT_CALL(*(m_speakerManager.get()), setVolume(ChannelVolumeInterface::Type::AVS_ALERTS_VOLUME, AVS_SET_VOLUME_MIN, _)).Times(1);
                std::condition_variable waitCV;
                ON_CALL(*(m_speakerManager.get()), setVolume(ChannelVolumeInterface::Type::AVS_ALERTS_VOLUME, AVS_SET_VOLUME_MIN, _))
                    .WillByDefault(Invoke([&waitCV](ChannelVolumeInterface::Type, int8_t, const SpeakerManagerInterface::NotificationProperties&) {
                                              waitCV.notify_all();
                                              promise<bool> promise;
                                              promise.set_value(true);
                                              return promise.get_future();
                                          }));
                auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                auto avsMessageHeader = make_shared<AVSMessageHeader>(SET_VOLUME_NAMESPACE_NAME, SET_VOLUME_DIRECTIVE_NAME, MESSAGE_ID);
                shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, VOLUME_PAYLOAD_BELOW_MIN, attachmentManager, "");
                static_pointer_cast<CapabilityAgent>(m_alertsCA)->preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                static_pointer_cast<CapabilityAgent>(m_alertsCA)->handleDirective(MESSAGE_ID);
                unique_lock<std::mutex> ulock(m_mutex);
                waitCV.wait_for(ulock, std::chrono::milliseconds(MAX_WAIT_TIME_MS));
            }
            TEST_F(AlertsCapabilityAgentTest, test_SetAlarmVolumeRampDirective) {
                auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                auto avsMessageHeader = make_shared<AVSMessageHeader>(ALERTS_NAMESPACE, SET_ALARM_VOLUME_RAMP_DIRECTIVE_NAME, MESSAGE_ID);
                shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, ALARM_VOLUME_RAMP_PAYLOAD_ENABLED, attachmentManager, "");
                WaitEvent waitEvent;
                EXPECT_CALL(*m_mockAlarmVolumeRampSetting, setAvsChange(types::AlarmVolumeRampTypes::ASCENDING))
                    .WillOnce(InvokeWithoutArgs([&waitEvent] {
                                  waitEvent.wakeUp();
                                  return true;
                              }));
                auto alertsCA = std::static_pointer_cast<CapabilityAgent>(m_alertsCA);
                alertsCA->preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
                alertsCA->handleDirective(MESSAGE_ID);
                ASSERT_TRUE(waitEvent.wait(TEST_TIMEOUT));
            }
            TEST_F(AlertsCapabilityAgentTest, test_SetAlarmVolumeRampDirectiveInvalid) {
                auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                auto avsMessageHeader = make_shared<AVSMessageHeader>(ALERTS_NAMESPACE, SET_ALARM_VOLUME_RAMP_DIRECTIVE_NAME, MESSAGE_ID);
                shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, ALARM_VOLUME_RAMP_PAYLOAD_INVALID, attachmentManager, "");
                WaitEvent waitEvent;
                EXPECT_CALL(*m_exceptionSender, sendExceptionEncountered(_, _, _)).WillOnce(InvokeWithoutArgs([&waitEvent] { waitEvent.wakeUp(); }));
                auto alertsCA = std::static_pointer_cast<CapabilityAgent>(m_alertsCA);
                alertsCA->preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                alertsCA->handleDirective(MESSAGE_ID);
                EXPECT_TRUE(waitEvent.wait(TEST_TIMEOUT));
            }
        }
    }
}