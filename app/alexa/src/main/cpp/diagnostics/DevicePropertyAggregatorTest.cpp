#include <future>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <avs/IndicatorState.h>
#include <sdkinterfaces/MockContextManager.h>
#include <sdkinterfaces/MockSpeakerManager.h>
#include <acsdk_alerts_interfaces/AlertObserverInterface.h>
#include "DevicePropertyAggregator.h"

namespace alexaClientSDK {
    namespace diagnostics {
        namespace test {
            using namespace acsdkAlertsInterfaces;
            using namespace testing;
            using namespace sdkInterfaces::test;
            static const string DEFAULT_ALERT_STATE = "IDLE";
            static const string DEFAULT_CONTENT_ID = "NONE";
            static const string DEFAULT_AUDIO_PLAYER_STATE = "IDLE";
            static const string DEVICE_CONTEXT_VALUE = "TEST_DEVICE_CONTEXT";
            static const ContextRequestToken MOCK_CONTEXT_REQUEST_TOKEN = 1;
            class DevicePropertyAggregatorTest : public Test {
            public:
                void SetUp() override;
                void TearDown() override;
                bool validatePropertyValue(const string& propertyKey, const string& expectedValue);
                shared_ptr<DevicePropertyAggregator> m_devicePropertyAggregator;
                SpeakerSettings m_avsVolumeSetting;
                SpeakerSettings m_alertsVolumeSetting;
                shared_ptr<MockContextManager> m_mockContextManager;
                shared_ptr<MockSpeakerManager> m_mockSpeakerManager;
                thread m_mockContextMangerThread;
            };
            void DevicePropertyAggregatorTest::SetUp() {
                m_mockContextManager = make_shared<MockContextManager>();
                m_mockSpeakerManager = make_shared<MockSpeakerManager>();
                m_avsVolumeSetting.volume = 10;
                m_avsVolumeSetting.mute = false;
                m_alertsVolumeSetting.volume = 15;
                m_alertsVolumeSetting.mute = true;
                /*EXPECT_CALL(*m_mockSpeakerManager, getSpeakerSettings(Type::AVS_SPEAKER_VOLUME, _))
                    .WillOnce(Invoke([this](Type type, SpeakerSettings* settings) {
                        settings->volume = m_avsVolumeSetting.volume;
                        settings->mute = m_avsVolumeSetting.mute;
                        promise<bool> result;
                        result.set_value(true);
                        return result.get_future();
                    }));
                EXPECT_CALL(*m_mockSpeakerManager, getSpeakerSettings(ChannelVolumeInterface::Type::AVS_ALERTS_VOLUME, _))
                    .WillOnce(Invoke([this](Type type, SpeakerSettings* settings) {
                        settings->volume = m_alertsVolumeSetting.volume;
                        settings->mute = m_alertsVolumeSetting.mute;
                        promise<bool> result;
                        result.set_value(true);
                        return result.get_future();
                    }));*/
                m_devicePropertyAggregator = DevicePropertyAggregator::create();
                m_devicePropertyAggregator->setContextManager(m_mockContextManager);
                m_devicePropertyAggregator->initializeVolume(m_mockSpeakerManager);
            }
            void DevicePropertyAggregatorTest::TearDown() {
                if (m_mockContextMangerThread.joinable()) m_mockContextMangerThread.join();
                m_mockContextManager.reset();
            }
            bool DevicePropertyAggregatorTest::validatePropertyValue(const string& propertyKey, const string& expectedValue) {
                auto maybePropertyValue = m_devicePropertyAggregator->getDeviceProperty(propertyKey);
                if (maybePropertyValue.hasValue()) return (maybePropertyValue.value() == expectedValue);
                else return false;
            }
            TEST_F(DevicePropertyAggregatorTest, test_noContextManager_EmptyOptional) {
                shared_ptr<DevicePropertyAggregator> devicePropertyAggregator = DevicePropertyAggregator::create();
                ASSERT_FALSE(devicePropertyAggregator->getDeviceProperty(DevicePropertyAggregator::DEVICE_CONTEXT).hasValue());
            }
            TEST_F(DevicePropertyAggregatorTest, test_intializePropertyMap) {
                ASSERT_TRUE(validatePropertyValue(DevicePropertyAggregator::AVS_SPEAKER_VOLUME, "10"));
                ASSERT_TRUE(validatePropertyValue(DevicePropertyAggregator::AVS_SPEAKER_MUTE, "false"));
                ASSERT_TRUE(validatePropertyValue(DevicePropertyAggregator::AVS_ALERTS_VOLUME, "15"));
                ASSERT_TRUE(validatePropertyValue(DevicePropertyAggregator::AVS_ALERTS_MUTE, "true"));
                ASSERT_TRUE(validatePropertyValue(DevicePropertyAggregator::AUDIO_PLAYER_STATE, DEFAULT_AUDIO_PLAYER_STATE));
                ASSERT_TRUE(validatePropertyValue(DevicePropertyAggregator::CONTENT_ID, DEFAULT_CONTENT_ID));
                ASSERT_TRUE(validatePropertyValue(DevicePropertyAggregator::ALERT_TYPE_AND_STATE, DEFAULT_ALERT_STATE));
            }
            TEST_F(DevicePropertyAggregatorTest, test_getContextFailure_EmptyOptional) {
                auto contextFailureLambda = [this](shared_ptr<ContextRequesterInterface> contextRequester, const string& endpointId,
                                                   const milliseconds& timeout) {
                    if (m_mockContextMangerThread.joinable()) m_mockContextMangerThread.join();
                    m_mockContextMangerThread = thread(
                        [contextRequester]() { contextRequester->onContextFailure(ContextRequestError::BUILD_CONTEXT_ERROR); });
                    return MOCK_CONTEXT_REQUEST_TOKEN;
                };
                EXPECT_CALL(*m_mockContextManager, getContext(_, _, _)).WillOnce(Invoke(contextFailureLambda));
                ASSERT_FALSE(m_devicePropertyAggregator->getDeviceProperty(DevicePropertyAggregator::DEVICE_CONTEXT).hasValue());
            }
            TEST_F(DevicePropertyAggregatorTest, test_getContextSuccessful) {
                auto contextSuccessLambda = [this](
                                                shared_ptr<ContextRequesterInterface> contextRequester,
                                                const string& endpointId,
                                                const milliseconds& timeout) {
                    if (m_mockContextMangerThread.joinable()) m_mockContextMangerThread.join();
                    m_mockContextMangerThread = thread([contextRequester]() { contextRequester->onContextAvailable(DEVICE_CONTEXT_VALUE); });
                    return MOCK_CONTEXT_REQUEST_TOKEN;
                };
                EXPECT_CALL(*m_mockContextManager, getContext(_, _, _)).WillOnce(Invoke(contextSuccessLambda));
                ASSERT_EQ(m_devicePropertyAggregator->getDeviceProperty(DevicePropertyAggregator::DEVICE_CONTEXT), DEVICE_CONTEXT_VALUE);
            }
            TEST_F(DevicePropertyAggregatorTest, test_getDevicePropertyForInvalidKey_EmptyOptional) {
                auto maybePropertyValue = m_devicePropertyAggregator->getDeviceProperty("TEST_KEY");
                ASSERT_FALSE(maybePropertyValue.hasValue());
            }
            TEST_F(DevicePropertyAggregatorTest, test_getSpeakerSettingsPropertyForNonInitializedVolume_EmptyString) {
                shared_ptr<DevicePropertyAggregator> devicePropertyAggregator = DevicePropertyAggregator::create();

                ASSERT_FALSE(validatePropertyValue(DevicePropertyAggregator::AVS_SPEAKER_VOLUME, ""));
                ASSERT_FALSE(validatePropertyValue(DevicePropertyAggregator::AVS_SPEAKER_MUTE, ""));
                ASSERT_FALSE(validatePropertyValue(DevicePropertyAggregator::AVS_ALERTS_VOLUME, ""));
                ASSERT_FALSE(validatePropertyValue(DevicePropertyAggregator::AVS_ALERTS_MUTE, ""));
            }
            TEST_F(DevicePropertyAggregatorTest, test_getSpeakerSettingsProperty) {
                SpeakerInterface::SpeakerSettings avsVolumeSettings, alertsVolumeSettings;
                avsVolumeSettings.mute = true;
                avsVolumeSettings.volume = 50;
                alertsVolumeSettings.mute = false;
                alertsVolumeSettings.volume = 80;
                m_devicePropertyAggregator->onSpeakerSettingsChanged(Source::LOCAL_API, Type::AVS_SPEAKER_VOLUME, avsVolumeSettings);
                m_devicePropertyAggregator->onSpeakerSettingsChanged(Source::LOCAL_API, Type::AVS_ALERTS_VOLUME, alertsVolumeSettings);
                ASSERT_TRUE(validatePropertyValue(DevicePropertyAggregator::AVS_SPEAKER_VOLUME, "50"));
                ASSERT_TRUE(validatePropertyValue(DevicePropertyAggregator::AVS_SPEAKER_MUTE, "true"));
                ASSERT_TRUE(validatePropertyValue(DevicePropertyAggregator::AVS_ALERTS_VOLUME, "80"));
                ASSERT_TRUE(validatePropertyValue(DevicePropertyAggregator::AVS_ALERTS_MUTE, "false"));
            }
            TEST_F(DevicePropertyAggregatorTest, test_getConnectionStatusProperty) {
                m_devicePropertyAggregator->onConnectionStatusChanged(Status::PENDING,ChangedReason::SUCCESS);
                ASSERT_TRUE(validatePropertyValue(DevicePropertyAggregator::CONNECTION_STATE, "PENDING"));
            }
            TEST_F(DevicePropertyAggregatorTest, test_getNotificationStatusProperty) {
                m_devicePropertyAggregator->onSetIndicator(IndicatorState::ON);
                ASSERT_TRUE(validatePropertyValue(DevicePropertyAggregator::NOTIFICATION_INDICATOR, "ON"));
            }
            TEST_F(DevicePropertyAggregatorTest, test_getAudioPlayerStatusProperty) {
                acsdkAudioPlayerInterfaces::AudioPlayerObserverInterface::Context context;
                context.audioItemId = "abcd";
                context.offset = seconds(5);
                m_devicePropertyAggregator->onPlayerActivityChanged(PlayerActivity::PLAYING, context);
                ASSERT_TRUE(validatePropertyValue(DevicePropertyAggregator::AUDIO_PLAYER_STATE, "PLAYING"));
                ASSERT_TRUE(validatePropertyValue(DevicePropertyAggregator::CONTENT_ID, context.audioItemId));
            }
            TEST_F(DevicePropertyAggregatorTest, test_getTTSPlayerStateProperty) {
                m_devicePropertyAggregator->onDialogUXStateChanged(DialogUXState::LISTENING);

                ASSERT_TRUE(validatePropertyValue(DevicePropertyAggregator::TTS_PLAYER_STATE, "LISTENING"));
            }
            TEST_F(DevicePropertyAggregatorTest, test_getAlarmStatusProperty) {
                m_devicePropertyAggregator->onAlertStateChange("TEST_TOKEN", "TEST_ALERT_TYPE", State::STARTED,
                                                               "TEST_ALERT_REASON");
                ASSERT_TRUE(validatePropertyValue(DevicePropertyAggregator::ALERT_TYPE_AND_STATE, "TEST_ALERT_TYPE:STARTED"));
            }
        }
    }
}