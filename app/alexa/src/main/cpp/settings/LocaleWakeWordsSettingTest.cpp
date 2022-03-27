#include <condition_variable>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <tuple>
#include <vector>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sdkinterfaces/MockLocaleAssetsManager.h>
#include <json/JSONUtils.h>
#include <util/WaitEvent.h>
#include "Types/LocaleWakeWordsSetting.h"
#include "Settings/MockDeviceSettingStorage.h"
#include "Settings/MockSettingEventSender.h"
#include "Settings/MockSettingObserver.h"
#include "SettingEventMetadata.h"
#include "SettingObserverInterface.h"
#include "SettingStringConversion.h"

namespace alexaClientSDK {
    namespace settings {
        namespace test {
            using namespace std;
            using namespace chrono;
            using namespace avsCommon;
            using namespace sdkInterfaces;
            using namespace sdkInterfaces::test;
            using namespace utils;
            using namespace json;
            using namespace storage;
            using namespace storage::test;
            using namespace testing;
            using namespace types;
            static const string ALEXA_VALUE = "ALEXA";
            static const string ECHO_VALUE = "ECHO";
            static const string ENGLISH_CANADA_VALUE = "en-CA";
            static const string FRENCH_CANADA_VALUE = "fr-CA";
            static const string INVALID_VALUE = "INVALID";
            static const seconds MY_WAIT_TIMEOUT{5};
            static const MockLocaleAssetsManager::LocaleAssetsManagerInterface::WakeWordsSets SUPPORTED_WAKE_WORDS_COMBINATION{
                {ALEXA_VALUE},
                {ECHO_VALUE},
                {ALEXA_VALUE, ECHO_VALUE}
            };
            static const set<string> SUPPORTED_LOCALES{ENGLISH_CANADA_VALUE, FRENCH_CANADA_VALUE};
            static const vector<string> ENGLISH_LOCALES{ENGLISH_CANADA_VALUE};
            static const string WAKE_WORDS_KEY = "SpeechRecognizer.wakeWords";
            static const string LOCALES_KEY = "System.locales";
            class LocaleWakeWordsSettingTest : public Test {
            protected:
                void SetUp() override;
                string toJson(set<string> values) const;
                void initializeSetting(const LocalesSetting::ValueType& locale, const WakeWordsSetting::ValueType wakeWords);
                shared_ptr<MockSettingEventSender> m_wakeWordsSenderMock;
                shared_ptr<MockSettingEventSender> m_localesSenderMock;
                shared_ptr<MockDeviceSettingStorage> m_storageMock;
                shared_ptr<MockLocaleAssetsManager> m_assetsManagerMock;
                shared_ptr<MockSettingObserver<LocalesSetting>> m_localeObserver;
                shared_ptr<MockSettingObserver<WakeWordsSetting>> m_wakeWordsObserver;
                shared_ptr<LocaleWakeWordsSetting> m_setting;
            };
            void LocaleWakeWordsSettingTest::SetUp() {
                m_wakeWordsSenderMock = std::make_shared<StrictMock<MockSettingEventSender>>();
                m_localesSenderMock = std::make_shared<StrictMock<MockSettingEventSender>>();
                m_storageMock = std::make_shared<StrictMock<MockDeviceSettingStorage>>();
                m_assetsManagerMock = std::make_shared<StrictMock<MockLocaleAssetsManager>>();
                m_localeObserver = std::make_shared<MockSettingObserver<LocalesSetting>>();
                m_wakeWordsObserver = std::make_shared<MockSettingObserver<WakeWordsSetting>>();
                EXPECT_CALL(*m_assetsManagerMock, getDefaultLocale()).WillRepeatedly(Return(ENGLISH_CANADA_VALUE));
                EXPECT_CALL(*m_assetsManagerMock, getSupportedLocales()).WillRepeatedly(Return(SUPPORTED_LOCALES));
                EXPECT_CALL(*m_assetsManagerMock, getDefaultSupportedWakeWords()).WillRepeatedly(Return(SUPPORTED_WAKE_WORDS_COMBINATION));
                EXPECT_CALL(*m_assetsManagerMock, getSupportedWakeWords(_)).WillRepeatedly(Return(SUPPORTED_WAKE_WORDS_COMBINATION));
                EXPECT_CALL(*m_assetsManagerMock, cancelOngoingChange()).Times(AtMost(1));
            }
            string LocaleWakeWordsSettingTest::toJson(set<string> values) const {
                return jsonUtils::convertToJsonString(values);
            }
            vector<tuple<string, string, SettingStatus>> convertToDBData(string key, string value, SettingStatus status) {
                vector<tuple<string, string, SettingStatus>> datum;
                datum.push_back(make_tuple(key, value, status));
                return datum;
            }
            void LocaleWakeWordsSettingTest::initializeSetting(
                const LocalesSetting::ValueType& locales,
                const WakeWordsSetting::ValueType wakeWords) {
                WaitEvent e;
                EXPECT_CALL(*m_assetsManagerMock, getDefaultLocale()).WillOnce(Return(FRENCH_CANADA_VALUE));
                EXPECT_CALL(*m_storageMock, loadSetting(LOCALES_KEY))
                    .WillOnce(Return(make_pair(SettingStatus::SYNCHRONIZED, settings::toSettingString<DeviceLocales>(locales).second)));
                EXPECT_CALL(*m_storageMock, loadSetting(WAKE_WORDS_KEY))
                    .WillOnce(Return(make_pair(SettingStatus::SYNCHRONIZED, jsonUtils::convertToJsonString(wakeWords))));
                EXPECT_CALL(*m_assetsManagerMock, changeAssets(locales, wakeWords)).WillOnce(InvokeWithoutArgs([&e] {
                    e.wakeUp();
                    return true;
                }));
                m_setting = LocaleWakeWordsSetting::create(m_localesSenderMock, m_wakeWordsSenderMock, m_storageMock,
                                                             m_assetsManagerMock);
                ASSERT_THAT(m_setting, NotNull());
                LocalesSetting& localeSetting = *m_setting;
                WakeWordsSetting& wakeWordsSetting = *m_setting;
                ASSERT_EQ(localeSetting.get(), locales);
                ASSERT_EQ(wakeWordsSetting.get(), wakeWords);
                localeSetting.addObserver(m_localeObserver);
                wakeWordsSetting.addObserver(m_wakeWordsObserver);
                e.wait(MY_WAIT_TIMEOUT);
            }
            TEST_F(LocaleWakeWordsSettingTest, test_nullEventSenders) {
                ASSERT_FALSE(LocaleWakeWordsSetting::create(nullptr, m_wakeWordsSenderMock, m_storageMock, m_assetsManagerMock));
                ASSERT_FALSE(LocaleWakeWordsSetting::create(m_localesSenderMock, nullptr, m_storageMock, m_assetsManagerMock));
            }
            TEST_F(LocaleWakeWordsSettingTest, test_nullStorage) {
                ASSERT_FALSE(LocaleWakeWordsSetting::create(m_localesSenderMock, m_wakeWordsSenderMock, nullptr, m_assetsManagerMock));
            }
            TEST_F(LocaleWakeWordsSettingTest, test_nullAssetsManager) {
                ASSERT_FALSE(LocaleWakeWordsSetting::create(m_localesSenderMock, m_wakeWordsSenderMock, m_storageMock, nullptr));
            }
            TEST_F(LocaleWakeWordsSettingTest, test_restoreSynchronized) {
                initializeSetting({ENGLISH_CANADA_VALUE}, WakeWordsSetting::ValueType({ALEXA_VALUE}));
            }
            TEST_F(LocaleWakeWordsSettingTest, test_AVSChangeWakeWordsRequest) {
                initializeSetting({ENGLISH_CANADA_VALUE}, WakeWordsSetting::ValueType({ALEXA_VALUE}));
                WakeWordsSetting::ValueType newWakeWords{ALEXA_VALUE, ECHO_VALUE};
                auto newWakeWordsJson = toJson({ALEXA_VALUE, ECHO_VALUE});
                WaitEvent event;
                EXPECT_CALL(*m_storageMock,storeSettings(convertToDBData(WAKE_WORDS_KEY, newWakeWordsJson, SettingStatus::AVS_CHANGE_IN_PROGRESS)))
                    .WillOnce(Return(true));
                EXPECT_CALL(*m_wakeWordsObserver, onSettingNotification(_, SettingNotifications::AVS_CHANGE_IN_PROGRESS));
                EXPECT_CALL(*m_assetsManagerMock, changeAssets(ENGLISH_LOCALES, newWakeWords)).WillOnce(Return(true));
                EXPECT_CALL(*m_wakeWordsObserver, onSettingNotification(newWakeWords, SettingNotifications::AVS_CHANGE));
                EXPECT_CALL(*m_wakeWordsSenderMock, sendReportEvent(newWakeWordsJson)).WillOnce(InvokeWithoutArgs([] {
                    promise<bool> retPromise;
                    retPromise.set_value(true);
                    return retPromise.get_future();
                }));
                {
                    InSequence dummy;
                    EXPECT_CALL(*m_storageMock, updateSettingStatus(WAKE_WORDS_KEY, SettingStatus::AVS_CHANGE_IN_PROGRESS)).WillOnce(Return(true));
                    EXPECT_CALL(*m_storageMock, updateSettingStatus(WAKE_WORDS_KEY, SettingStatus::SYNCHRONIZED))
                        .WillOnce(InvokeWithoutArgs([&event] {
                            event.wakeUp();
                            return true;
                        }));
                }
                m_setting->setAvsChange(newWakeWords);
                EXPECT_TRUE(event.wait(MY_WAIT_TIMEOUT));
                EXPECT_EQ(static_cast<shared_ptr<WakeWordsSetting>>(m_setting)->get(), newWakeWords);
            }
            TEST_F(LocaleWakeWordsSettingTest, test_AVSChangeWakeWordsRequestSetFailed) {
                WakeWordsSetting::ValueType initialWakeWords{ALEXA_VALUE};
                initializeSetting({ENGLISH_CANADA_VALUE}, initialWakeWords);
                WakeWordsSetting::ValueType newWakeWords{ALEXA_VALUE, ECHO_VALUE};
                WaitEvent event;
                EXPECT_CALL(*m_wakeWordsObserver, onSettingNotification(_, SettingNotifications::AVS_CHANGE_IN_PROGRESS));
                EXPECT_CALL(*m_assetsManagerMock, changeAssets(ENGLISH_LOCALES, newWakeWords)).WillOnce(Return(false));
                EXPECT_CALL(*m_wakeWordsObserver, onSettingNotification(initialWakeWords, SettingNotifications::AVS_CHANGE_FAILED));
                EXPECT_CALL(*m_wakeWordsSenderMock, sendReportEvent(toJson({ALEXA_VALUE}))).WillOnce(InvokeWithoutArgs([] {
                    std::promise<bool> retPromise;
                    retPromise.set_value(true);
                    return retPromise.get_future();
                }));
                {
                    InSequence dummy;
                    EXPECT_CALL(*m_storageMock, updateSettingStatus(WAKE_WORDS_KEY, SettingStatus::AVS_CHANGE_IN_PROGRESS))
                        .WillOnce(Return(true));
                    EXPECT_CALL(*m_storageMock, updateSettingStatus(WAKE_WORDS_KEY, SettingStatus::SYNCHRONIZED))
                        .WillOnce(InvokeWithoutArgs([&event] {
                            event.wakeUp();
                            return true;
                        }));
                }
                m_setting->setAvsChange(newWakeWords);
                EXPECT_TRUE(event.wait(MY_WAIT_TIMEOUT));
                EXPECT_EQ(static_cast<shared_ptr<WakeWordsSetting>>(m_setting)->get(), initialWakeWords);
            }
            TEST_F(LocaleWakeWordsSettingTest, test_AVSChangeWakeWordsRequestSendEventFailed) {
                initializeSetting({ENGLISH_CANADA_VALUE}, WakeWordsSetting::ValueType({ALEXA_VALUE}));
                WakeWordsSetting::ValueType newWakeWords{ALEXA_VALUE, ECHO_VALUE};
                auto newWakeWordsJson = toJson({ALEXA_VALUE, ECHO_VALUE});
                WaitEvent event;
                EXPECT_CALL(*m_storageMock, updateSettingStatus(WAKE_WORDS_KEY, SettingStatus::AVS_CHANGE_IN_PROGRESS)).WillOnce(Return(true));
                EXPECT_CALL(*m_storageMock, updateSettingStatus(WAKE_WORDS_KEY, SettingStatus::SYNCHRONIZED)).Times(0);
                EXPECT_CALL(*m_storageMock,storeSettings(convertToDBData(WAKE_WORDS_KEY, newWakeWordsJson, SettingStatus::AVS_CHANGE_IN_PROGRESS)))
                    .WillOnce(Return(true));
                EXPECT_CALL(*m_wakeWordsObserver, onSettingNotification(_, SettingNotifications::AVS_CHANGE_IN_PROGRESS));
                EXPECT_CALL(*m_assetsManagerMock, changeAssets(ENGLISH_LOCALES, newWakeWords)).WillOnce(Return(true));
                EXPECT_CALL(*m_wakeWordsObserver, onSettingNotification(newWakeWords, SettingNotifications::AVS_CHANGE));
                EXPECT_CALL(*m_wakeWordsSenderMock, sendReportEvent(newWakeWordsJson)).WillOnce(InvokeWithoutArgs([&] {
                    std::promise<bool> retPromise;
                    retPromise.set_value(false);
                    event.wakeUp();
                    return retPromise.get_future();
                }));
                m_setting->setAvsChange(newWakeWords);
                EXPECT_TRUE(event.wait(MY_WAIT_TIMEOUT));
                EXPECT_EQ(static_cast<std::shared_ptr<WakeWordsSetting>>(m_setting)->get(), newWakeWords);
            }
            TEST_F(LocaleWakeWordsSettingTest, test_AVSChangeLocaleRequest) {
                WakeWordsSetting::ValueType wakeWords{ALEXA_VALUE};
                initializeSetting({ENGLISH_CANADA_VALUE}, wakeWords);
                LocalesSetting::ValueType newLocale{FRENCH_CANADA_VALUE};
                auto newLocaleJson = toSettingString<DeviceLocales>(newLocale).second;
                WaitEvent event;
                EXPECT_CALL(*m_storageMock,storeSettings(convertToDBData(LOCALES_KEY, newLocaleJson, SettingStatus::AVS_CHANGE_IN_PROGRESS)))
                    .WillOnce(Return(true));
                EXPECT_CALL(*m_localeObserver, onSettingNotification(_, SettingNotifications::AVS_CHANGE_IN_PROGRESS));
                EXPECT_CALL(*m_assetsManagerMock, changeAssets(newLocale, wakeWords)).WillOnce(Return(true));
                EXPECT_CALL(*m_localeObserver, onSettingNotification(newLocale, SettingNotifications::AVS_CHANGE));
                EXPECT_CALL(*m_localesSenderMock, sendReportEvent(newLocaleJson)).WillOnce(InvokeWithoutArgs([] {
                    promise<bool> retPromise;
                    retPromise.set_value(true);
                    return retPromise.get_future();
                }));
                InSequence dummy;
                {
                    EXPECT_CALL(*m_storageMock, updateSettingStatus(LOCALES_KEY, SettingStatus::AVS_CHANGE_IN_PROGRESS)).WillOnce(Return(true));
                    EXPECT_CALL(*m_storageMock, updateSettingStatus(LOCALES_KEY, SettingStatus::SYNCHRONIZED))
                        .WillOnce(InvokeWithoutArgs([&event] {
                            event.wakeUp();
                            return true;
                        }));
                }
                m_setting->setAvsChange(newLocale);
                EXPECT_TRUE(event.wait(MY_WAIT_TIMEOUT));
                EXPECT_EQ(static_cast<shared_ptr<LocalesSetting>>(m_setting)->get(), newLocale);
            }
            TEST_F(LocaleWakeWordsSettingTest, test_AVSChangeLocaleRequestSetFailed) {
                WakeWordsSetting::ValueType wakeWords{ALEXA_VALUE};
                initializeSetting({ENGLISH_CANADA_VALUE}, wakeWords);
                LocalesSetting::ValueType newLocale{FRENCH_CANADA_VALUE};
                auto initialLocaleJson = toSettingString<DeviceLocales>({ENGLISH_CANADA_VALUE}).second;
                WaitEvent event;
                EXPECT_CALL(*m_localeObserver, onSettingNotification(_, SettingNotifications::AVS_CHANGE_IN_PROGRESS));
                EXPECT_CALL(*m_assetsManagerMock, changeAssets(newLocale, wakeWords)).WillOnce(Return(false));
                EXPECT_CALL(*m_localeObserver,onSettingNotification(DeviceLocales({ENGLISH_CANADA_VALUE}), SettingNotifications::AVS_CHANGE_FAILED));
                EXPECT_CALL(*m_localesSenderMock, sendReportEvent(initialLocaleJson)).WillOnce(InvokeWithoutArgs([] {
                    promise<bool> retPromise;
                    retPromise.set_value(true);
                    return retPromise.get_future();
                }));
                InSequence dummy;
                {
                    EXPECT_CALL(*m_storageMock, updateSettingStatus(LOCALES_KEY, SettingStatus::AVS_CHANGE_IN_PROGRESS)).WillOnce(Return(true));
                    EXPECT_CALL(*m_storageMock, updateSettingStatus(LOCALES_KEY, SettingStatus::SYNCHRONIZED))
                        .WillOnce(InvokeWithoutArgs([&event] {
                            event.wakeUp();
                            return true;
                        }));
                }
                m_setting->setAvsChange(newLocale);
                EXPECT_TRUE(event.wait(MY_WAIT_TIMEOUT));
                EXPECT_EQ(static_cast<shared_ptr<LocalesSetting>>(m_setting)->get(),LocalesSetting::ValueType{ENGLISH_CANADA_VALUE});
            }
            TEST_F(LocaleWakeWordsSettingTest, test_AVSChangeLocaleRequestSendEventFailed) {
                WakeWordsSetting::ValueType wakeWords{ALEXA_VALUE};
                initializeSetting({ENGLISH_CANADA_VALUE}, wakeWords);
                LocalesSetting::ValueType newLocale{FRENCH_CANADA_VALUE};
                auto newLocaleJson = toSettingString<DeviceLocales>(newLocale).second;
                WaitEvent event;
                EXPECT_CALL(*m_storageMock, updateSettingStatus(LOCALES_KEY, SettingStatus::AVS_CHANGE_IN_PROGRESS)).WillOnce(Return(true));
                EXPECT_CALL(*m_storageMock, updateSettingStatus(WAKE_WORDS_KEY, SettingStatus::SYNCHRONIZED)).Times(0);
                EXPECT_CALL(*m_storageMock,storeSettings(convertToDBData(LOCALES_KEY, newLocaleJson, SettingStatus::AVS_CHANGE_IN_PROGRESS)))
                    .WillOnce(Return(true));
                EXPECT_CALL(*m_localeObserver, onSettingNotification(_, SettingNotifications::AVS_CHANGE_IN_PROGRESS));
                EXPECT_CALL(*m_assetsManagerMock, changeAssets(newLocale, wakeWords)).WillOnce(Return(true));
                EXPECT_CALL(*m_localeObserver, onSettingNotification(newLocale, SettingNotifications::AVS_CHANGE));
                EXPECT_CALL(*m_localesSenderMock, sendReportEvent(newLocaleJson)).WillOnce(InvokeWithoutArgs([&] {
                    promise<bool> retPromise;
                    retPromise.set_value(false);
                    event.wakeUp();
                    return retPromise.get_future();
                }));
                m_setting->setAvsChange(newLocale);
                EXPECT_TRUE(event.wait(MY_WAIT_TIMEOUT));
                EXPECT_EQ(static_cast<std::shared_ptr<LocalesSetting>>(m_setting)->get(), newLocale);
            }
            TEST_F(LocaleWakeWordsSettingTest, test_restoreValueNotAvailable) {
                EXPECT_CALL(*m_assetsManagerMock, getDefaultLocale()).WillOnce(Return(FRENCH_CANADA_VALUE));
                EXPECT_CALL(*m_storageMock, loadSetting(LOCALES_KEY))
                    .WillOnce(Return(std::make_pair(SettingStatus::NOT_AVAILABLE, "")));
                EXPECT_CALL(*m_storageMock, loadSetting(WAKE_WORDS_KEY))
                    .WillOnce(Return(std::make_pair(SettingStatus::NOT_AVAILABLE, "")));
                WakeWordsSetting::ValueType initialWakeWords{{ALEXA_VALUE}};
                LocalesSetting::ValueType initialLocale{FRENCH_CANADA_VALUE};
                auto wakeWordsJson = toJson({ALEXA_VALUE});
                auto localeJson = toSettingString<DeviceLocales>(DeviceLocales{FRENCH_CANADA_VALUE}).second;
                vector<tuple<string, string, SettingStatus>> datum;
                datum.push_back(make_tuple(LOCALES_KEY, localeJson, SettingStatus::LOCAL_CHANGE_IN_PROGRESS));
                datum.push_back(make_tuple(WAKE_WORDS_KEY, wakeWordsJson, SettingStatus::LOCAL_CHANGE_IN_PROGRESS));
                EXPECT_CALL(*m_storageMock, storeSettings(datum)).WillOnce(Return(true));
                EXPECT_CALL(*m_assetsManagerMock, changeAssets(initialLocale, initialWakeWords)).WillOnce(Return(true));
                EXPECT_CALL(*m_wakeWordsSenderMock, sendChangedEvent(wakeWordsJson)).WillOnce(InvokeWithoutArgs([] {
                    promise<bool> retPromise;
                    retPromise.set_value(true);
                    return retPromise.get_future();
                }));
                EXPECT_CALL(*m_localesSenderMock, sendChangedEvent(localeJson)).WillOnce(InvokeWithoutArgs([] {
                    promise<bool> retPromise;
                    retPromise.set_value(true);
                    return retPromise.get_future();
                }));
                condition_variable updatedCV;
                bool wakeWordStateUpdated = false;
                bool localeStateUpdated = false;
                mutex statusMutex;
                EXPECT_CALL(*m_storageMock, updateSettingStatus(WAKE_WORDS_KEY, SettingStatus::SYNCHRONIZED))
                    .WillOnce(InvokeWithoutArgs([&wakeWordStateUpdated, &statusMutex, &updatedCV] {
                        lock_guard<mutex> lock{statusMutex};
                        wakeWordStateUpdated = true;
                        updatedCV.notify_all();
                        return true;
                    }));
                EXPECT_CALL(*m_storageMock, updateSettingStatus(LOCALES_KEY, SettingStatus::SYNCHRONIZED))
                    .WillOnce(InvokeWithoutArgs([&localeStateUpdated, &statusMutex, &updatedCV] {
                        lock_guard<mutex> lock{statusMutex};
                        localeStateUpdated = true;
                        updatedCV.notify_all();
                        return true;
                    }));
                m_setting = LocaleWakeWordsSetting::create(m_localesSenderMock, m_wakeWordsSenderMock, m_storageMock,
                                              m_assetsManagerMock);
                ASSERT_THAT(m_setting, NotNull());
                unique_lock<mutex> statusLock{statusMutex};
                EXPECT_TRUE(updatedCV.wait_for(statusLock, MY_WAIT_TIMEOUT, [&wakeWordStateUpdated, &localeStateUpdated] {
                    return wakeWordStateUpdated && localeStateUpdated;
                }));
                EXPECT_EQ(static_cast<shared_ptr<LocalesSetting>>(m_setting)->get(), initialLocale);
                EXPECT_EQ(static_cast<shared_ptr<WakeWordsSetting>>(m_setting)->get(), initialWakeWords);
            }
            TEST_F(LocaleWakeWordsSettingTest, test_localChangeWakeWordsRequest) {
                initializeSetting({ENGLISH_CANADA_VALUE}, WakeWordsSetting::ValueType({ALEXA_VALUE}));
                WakeWordsSetting::ValueType newWakeWords{ALEXA_VALUE, ECHO_VALUE};
                auto newWakeWordsJson = toJson({ALEXA_VALUE, ECHO_VALUE});
                WaitEvent event;
                EXPECT_CALL(*m_storageMock,storeSettings(convertToDBData(WAKE_WORDS_KEY, newWakeWordsJson, SettingStatus::LOCAL_CHANGE_IN_PROGRESS)))
                    .WillOnce(Return(true));
                EXPECT_CALL(*m_wakeWordsObserver, onSettingNotification(_, SettingNotifications::LOCAL_CHANGE_IN_PROGRESS));
                EXPECT_CALL(*m_assetsManagerMock, changeAssets(ENGLISH_LOCALES, newWakeWords)).WillOnce(Return(true));
                EXPECT_CALL(*m_wakeWordsObserver, onSettingNotification(newWakeWords, SettingNotifications::LOCAL_CHANGE));
                EXPECT_CALL(*m_wakeWordsSenderMock, sendChangedEvent(newWakeWordsJson)).WillOnce(InvokeWithoutArgs([] {
                    promise<bool> retPromise;
                    retPromise.set_value(true);
                    return retPromise.get_future();
                }));
                EXPECT_CALL(*m_storageMock, updateSettingStatus(WAKE_WORDS_KEY, SettingStatus::SYNCHRONIZED))
                    .WillOnce(InvokeWithoutArgs([&event] {
                        event.wakeUp();
                        return true;
                    }));
                m_setting->setLocalChange(newWakeWords);
                EXPECT_TRUE(event.wait(MY_WAIT_TIMEOUT));
                EXPECT_EQ(static_cast<shared_ptr<WakeWordsSetting>>(m_setting)->get(), newWakeWords);
            }
            TEST_F(LocaleWakeWordsSettingTest, test_localChangeWakeWordsRequestSetFailed) {
                WakeWordsSetting::ValueType initialWakeWords{{ALEXA_VALUE}};
                initializeSetting({ENGLISH_CANADA_VALUE}, initialWakeWords);
                WakeWordsSetting::ValueType newWakeWords{ALEXA_VALUE, ECHO_VALUE};
                auto initialWakeWordsJson = toJson({ALEXA_VALUE});
                WaitEvent event;
                EXPECT_CALL(*m_wakeWordsObserver, onSettingNotification(_, SettingNotifications::LOCAL_CHANGE_IN_PROGRESS));
                EXPECT_CALL(*m_assetsManagerMock, changeAssets(ENGLISH_LOCALES, newWakeWords)).WillOnce(Return(false));
                EXPECT_CALL(*m_wakeWordsObserver, onSettingNotification(initialWakeWords, SettingNotifications::LOCAL_CHANGE_FAILED))
                    .WillOnce(InvokeWithoutArgs([&event] { event.wakeUp(); }));
                EXPECT_CALL(*m_storageMock, updateSettingStatus(WAKE_WORDS_KEY, SettingStatus::SYNCHRONIZED)).Times(0);
                m_setting->setLocalChange(newWakeWords);
                EXPECT_TRUE(event.wait(MY_WAIT_TIMEOUT));
                EXPECT_EQ(static_cast<std::shared_ptr<WakeWordsSetting>>(m_setting)->get(), initialWakeWords);
            }
            TEST_F(LocaleWakeWordsSettingTest, test_localChangeWakeWordsRequestSendEventFailed) {}
            TEST_F(LocaleWakeWordsSettingTest, test_localChangeLocaleRequest) {}
            TEST_F(LocaleWakeWordsSettingTest, test_localChangeLocaleRequestSetFailed) {}
            TEST_F(LocaleWakeWordsSettingTest, test_localChangeLocaleRequestSendEventFailed) {}
            TEST_F(LocaleWakeWordsSettingTest, test_localeChangeTriggerWakeWordsChange) {}
            TEST_F(LocaleWakeWordsSettingTest, test_localeChangeCancelPendingWakeWordsChange) {}
            TEST_F(LocaleWakeWordsSettingTest, test_localeChangeMergePendingWakeWordsChange) {}
            TEST_F(LocaleWakeWordsSettingTest, test_wakeWordChangeRequestFailedOnCurrentLocale) {}
            TEST_F(LocaleWakeWordsSettingTest, test_reconnectAfterOfflineChange) {
                WakeWordsSetting::ValueType wakeWords{ALEXA_VALUE};
                initializeSetting({ENGLISH_CANADA_VALUE}, wakeWords);
                LocalesSetting::ValueType newLocale{FRENCH_CANADA_VALUE};
                auto newLocaleJson = toSettingString<DeviceLocales>({newLocale}).second;
                WaitEvent event;
                EXPECT_CALL(*m_storageMock, updateSettingStatus(LOCALES_KEY, SettingStatus::AVS_CHANGE_IN_PROGRESS))
                    .WillOnce(Return(true));
                EXPECT_CALL(*m_storageMock,storeSettings(convertToDBData(LOCALES_KEY, newLocaleJson, SettingStatus::AVS_CHANGE_IN_PROGRESS)))
                    .WillOnce(Return(true));
                EXPECT_CALL(*m_localeObserver, onSettingNotification(_, SettingNotifications::AVS_CHANGE_IN_PROGRESS));
                EXPECT_CALL(*m_assetsManagerMock, changeAssets(newLocale, wakeWords)).WillOnce(Return(true));
                EXPECT_CALL(*m_localeObserver, onSettingNotification(newLocale, SettingNotifications::AVS_CHANGE));
                EXPECT_CALL(*m_localesSenderMock, sendReportEvent(newLocaleJson)).WillOnce(InvokeWithoutArgs([&] {
                    promise<bool> retPromise;
                    retPromise.set_value(false);
                    event.wakeUp();
                    return retPromise.get_future();
                }));
                m_setting->setAvsChange(newLocale);
                ASSERT_TRUE(event.wait(MY_WAIT_TIMEOUT));
                event.reset();
                EXPECT_CALL(*m_localesSenderMock, sendReportEvent(newLocaleJson)).WillOnce(InvokeWithoutArgs([] {
                    promise<bool> retPromise;
                    retPromise.set_value(true);
                    return retPromise.get_future();
                }));
                EXPECT_CALL(*m_storageMock, updateSettingStatus(LOCALES_KEY, SettingStatus::SYNCHRONIZED))
                    .WillOnce(InvokeWithoutArgs([&event] {
                        event.wakeUp();
                        return true;
                    }));
                m_setting->onConnectionStatusChanged(ConnectionStatusObserverInterface::Status::CONNECTED,
                                                     ConnectionStatusObserverInterface::ChangedReason::SUCCESS);
                ASSERT_TRUE(event.wait(MY_WAIT_TIMEOUT));
            }
        }
    }
}
