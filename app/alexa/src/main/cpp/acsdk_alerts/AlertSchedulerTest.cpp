#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <gmock/gmock-actions.h>
#include <metrics/MetricRecorderInterface.h>
#include <metrics/MockMetricRecorder.h>
#include <timing/TimeUtils.h>
#include <registration_manager/CustomerDataManager.h>
#include <settings/DeviceSettingsManager.h>
#include "AlertScheduler.h"

namespace alexaClientSDK {
    namespace acsdkAlerts {
        namespace test {
            using namespace std;
            using namespace chrono;
            using namespace avsCommon;
            using namespace acsdkAlertsInterfaces;
            using namespace avs;
            using namespace utils;
            using namespace metrics;
            using namespace settings;
            using namespace timing;
            using namespace testing;
            using namespace rapidjson;
            using namespace registrationManager;
            using namespace renderer;
            using namespace metrics::test;
            static const string ALERT1_TOKEN = "token1";
            static const string ALERT2_TOKEN = "token2";
            static const string ALERT3_TOKEN = "token3";
            static const string ALERT4_TOKEN = "token4";
            static const string ALERT_TYPE = "TEST_ALERT_TYPE";
            static const string PAST_INSTANT = "2000-01-01T12:34:56+0000";
            static const string FUTURE_INSTANT_SUFFIX = "-01-01T12:34:56+0000";
            static const milliseconds TEST_TIMEOUT{100};
            static const seconds ALERT_PAST_DUE_TIME_LIMIT{10};
            class MockRenderer : public RendererInterface {
            public:
                MOCK_METHOD7(start, void(shared_ptr<RendererObserverInterface> observer, function<pair<unique_ptr<istream>, const MediaType>()> audioFactory,
                             bool alarmVolumeRampEnabled, const vector<string>& urls, int loopCount, milliseconds loopPause, bool startWithPause));
                MOCK_METHOD0(stop, void());
            };
            class TestAlert : public Alert {
            public:
                TestAlert() : Alert(defaultAudioFactory, shortAudioFactory,nullptr), m_alertType{ALERT_TYPE}, m_renderer{make_shared<MockRenderer>()} {
                    this->setRenderer(m_renderer);
                }
                TestAlert(const string& token, const string& schedTime) : Alert(defaultAudioFactory, shortAudioFactory,nullptr),
                                                                          m_alertType{ALERT_TYPE}, m_renderer{make_shared<MockRenderer>()} {
                    this->setRenderer(m_renderer);
                    const string payloadJson = "{\"token\": \"" + token + "\",\"type\": \"" + m_alertType + "\",\"scheduledTime\": \"" + schedTime + "\"}";
                    string errorMessage;
                    Document payload;
                    payload.Parse(payloadJson.data());
                    this->parseFromJson(payload, &errorMessage);
                }
                string getTypeName() const override {
                    return m_alertType;
                }
            private:
                static pair<unique_ptr<istream>, const MediaType> defaultAudioFactory() {
                    return pair<unique_ptr<istream>, const MediaType>(unique_ptr<stringstream>(new stringstream("default audio")),MediaType::MPEG);
                }
                static pair<unique_ptr<istream>, const MediaType> shortAudioFactory() {
                    return pair<unique_ptr<istream>, const MediaType>(unique_ptr<stringstream>(new stringstream("short audio")),MediaType::MPEG);
                }
                const string m_alertType;
                shared_ptr<MockRenderer> m_renderer;
            };
            class MockAlertStorage : public storage::AlertStorageInterface {
            public:
                MockAlertStorage() {
                    m_createDatabaseRetVal = true;
                    m_openRetVal = true;
                    m_isOpenRetVal = true;
                    m_alertExistsRetVal = true;
                    m_storeRetVal = true;
                    m_loadRetVal = true;
                    m_eraseRetVal = true;
                }
                bool createDatabase() {
                    return m_createDatabaseRetVal;
                }
                bool open() {
                    return m_openRetVal;
                }
                bool isOpen() {
                    return m_isOpenRetVal;
                }
                void close() {
                }
                bool alertExists(const std::string& token) {
                    return m_alertExistsRetVal;
                }
                bool store(std::shared_ptr<Alert> alert) {
                    return m_storeRetVal;
                }
                bool load(vector<shared_ptr<Alert>>* alertContainer, shared_ptr<DeviceSettingsManager> settingsManager) {
                    if (m_loadRetVal) {
                        alertContainer->clear();
                        for (shared_ptr<Alert> alertToAdd : m_alertsInStorage) {
                            alertContainer->push_back(alertToAdd);
                        }
                    }
                    return m_loadRetVal;
                }
                bool erase(const vector<int>& alertDbIds) {
                    return m_eraseRetVal;
                }
                void setCreateDatabaseRetVal(bool retVal) {
                    m_createDatabaseRetVal = retVal;
                }
                void setOpenRetVal(bool retVal) {
                    m_openRetVal = retVal;
                }
                void setIsOpenRetVal(bool retVal) {
                    m_isOpenRetVal = retVal;
                }
                void setAlertExistsRetVal(bool retVal) {
                    m_alertExistsRetVal = retVal;
                }
                void setStoreRetVal(bool retVal) {
                    m_storeRetVal = retVal;
                }
                void setLoadRetVal(bool retVal) {
                    m_loadRetVal = retVal;
                }
                void setEraseRetVal(bool retVal) {
                    m_eraseRetVal = retVal;
                }
                void setAlerts(vector<shared_ptr<TestAlert>> alertsToAdd) {
                    m_alertsInStorage.clear();
                    for (shared_ptr<TestAlert> alertToAdd : alertsToAdd) m_alertsInStorage.push_back(alertToAdd);
                }
                MOCK_METHOD1(bulkErase, bool(const std::list<std::shared_ptr<Alert>>&));
                MOCK_METHOD1(erase, bool(std::shared_ptr<Alert>));
                MOCK_METHOD1(modify, bool(std::shared_ptr<Alert>));
                MOCK_METHOD0(clearDatabase, bool());
            private:
                vector<shared_ptr<Alert>> m_alertsInStorage;
                bool m_createDatabaseRetVal;
                bool m_openRetVal;
                bool m_isOpenRetVal;
                bool m_alertExistsRetVal;
                bool m_storeRetVal;
                bool m_loadRetVal;
                bool m_eraseRetVal;
            };
            class TestAlertObserver : public AlertObserverInterface {
            public:
                bool waitFor(AlertScheduler::State newState) {
                    unique_lock<mutex> lock(m_mutex);
                    return m_conditionVariable.wait_for(lock, TEST_TIMEOUT, [this, newState] { return m_state == newState; });
                }
                void onAlertStateChange(const string& alertToken, const string& alertType, AlertScheduler::State newState, const string& reason) {
                    lock_guard<mutex> lock(m_mutex);
                    m_state = newState;
                    m_conditionVariable.notify_all();
                }
            private:
                mutex m_mutex;
                condition_variable m_conditionVariable;
                AlertScheduler::State m_state;
            };
            class AlertSchedulerTest : public Test {
            public:
                AlertSchedulerTest();
                void SetUp() override;
                shared_ptr<TestAlert> doSimpleTestSetup(bool activateAlert = false, bool initWithAlertObserver = false);
            protected:
                shared_ptr<MockAlertStorage> m_alertStorage;
                shared_ptr<MockRenderer> m_alertRenderer;
                seconds m_alertPastDueTimeLimit;
                shared_ptr<MetricRecorderInterface> m_metricRecorder;
                shared_ptr<AlertScheduler> m_alertScheduler;
                shared_ptr<TestAlertObserver> m_testAlertObserver;
                shared_ptr<DeviceSettingsManager> m_settingsManager;
            };
            static string getFutureInstant(int yearsPlus) {
                system_clock::time_point now = std::chrono::system_clock::now();
                time_t tt = system_clock::to_time_t(now);
                tm utc_tm = *gmtime(&tt);
                return to_string(utc_tm.tm_year + 1900 + yearsPlus) + FUTURE_INSTANT_SUFFIX;
            }
            static string getTimeNow() {
                string timeNowStr;
                auto timeNow = system_clock::now();
                TimeUtils().convertTimeToUtcIso8601Rfc3339(timeNow, &timeNowStr);
                return timeNowStr;
            }
            AlertSchedulerTest::AlertSchedulerTest() : m_alertStorage{make_shared<MockAlertStorage>()}, m_alertRenderer{make_shared<MockRenderer>()},
                                                       m_alertPastDueTimeLimit{ALERT_PAST_DUE_TIME_LIMIT}, m_metricRecorder{make_shared<NiceMock<MockMetricRecorder>>()},
                                                       m_alertScheduler{make_shared<AlertScheduler>(m_alertStorage, m_alertRenderer, m_alertPastDueTimeLimit,
                                                       m_metricRecorder)}, m_testAlertObserver{std::make_shared<TestAlertObserver>()} {}
            void AlertSchedulerTest::SetUp() {
                m_alertStorage->setOpenRetVal(true);
                m_settingsManager = make_shared<DeviceSettingsManager>(make_shared<CustomerDataManager>());
            }
            shared_ptr<TestAlert> AlertSchedulerTest::doSimpleTestSetup(bool activateAlert, bool initWithAlertObserver) {
                vector<shared_ptr<TestAlert>> alertToAdd;
                shared_ptr<TestAlert> alert = make_shared<TestAlert>(ALERT1_TOKEN, getFutureInstant(1));
                alertToAdd.push_back(alert);
                m_alertStorage->setAlerts(alertToAdd);
                if (initWithAlertObserver) m_alertScheduler->initialize(m_testAlertObserver, m_settingsManager);
                else {
                    shared_ptr<AlertScheduler> alertSchedulerObs{make_shared<AlertScheduler>(m_alertStorage, m_alertRenderer, m_alertPastDueTimeLimit, m_metricRecorder)};
                    m_alertScheduler->initialize(alertSchedulerObs, m_settingsManager);
                }
                if (activateAlert) {
                    alert->activate();
                    m_alertScheduler->updateFocus(avsCommon::avs::FocusState::BACKGROUND);
                }
                return alert;
            }
            TEST_F(AlertSchedulerTest, test_initialize) {
                ASSERT_FALSE(m_alertScheduler->initialize(nullptr, nullptr));
                ASSERT_FALSE(m_alertScheduler->initialize(nullptr, m_settingsManager));
                m_alertStorage->setOpenRetVal(false);
                m_alertStorage->setCreateDatabaseRetVal(false);
                ASSERT_FALSE(m_alertScheduler->initialize(m_alertScheduler, m_settingsManager));
                shared_ptr<AlertScheduler> alertSchedulerObs{make_shared<AlertScheduler>(m_alertStorage, m_alertRenderer, m_alertPastDueTimeLimit, m_metricRecorder)};
                m_alertStorage->setOpenRetVal(true);
                ASSERT_TRUE(m_alertScheduler->initialize(alertSchedulerObs, m_settingsManager, false));
            }
            TEST_F(AlertSchedulerTest, test_updateGetFocus) {
                shared_ptr<TestAlert> alert = doSimpleTestSetup();
                m_alertScheduler->updateFocus(FocusState::FOREGROUND);
                ASSERT_EQ(m_alertScheduler->getFocusState(), FocusState::FOREGROUND);
                m_alertScheduler->updateFocus(FocusState::BACKGROUND);
                ASSERT_EQ(m_alertScheduler->getFocusState(), FocusState::BACKGROUND);
                m_alertScheduler->updateFocus(FocusState::NONE);
                ASSERT_EQ(alert->getState(), Alert::State::STOPPING);
            }
            TEST_F(AlertSchedulerTest, DISABLED_test_scheduleAlert) {
                shared_ptr<TestAlert> alert1 = doSimpleTestSetup(true);
                ASSERT_TRUE(m_alertScheduler->scheduleAlert(alert1));
                shared_ptr<TestAlert> alert2 = make_shared<TestAlert>(ALERT2_TOKEN, getFutureInstant(1));
                m_alertStorage->setStoreRetVal(false);
                ASSERT_FALSE(m_alertScheduler->scheduleAlert(alert2));
                shared_ptr<TestAlert> alert3 = make_shared<TestAlert>(ALERT3_TOKEN, PAST_INSTANT);
                m_alertStorage->setStoreRetVal(true);
                ASSERT_TRUE(m_alertScheduler->scheduleAlert(alert2));
                ASSERT_FALSE(m_alertScheduler->scheduleAlert(alert3));
            }
            TEST_F(AlertSchedulerTest, DISABLED_test_reloadAlertsFromDatabaseWithScheduling) {
                shared_ptr<AlertScheduler> alertSchedulerObs{make_shared<AlertScheduler>(m_alertStorage, m_alertRenderer, m_alertPastDueTimeLimit, m_metricRecorder)};
                m_alertStorage->setOpenRetVal(true);
                vector<shared_ptr<TestAlert>> alertsToAdd;
                m_alertScheduler->initialize(alertSchedulerObs, m_settingsManager, false);
                shared_ptr<TestAlert> alert1 = make_shared<TestAlert>(ALERT1_TOKEN, PAST_INSTANT);
                alertsToAdd.push_back(alert1);
                shared_ptr<TestAlert> alert2 = make_shared<TestAlert>(ALERT2_TOKEN, getFutureInstant(1));
                alert2->activate();
                alert2->setStateActive();
                alertsToAdd.push_back(alert2);
                shared_ptr<TestAlert> alert3 = make_shared<TestAlert>(ALERT3_TOKEN, getFutureInstant(1));
                alertsToAdd.push_back(alert3);
                m_alertStorage->setAlerts(alertsToAdd);
                EXPECT_CALL(*(m_alertStorage.get()), erase(testing::_)).Times(1);
                EXPECT_CALL(*(m_alertStorage.get()), modify(testing::_)).Times(1);
                const bool shouldScheduleAlerts = true;
                ASSERT_TRUE(m_alertScheduler->reloadAlertsFromDatabase(m_settingsManager, shouldScheduleAlerts));
                const unsigned int expectedRemainingAlerts = 2;
                ASSERT_EQ(m_alertScheduler->getContextInfo().scheduledAlerts.size(), expectedRemainingAlerts);
            }
            TEST_F(AlertSchedulerTest, test_reloadAlertsFromDatabaseWithoutScheduling) {
                shared_ptr<AlertScheduler> alertSchedulerObs{make_shared<AlertScheduler>(m_alertStorage, m_alertRenderer, m_alertPastDueTimeLimit, m_metricRecorder)};
                m_alertStorage->setOpenRetVal(true);
                vector<shared_ptr<TestAlert>> alertsToAdd;
                m_alertScheduler->initialize(alertSchedulerObs, m_settingsManager, false);
                shared_ptr<TestAlert> alert1 = make_shared<TestAlert>(ALERT1_TOKEN, PAST_INSTANT);
                alertsToAdd.push_back(alert1);
                shared_ptr<TestAlert> alert2 = make_shared<TestAlert>(ALERT2_TOKEN, getFutureInstant(1));
                alert2->activate();
                alert2->setStateActive();
                alertsToAdd.push_back(alert2);
                shared_ptr<TestAlert> alert3 = make_shared<TestAlert>(ALERT3_TOKEN, getFutureInstant(1));
                alertsToAdd.push_back(alert3);
                m_alertStorage->setAlerts(alertsToAdd);
                EXPECT_CALL(*(m_alertStorage.get()), erase(testing::_)).Times(0);
                EXPECT_CALL(*(m_alertStorage.get()), modify(testing::_)).Times(0);
                const bool shouldScheduleAlerts = false;
                ASSERT_TRUE(m_alertScheduler->reloadAlertsFromDatabase(m_settingsManager, shouldScheduleAlerts));
                const unsigned int expectedRemainingAlerts = 3;
                ASSERT_EQ(m_alertScheduler->getContextInfo().scheduledAlerts.size(), expectedRemainingAlerts);
            }
            TEST_F(AlertSchedulerTest, test_updateAlertTime) {
                auto oldAlert = doSimpleTestSetup();
                auto newAlert = make_shared<TestAlert>(oldAlert->getToken(), getFutureInstant(2));
                ASSERT_NE(oldAlert->getScheduledTime_ISO_8601(), newAlert->getScheduledTime_ISO_8601());
                EXPECT_CALL(*m_alertStorage, modify(_)).WillOnce(Return(true));
                EXPECT_TRUE(m_alertScheduler->scheduleAlert(newAlert));
                ASSERT_EQ(oldAlert->getScheduledTime_ISO_8601(), newAlert->getScheduledTime_ISO_8601());
            }
            TEST_F(AlertSchedulerTest, test_updateAlertAssets) {
                auto oldAlert = doSimpleTestSetup();
                auto newAlert = make_shared<TestAlert>(oldAlert->getToken(), oldAlert->getScheduledTime_ISO_8601());
                ASSERT_EQ(oldAlert->getScheduledTime_ISO_8601(), newAlert->getScheduledTime_ISO_8601());
                unordered_map<string, Alert::Asset> assets;
                assets["A"] = Alert::Asset("A", "http://test.com/a");
                assets["B"] = Alert::Asset("B", "http://test.com/a");
                vector<string> playOrderItems;
                playOrderItems.push_back("A");
                playOrderItems.push_back("B");
                Alert::AssetConfiguration c;
                c.assets = assets;
                c.assetPlayOrderItems = playOrderItems;
                c.backgroundAssetId = "A";
                c.loopPause = milliseconds(100);
                ASSERT_TRUE(newAlert->setAssetConfiguration(c));
                EXPECT_CALL(*m_alertStorage, modify(_)).WillOnce(Return(true));
                EXPECT_TRUE(m_alertScheduler->scheduleAlert(newAlert));
                ASSERT_EQ(oldAlert->getScheduledTime_ISO_8601(), newAlert->getScheduledTime_ISO_8601());
                Alert::AssetConfiguration oldAlertAssets = oldAlert->getAssetConfiguration();
                Alert::AssetConfiguration newAlertAssets = newAlert->getAssetConfiguration();
                ASSERT_EQ(oldAlertAssets.assets.size(), newAlertAssets.assets.size());
                ASSERT_EQ(oldAlertAssets.assets["A"].id, newAlertAssets.assets["A"].id);
                ASSERT_EQ(oldAlertAssets.assets["A"].url, newAlertAssets.assets["A"].url);
                ASSERT_EQ(oldAlertAssets.assets["B"].id, newAlertAssets.assets["B"].id);
                ASSERT_EQ(oldAlertAssets.assets["B"].url, newAlertAssets.assets["B"].url);
                ASSERT_EQ(oldAlertAssets.assetPlayOrderItems, newAlertAssets.assetPlayOrderItems);
                ASSERT_EQ(oldAlertAssets.backgroundAssetId, newAlertAssets.backgroundAssetId);
                ASSERT_EQ(oldAlertAssets.loopPause, newAlertAssets.loopPause);
            }
            TEST_F(AlertSchedulerTest, test_rescheduleAlertNow) {
                auto oldAlert = doSimpleTestSetup(false, true);
                auto newAlert = make_shared<TestAlert>(oldAlert->getToken(), getTimeNow());
                ASSERT_NE(oldAlert->getScheduledTime_ISO_8601(), newAlert->getScheduledTime_ISO_8601());
                EXPECT_CALL(*m_alertStorage, modify(_)).WillOnce(Return(true));
                EXPECT_TRUE(m_alertScheduler->scheduleAlert(newAlert));
                ASSERT_EQ(oldAlert->getScheduledTime_ISO_8601(), newAlert->getScheduledTime_ISO_8601());
                EXPECT_TRUE(m_testAlertObserver->waitFor(AlertObserverInterface::State::READY));
            }
            TEST_F(AlertSchedulerTest, test_rescheduleAlertFails) {
                auto oldAlert = doSimpleTestSetup();
                auto newAlert = make_shared<TestAlert>(oldAlert->getToken(), getFutureInstant(2));
                auto oldScheduledTime = oldAlert->getScheduledTime_ISO_8601();
                ASSERT_NE(newAlert->getScheduledTime_ISO_8601(), oldScheduledTime);
                EXPECT_CALL(*m_alertStorage, modify(_)).WillOnce(Return(false));
                EXPECT_FALSE(m_alertScheduler->scheduleAlert(newAlert));
                EXPECT_EQ(oldAlert->getScheduledTime_ISO_8601(), oldScheduledTime);
            }
            TEST_F(AlertSchedulerTest, test_snoozeAlert) {
                doSimpleTestSetup(true);
                ASSERT_FALSE(m_alertScheduler->snoozeAlert(ALERT2_TOKEN, getFutureInstant(1)));
                ASSERT_TRUE(m_alertScheduler->snoozeAlert(ALERT1_TOKEN, getFutureInstant(1)));
            }
            TEST_F(AlertSchedulerTest, test_deleteAlertSingle) {
                shared_ptr<AlertScheduler> alertSchedulerObs{make_shared<AlertScheduler>(m_alertStorage, m_alertRenderer, m_alertPastDueTimeLimit, m_metricRecorder)};
                vector<shared_ptr<TestAlert>> alertsToAdd;
                shared_ptr<TestAlert> alert1 = make_shared<TestAlert>(ALERT1_TOKEN, getFutureInstant(1));
                alertsToAdd.push_back(alert1);
                m_alertStorage->setAlerts(alertsToAdd);
                m_alertScheduler->initialize(alertSchedulerObs, m_settingsManager);
                m_alertScheduler->updateFocus(avsCommon::avs::FocusState::BACKGROUND);
                EXPECT_CALL(*(m_alertStorage.get()), erase(testing::_)).Times(0);
                ASSERT_TRUE(m_alertScheduler->deleteAlert(ALERT1_TOKEN));
                ASSERT_TRUE(m_alertScheduler->deleteAlert(ALERT2_TOKEN));
                shared_ptr<TestAlert> alert2 = std::make_shared<TestAlert>(ALERT2_TOKEN, getFutureInstant(1));
                alertsToAdd.push_back(alert2);
                m_alertStorage->setAlerts(alertsToAdd);
                m_alertScheduler->initialize(alertSchedulerObs, m_settingsManager);
                EXPECT_CALL(*(m_alertStorage.get()), erase(testing::_)).Times(1);
                ASSERT_TRUE(m_alertScheduler->deleteAlert(ALERT2_TOKEN));
            }
            TEST_F(AlertSchedulerTest, test_bulkDeleteAlertsSingle) {
                shared_ptr<AlertScheduler> alertSchedulerObs{make_shared<AlertScheduler>(m_alertStorage, m_alertRenderer, m_alertPastDueTimeLimit, m_metricRecorder)};
                vector<shared_ptr<TestAlert>> alertsToAdd;
                shared_ptr<TestAlert> alert1 = make_shared<TestAlert>(ALERT1_TOKEN, getFutureInstant(1));
                shared_ptr<TestAlert> alert2 = make_shared<TestAlert>(ALERT2_TOKEN, getFutureInstant(2));
                ON_CALL(*m_alertStorage.get(), bulkErase(_)).WillByDefault(Return(true));
                alertsToAdd.push_back(alert1);
                alertsToAdd.push_back(alert2);
                m_alertStorage->setAlerts(alertsToAdd);
                m_alertScheduler->initialize(alertSchedulerObs, m_settingsManager);
                ASSERT_EQ(m_alertScheduler->getAllAlerts().size(), 2u);
                EXPECT_TRUE(m_alertScheduler->deleteAlerts({ALERT1_TOKEN}));
                ASSERT_EQ(m_alertScheduler->getAllAlerts().size(), 1u);
                EXPECT_TRUE(m_alertScheduler->deleteAlerts({ALERT3_TOKEN}));
                ASSERT_EQ(m_alertScheduler->getAllAlerts().size(), 1u);
                EXPECT_TRUE(m_alertScheduler->deleteAlerts({ALERT2_TOKEN}));
                ASSERT_EQ(m_alertScheduler->getAllAlerts().size(), 0u);
            }
            TEST_F(AlertSchedulerTest, test_bulkDeleteAlertsMultipleExisting) {
                shared_ptr<AlertScheduler> alertSchedulerObs{make_shared<AlertScheduler>(m_alertStorage, m_alertRenderer, m_alertPastDueTimeLimit, m_metricRecorder)};
                vector<shared_ptr<TestAlert>> alertsToAdd;
                shared_ptr<TestAlert> alert1 = make_shared<TestAlert>(ALERT1_TOKEN, getFutureInstant(1));
                shared_ptr<TestAlert> alert2 = make_shared<TestAlert>(ALERT2_TOKEN, getFutureInstant(2));
                ON_CALL(*m_alertStorage.get(), bulkErase(_)).WillByDefault(Return(true));
                alertsToAdd.push_back(alert1);
                alertsToAdd.push_back(alert2);
                m_alertStorage->setAlerts(alertsToAdd);
                m_alertScheduler->initialize(alertSchedulerObs, m_settingsManager);
                EXPECT_TRUE(m_alertScheduler->deleteAlerts({ALERT1_TOKEN, ALERT2_TOKEN}));
                EXPECT_EQ(m_alertScheduler->getAllAlerts().size(), 0u);
            }
            TEST_F(AlertSchedulerTest, test_bulkDeleteAlertsMultipleMixed) {
                shared_ptr<AlertScheduler> alertSchedulerObs{make_shared<AlertScheduler>(m_alertStorage, m_alertRenderer, m_alertPastDueTimeLimit, m_metricRecorder)};
                vector<shared_ptr<TestAlert>> alertsToAdd;
                shared_ptr<TestAlert> alert1 = make_shared<TestAlert>(ALERT1_TOKEN, getFutureInstant(1));
                shared_ptr<TestAlert> alert2 = make_shared<TestAlert>(ALERT2_TOKEN, getFutureInstant(2));
                ON_CALL(*m_alertStorage.get(), bulkErase(_)).WillByDefault(Return(true));
                alertsToAdd.push_back(alert1);
                alertsToAdd.push_back(alert2);
                m_alertStorage->setAlerts(alertsToAdd);
                m_alertScheduler->initialize(alertSchedulerObs, m_settingsManager);
                EXPECT_TRUE(m_alertScheduler->deleteAlerts({ALERT1_TOKEN, ALERT3_TOKEN}));
                EXPECT_EQ(m_alertScheduler->getAllAlerts().size(), 1u);
            }
            TEST_F(AlertSchedulerTest, test_bulkDeleteAlertsMultipleMissing) {
                shared_ptr<AlertScheduler> alertSchedulerObs{make_shared<AlertScheduler>(m_alertStorage, m_alertRenderer, m_alertPastDueTimeLimit, m_metricRecorder)};
                vector<shared_ptr<TestAlert>> alertsToAdd;
                shared_ptr<TestAlert> alert1 = make_shared<TestAlert>(ALERT1_TOKEN, getFutureInstant(1));
                shared_ptr<TestAlert> alert2 = make_shared<TestAlert>(ALERT2_TOKEN, getFutureInstant(2));
                ON_CALL(*m_alertStorage.get(), bulkErase(_)).WillByDefault(Return(true));
                alertsToAdd.push_back(alert1);
                alertsToAdd.push_back(alert2);
                m_alertStorage->setAlerts(alertsToAdd);
                m_alertScheduler->initialize(alertSchedulerObs, m_settingsManager);
                EXPECT_TRUE(m_alertScheduler->deleteAlerts({ALERT3_TOKEN, ALERT4_TOKEN}));
                EXPECT_EQ(m_alertScheduler->getAllAlerts().size(), 2u);
            }
            TEST_F(AlertSchedulerTest, test_bulkDeleteAlertsMultipleSame) {
                shared_ptr<AlertScheduler> alertSchedulerObs{make_shared<AlertScheduler>(m_alertStorage, m_alertRenderer, m_alertPastDueTimeLimit, m_metricRecorder)};
                vector<shared_ptr<TestAlert>> alertsToAdd;
                shared_ptr<TestAlert> alert1 = make_shared<TestAlert>(ALERT1_TOKEN, getFutureInstant(1));
                shared_ptr<TestAlert> alert2 = make_shared<TestAlert>(ALERT2_TOKEN, getFutureInstant(2));
                ON_CALL(*m_alertStorage.get(), bulkErase(_)).WillByDefault(Return(true));
                alertsToAdd.push_back(alert1);
                alertsToAdd.push_back(alert2);
                m_alertStorage->setAlerts(alertsToAdd);
                m_alertScheduler->initialize(alertSchedulerObs, m_settingsManager);
                EXPECT_TRUE(m_alertScheduler->deleteAlerts({ALERT1_TOKEN, ALERT1_TOKEN}));
                EXPECT_EQ(m_alertScheduler->getAllAlerts().size(), 1u);
            }
            TEST_F(AlertSchedulerTest, test_bulkDeleteAlertsMultipleEmpty) {
                shared_ptr<AlertScheduler> alertSchedulerObs{make_shared<AlertScheduler>(m_alertStorage, m_alertRenderer, m_alertPastDueTimeLimit, m_metricRecorder)};
                vector<shared_ptr<TestAlert>> alertsToAdd;
                shared_ptr<TestAlert> alert1 = make_shared<TestAlert>(ALERT1_TOKEN, getFutureInstant(1));
                shared_ptr<TestAlert> alert2 = make_shared<TestAlert>(ALERT2_TOKEN, getFutureInstant(2));
                ON_CALL(*m_alertStorage.get(), bulkErase(_)).WillByDefault(Return(true));
                alertsToAdd.push_back(alert1);
                alertsToAdd.push_back(alert2);
                m_alertStorage->setAlerts(alertsToAdd);
                m_alertScheduler->initialize(alertSchedulerObs, m_settingsManager);
                EXPECT_TRUE(m_alertScheduler->deleteAlerts({}));
                EXPECT_EQ(m_alertScheduler->getAllAlerts().size(), 2u);
            }
            TEST_F(AlertSchedulerTest, test_isAlertActive) {
                shared_ptr<AlertScheduler> alertSchedulerObs{make_shared<AlertScheduler>(m_alertStorage, m_alertRenderer, m_alertPastDueTimeLimit, m_metricRecorder)};
                vector<shared_ptr<TestAlert>> alertsToAdd;
                shared_ptr<TestAlert> alert1 = std::make_shared<TestAlert>(ALERT1_TOKEN, getFutureInstant(1));
                alertsToAdd.push_back(alert1);
                m_alertStorage->setAlerts(alertsToAdd);
                m_alertScheduler->initialize(alertSchedulerObs, m_settingsManager);
                m_alertScheduler->updateFocus(FocusState::BACKGROUND);
                shared_ptr<TestAlert> alert2 = make_shared<TestAlert>(ALERT2_TOKEN, getFutureInstant(1));
                alertsToAdd.push_back(alert2);
                m_alertStorage->setAlerts(alertsToAdd);
                ASSERT_TRUE(m_alertScheduler->isAlertActive(alert1));
                ASSERT_FALSE(m_alertScheduler->isAlertActive(alert2));
            }
            TEST_F(AlertSchedulerTest, test_getContextInfo) {
                shared_ptr<AlertScheduler> alertSchedulerObs{make_shared<AlertScheduler>(m_alertStorage, m_alertRenderer, m_alertPastDueTimeLimit, m_metricRecorder)};
                vector<shared_ptr<TestAlert>> alertsToAdd;
                shared_ptr<TestAlert> alert1 = make_shared<TestAlert>(ALERT1_TOKEN, getFutureInstant(1));
                alertsToAdd.push_back(alert1);
                shared_ptr<TestAlert> alert2 = make_shared<TestAlert>(ALERT2_TOKEN, getFutureInstant(1));
                alertsToAdd.push_back(alert2);
                m_alertStorage->setAlerts(alertsToAdd);
                m_alertScheduler->initialize(alertSchedulerObs, m_settingsManager);
                m_alertScheduler->updateFocus(FocusState::BACKGROUND);
                AlertScheduler::AlertsContextInfo resultContextInfo = m_alertScheduler->getContextInfo();
                const unsigned int expectedRemainingScheduledAlerts = 2;
                const unsigned int expectedRemainingActiveAlerts = 1;
                ASSERT_EQ(resultContextInfo.scheduledAlerts.size(), expectedRemainingScheduledAlerts);
                ASSERT_EQ(resultContextInfo.activeAlerts.size(), expectedRemainingActiveAlerts);
            }
            TEST_F(AlertSchedulerTest, test_onLocalStop) {
                shared_ptr<TestAlert> alert = doSimpleTestSetup(true);
                m_alertScheduler->onLocalStop();
                ASSERT_EQ(alert->getState(), Alert::State::STOPPING);
                ASSERT_EQ(alert->getStopReason(), Alert::StopReason::LOCAL_STOP);
            }
            TEST_F(AlertSchedulerTest, test_clearData) {
                shared_ptr<TestAlert> alert = doSimpleTestSetup(true);
                EXPECT_CALL(*(m_alertStorage.get()), clearDatabase()).Times(1);
                m_alertScheduler->clearData();
                ASSERT_EQ(alert->getState(), Alert::State::STOPPING);
                ASSERT_EQ(alert->getStopReason(), Alert::StopReason::SHUTDOWN);
            }
            TEST_F(AlertSchedulerTest, test_clearDataLogout) {
                shared_ptr<TestAlert> alert = doSimpleTestSetup(true);
                EXPECT_CALL(*(m_alertStorage.get()), clearDatabase()).Times(1);
                m_alertScheduler->clearData(Alert::StopReason::LOG_OUT);
                ASSERT_EQ(alert->getState(), Alert::State::STOPPING);
                ASSERT_EQ(alert->getStopReason(), Alert::StopReason::LOG_OUT);
            }
            TEST_F(AlertSchedulerTest, test_shutdown) {
                doSimpleTestSetup(true);
                m_alertScheduler->shutdown();
                const unsigned int expectedRemainingAlerts = 0;
                ASSERT_EQ(m_alertScheduler->getContextInfo().scheduledAlerts.size(), expectedRemainingAlerts);
            }
            TEST_F(AlertSchedulerTest, test_onAlertStateChangeStartedInactiveAlert) {
                const string testReason = "stateStarted";
                auto testState = AlertScheduler::State::STARTED;
                doSimpleTestSetup(false, true);
                EXPECT_CALL(*(m_alertStorage.get()), modify(testing::_)).Times(0);
                m_alertScheduler->onAlertStateChange(ALERT1_TOKEN, ALERT_TYPE, testState, testReason);
            }
            TEST_F(AlertSchedulerTest, test_onAlertStateChangeStartedActiveAlert) {
                const string testReason = "stateStarted";
                auto testState = AlertScheduler::State::STARTED;
                doSimpleTestSetup(true, true);
                EXPECT_CALL(*(m_alertStorage.get()), modify(testing::_)).Times(1);
                m_alertScheduler->onAlertStateChange(ALERT1_TOKEN, ALERT_TYPE, testState, testReason);
                ASSERT_TRUE(m_testAlertObserver->waitFor(testState));
            }
            TEST_F(AlertSchedulerTest, test_onAlertStateChangeStopped) {
                const string testReason = "stateStopped";
                auto testState = AlertScheduler::State::STOPPED;
                doSimpleTestSetup(true, true);
                EXPECT_CALL(*(m_alertStorage.get()), erase(testing::_)).Times(1);
                m_alertScheduler->onAlertStateChange(ALERT1_TOKEN, ALERT_TYPE, testState, testReason);
                ASSERT_TRUE(m_testAlertObserver->waitFor(testState));
            }
            TEST_F(AlertSchedulerTest, test_onAlertStateChangeCompleted) {
                const string testReason = "stateCompleted";
                auto testState = AlertScheduler::State::COMPLETED;
                doSimpleTestSetup(true, true);
                EXPECT_CALL(*(m_alertStorage.get()), erase(testing::_)).Times(1);
                m_alertScheduler->onAlertStateChange(ALERT1_TOKEN, ALERT_TYPE, testState, testReason);
                ASSERT_TRUE(m_testAlertObserver->waitFor(testState));
            }
            TEST_F(AlertSchedulerTest, test_onAlertStateChangeSnoozed) {
                const string testReason = "stateSnoozed";
                auto testState = AlertScheduler::State::SNOOZED;
                doSimpleTestSetup(true, true);
                EXPECT_CALL(*(m_alertStorage.get()), modify(testing::_)).Times(1);
                m_alertScheduler->onAlertStateChange(ALERT1_TOKEN, ALERT_TYPE, testState, testReason);
                ASSERT_TRUE(m_testAlertObserver->waitFor(testState));
            }
            TEST_F(AlertSchedulerTest, test_onAlertStateChangeErrorActiveAlert) {
                const string testReason = "stateError";
                auto testState = AlertScheduler::State::ERROR;
                doSimpleTestSetup(true, true);
                EXPECT_CALL(*(m_alertStorage.get()), erase(testing::_)).Times(1);
                m_alertScheduler->onAlertStateChange(ALERT1_TOKEN, ALERT_TYPE, testState, testReason);
                ASSERT_TRUE(m_testAlertObserver->waitFor(testState));
            }
            TEST_F(AlertSchedulerTest, test_onAlertStateChangeErrorInactiveAlert) {
                const string testReason = "stateError";
                auto testState = AlertScheduler::State::ERROR;
                doSimpleTestSetup(false, true);
                EXPECT_CALL(*(m_alertStorage.get()), erase(testing::_)).Times(1);
                m_alertScheduler->onAlertStateChange(ALERT1_TOKEN, ALERT_TYPE, testState, testReason);
                ASSERT_TRUE(m_testAlertObserver->waitFor(testState));
            }
        }
    }
}