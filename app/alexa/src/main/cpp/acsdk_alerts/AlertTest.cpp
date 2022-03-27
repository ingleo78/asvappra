#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <timing/TimeUtils.h>
#include "Alert.h"

namespace alexaClientSDK {
    namespace acsdkAlerts {
        namespace test {
            using namespace std;
            using namespace chrono;
            using namespace avsCommon;
            using namespace avs;
            using namespace utils;
            using namespace timing;
            using namespace acsdkAlerts;
            using namespace renderer;
            using namespace testing;
            using namespace rapidjson;
            static const string TOKEN_TEST("Token_Test");
            static const size_t NUM_ASSETS{2};
            static const string ASSET_ID1("assetId1");
            static const string ASSET_ID2("assetId2");
            static const string ASSET_PLAY_ORDER("[\"assetId1\",\"assetId2\"]");
            static const string BACKGROUND_ALERT_ASSET("assetId2");
            static const string ASSET_URL1("cid:Test1");
            static const string ASSET_URL2("cid:Test2");
            static const string ALERT_TYPE("MOCK_ALERT_TYPE");
            static const string SCHED_TIME{"2030-01-01T12:34:56+0000"};
            static const string INVALID_FORMAT_SCHED_TIME{"abc"};
            static const string TEST_DATE_IN_THE_PAST{"2000-02-02T12:56:34+0000"};
            static const string TEST_DATE_IN_THE_FUTURE{"2030-02-02T12:56:34+0000"};
            static const long LOOP_COUNT{2};
            static const long LOOP_PAUSE_MS{300};
            static const string DEFAULT_AUDIO{"default audio"};
            static const string SHORT_AUDIO{"short audio"};
            class MockAlert : public Alert {
            public:
                MockAlert() : Alert(defaultAudioFactory, shortAudioFactory, nullptr) {}
                string getTypeName() const override;
            private:
                static pair<unique_ptr<istream>, const MediaType> defaultAudioFactory() {
                    return pair<unique_ptr<istream>, const MediaType>(unique_ptr<stringstream>(new stringstream(DEFAULT_AUDIO)),MediaType::MPEG);
                }
                static pair<unique_ptr<istream>, const MediaType> shortAudioFactory() {
                    return pair<unique_ptr<istream>, const MediaType>(unique_ptr<stringstream>(new stringstream(SHORT_AUDIO)), MediaType::MPEG);
                }
            };
            string MockAlert::getTypeName() const {
                return ALERT_TYPE;
            }
            class MockRenderer : public RendererInterface {
            public:
                MOCK_METHOD7(start, void(shared_ptr<RendererObserverInterface> observer, function<pair<unique_ptr<istream>, const MediaType>()> audioFactory,
                             bool alarmVolumeRampEnabled, const vector<string>& urls, int loopCount, milliseconds loopPause, bool startWithPause));
                MOCK_METHOD0(stop, void());
            };
            class AlertTest : public Test {
            public:
                AlertTest();
            protected:
                shared_ptr<MockAlert> m_alert;
                shared_ptr<MockRenderer> m_renderer;
            };
            AlertTest::AlertTest() : m_alert{make_shared<MockAlert>()}, m_renderer{make_shared<MockRenderer>()} {
                m_alert->setRenderer(m_renderer);
            }
            const string getPayloadJson(bool inclToken, bool inclSchedTime, const string& schedTime) {
                string tokenJson;
                if (inclToken) tokenJson = "\"token\": \"" + TOKEN_TEST + "\",";
                string schedTimeJson;
                if (inclSchedTime) schedTimeJson = "\"scheduledTime\": \"" + schedTime + "\",";
                const string payloadJson = "{" + tokenJson + "\"type\": \"" + ALERT_TYPE + "\"," + schedTimeJson + "\"assets\": [{\"assetId\": \"" + ASSET_ID1 +
                                           "\",\"url\": \"" + ASSET_URL1 + "\"},{\"assetId\": \"" + ASSET_ID2 + "\",\"url\": \"" + ASSET_URL2 + "\"}],"
                                           "\"assetPlayOrder\": "  + ASSET_PLAY_ORDER + ",\"backgroundAlertAsset\": \"" + BACKGROUND_ALERT_ASSET + "\","
                                           "\"loopCount\": " + std::to_string(LOOP_COUNT) + ",\"loopPauseInMilliSeconds\": " + std::to_string(LOOP_PAUSE_MS) + "}";
                return payloadJson;
            }
            TEST_F(AlertTest, test_defaultAudio) {
                ostringstream oss;
                auto audioStream = get<0>(m_alert->getDefaultAudioFactory()());
                oss << audioStream->rdbuf();
                ASSERT_EQ(DEFAULT_AUDIO, oss.str());
            }
            TEST_F(AlertTest, test_defaultShortAudio) {
                ostringstream oss;
                auto audioStream = get<0>(m_alert->getShortAudioFactory()());
                oss << audioStream->rdbuf();
                ASSERT_EQ(SHORT_AUDIO, oss.str());
            }
            TEST_F(AlertTest, test_parseFromJsonHappyCase) {
                string errorMessage;
                const string payloadJson = getPayloadJson(true, true, SCHED_TIME);
                Document payload;
                payload.Parse(payloadJson.data());
                Alert::ParseFromJsonStatus resultStatus = m_alert->parseFromJson(payload, &errorMessage);
                Alert::AssetConfiguration assetConfiguration = m_alert->getAssetConfiguration();
                ASSERT_EQ(resultStatus, Alert::ParseFromJsonStatus::OK);
                ASSERT_EQ(m_alert->getToken(), TOKEN_TEST);
                ASSERT_EQ(m_alert->getScheduledTime_ISO_8601(), SCHED_TIME);
                ASSERT_EQ(m_alert->getBackgroundAssetId(), BACKGROUND_ALERT_ASSET);
                ASSERT_EQ(m_alert->getLoopCount(), LOOP_COUNT);
                ASSERT_EQ(m_alert->getLoopPause(), std::chrono::milliseconds{LOOP_PAUSE_MS});
                vector<string> assetPlayOrderItems;
                assetPlayOrderItems.push_back(ASSET_ID1);
                assetPlayOrderItems.push_back(ASSET_ID2);
                ASSERT_EQ(assetConfiguration.assetPlayOrderItems, assetPlayOrderItems);
                unordered_map<string, Alert::Asset> assetsMap = assetConfiguration.assets;
                ASSERT_EQ(assetsMap.size(), NUM_ASSETS);
                ASSERT_EQ(assetsMap[ASSET_ID1].id, ASSET_ID1);
                ASSERT_EQ(assetsMap[ASSET_ID1].url, ASSET_URL1);
                ASSERT_EQ(assetsMap[ASSET_ID2].id, ASSET_ID2);
                ASSERT_EQ(assetsMap[ASSET_ID2].url, ASSET_URL2);
            }
            TEST_F(AlertTest, test_parseFromJsonMissingToken) {
                string errorMessage;
                const string payloadJson = getPayloadJson(false, true, SCHED_TIME);
                Document payload;
                payload.Parse(payloadJson.data());
                Alert::ParseFromJsonStatus resultStatus = m_alert->parseFromJson(payload, &errorMessage);
                ASSERT_EQ(resultStatus, Alert::ParseFromJsonStatus::MISSING_REQUIRED_PROPERTY);
            }
            TEST_F(AlertTest, test_parseFromJsonMissingSchedTime) {
                string errorMessage;
                const string payloadJson = getPayloadJson(true, false, SCHED_TIME);
                Document payload;
                payload.Parse(payloadJson.data());
                Alert::ParseFromJsonStatus resultStatus = m_alert->parseFromJson(payload, &errorMessage);
                ASSERT_EQ(resultStatus, Alert::ParseFromJsonStatus::MISSING_REQUIRED_PROPERTY);
            }
            TEST_F(AlertTest, test_parseFromJsonBadSchedTimeFormat) {
                const string schedTime{INVALID_FORMAT_SCHED_TIME};
                string errorMessage;
                const string payloadJson = getPayloadJson(true, true, schedTime);
                Document payload;
                payload.Parse(payloadJson.data());
                Alert::ParseFromJsonStatus resultStatus = m_alert->parseFromJson(payload, &errorMessage);
                ASSERT_EQ(resultStatus, Alert::ParseFromJsonStatus::INVALID_VALUE);
            }
            TEST_F(AlertTest, test_setStateActive) {
                m_alert->reset();
                ASSERT_EQ(m_alert->getState(), Alert::State::SET);
                m_alert->setStateActive();
                ASSERT_NE(m_alert->getState(), Alert::State::ACTIVE);
                m_alert->activate();
                ASSERT_EQ(m_alert->getState(), Alert::State::ACTIVATING);
                m_alert->setStateActive();
                ASSERT_EQ(m_alert->getState(), Alert::State::ACTIVE);
            }
            TEST_F(AlertTest, test_deactivate) {
                Alert::StopReason stopReason = Alert::StopReason::AVS_STOP;
                m_alert->deactivate(stopReason);
                ASSERT_EQ(m_alert->getState(), Alert::State::STOPPING);
                ASSERT_EQ(m_alert->getStopReason(), stopReason);
            }
            TEST_F(AlertTest, test_setTimeISO8601) {
                TimeUtils timeUtils;
                string schedTime{"2030-02-02T12:56:34+0000"};
                Alert::DynamicData dynamicData;
                m_alert->getAlertData(nullptr, &dynamicData);
                ASSERT_TRUE(dynamicData.timePoint.setTime_ISO_8601(schedTime));
                m_alert->setAlertData(nullptr, &dynamicData);
                int64_t unixTime = 0;
                timeUtils.convert8601TimeStringToUnix(schedTime, &unixTime);
                ASSERT_EQ(m_alert->getScheduledTime_ISO_8601(), schedTime);
                ASSERT_EQ(m_alert->getScheduledTime_Unix(), unixTime);
            }
            TEST_F(AlertTest, test_updateScheduleActiveFailed) {
                m_alert->activate();
                m_alert->setStateActive();
                ASSERT_EQ(m_alert->getState(), Alert::State::ACTIVE);
                auto oldScheduledTime = m_alert->getScheduledTime_ISO_8601();
                ASSERT_FALSE(m_alert->updateScheduledTime("2030-02-02T12:56:34+0000"));
                ASSERT_EQ(m_alert->getState(), Alert::State::ACTIVE);
                ASSERT_EQ(m_alert->getScheduledTime_ISO_8601(), oldScheduledTime);
            }
            TEST_F(AlertTest, test_updateScheduleBadTime) {
                auto oldScheduledTime = m_alert->getScheduledTime_ISO_8601();
                ASSERT_FALSE(m_alert->updateScheduledTime(INVALID_FORMAT_SCHED_TIME));
                ASSERT_EQ(m_alert->getScheduledTime_ISO_8601(), oldScheduledTime);
            }
            TEST_F(AlertTest, test_updateScheduleHappyCase) {
                m_alert->reset();
                ASSERT_TRUE(m_alert->updateScheduledTime("2030-02-02T12:56:34+0000"));
                ASSERT_EQ(m_alert->getState(), Alert::State::SET);
            }
            TEST_F(AlertTest, test_snoozeBadTime) {
                m_alert->reset();
                ASSERT_FALSE(m_alert->snooze(INVALID_FORMAT_SCHED_TIME));
                ASSERT_NE(m_alert->getState(), Alert::State::SNOOZING);
            }
            TEST_F(AlertTest, test_snoozeHappyCase) {
                m_alert->reset();
                ASSERT_TRUE(m_alert->snooze("2030-02-02T12:56:34+0000"));
                ASSERT_EQ(m_alert->getState(), Alert::State::SNOOZING);
            }
            TEST_F(AlertTest, test_setLoopCountNegative) {
                int loopCount = -1;
                Alert::DynamicData dynamicData;
                m_alert->getAlertData(nullptr, &dynamicData);
                dynamicData.loopCount = loopCount;
                m_alert->setAlertData(nullptr, &dynamicData);
                ASSERT_NE(m_alert->getLoopCount(), loopCount);
            }
            TEST_F(AlertTest, test_setLoopCountHappyCase) {
                int loopCount = 3;
                Alert::DynamicData dynamicData;
                m_alert->getAlertData(nullptr, &dynamicData);
                dynamicData.loopCount = loopCount;
                m_alert->setAlertData(nullptr, &dynamicData);
                ASSERT_EQ(m_alert->getLoopCount(), loopCount);
            }
            TEST_F(AlertTest, test_setLoopPause) {
                milliseconds loopPause{900};
                Alert::DynamicData dynamicData;
                m_alert->getAlertData(nullptr, &dynamicData);
                dynamicData.assetConfiguration.loopPause = loopPause;
                m_alert->setAlertData(nullptr, &dynamicData);
                ASSERT_EQ(m_alert->getLoopPause(), loopPause);
            }
            TEST_F(AlertTest, test_setBackgroundAssetId) {
                unordered_map<string, Alert::Asset> assets;
                assets["testAssetId"] = Alert::Asset("testAssetId", "http://test.com/a");
                string backgroundAssetId{"testAssetId"};
                Alert::DynamicData dynamicData;
                m_alert->getAlertData(nullptr, &dynamicData);
                dynamicData.assetConfiguration.backgroundAssetId = backgroundAssetId;
                dynamicData.assetConfiguration.assets = assets;
                m_alert->setAlertData(nullptr, &dynamicData);
                ASSERT_EQ(m_alert->getBackgroundAssetId(), backgroundAssetId);
            }
            TEST_F(AlertTest, DISABLED_test_isPastDue) {
                Alert::DynamicData dynamicData;
                TimeUtils timeUtils;
                int64_t currentUnixTime = 0;
                timeUtils.getCurrentUnixTime(&currentUnixTime);
                m_alert->getAlertData(nullptr, &dynamicData);
                ASSERT_TRUE(dynamicData.timePoint.setTime_ISO_8601(TEST_DATE_IN_THE_FUTURE));
                m_alert->setAlertData(nullptr, &dynamicData);
                ASSERT_FALSE(m_alert->isPastDue(currentUnixTime, std::chrono::seconds{1}));
                m_alert->getAlertData(nullptr, &dynamicData);
                ASSERT_TRUE(dynamicData.timePoint.setTime_ISO_8601(TEST_DATE_IN_THE_PAST));
                m_alert->setAlertData(nullptr, &dynamicData);
                ASSERT_TRUE(m_alert->isPastDue(currentUnixTime, std::chrono::seconds{1}));
            }
            TEST_F(AlertTest, test_stateToString) {
                ASSERT_EQ(m_alert->stateToString(Alert::State::UNSET), "UNSET");
                ASSERT_EQ(m_alert->stateToString(Alert::State::SET), "SET");
                ASSERT_EQ(m_alert->stateToString(Alert::State::READY), "READY");
                ASSERT_EQ(m_alert->stateToString(Alert::State::ACTIVATING), "ACTIVATING");
                ASSERT_EQ(m_alert->stateToString(Alert::State::ACTIVE), "ACTIVE");
                ASSERT_EQ(m_alert->stateToString(Alert::State::SNOOZING), "SNOOZING");
                ASSERT_EQ(m_alert->stateToString(Alert::State::SNOOZED), "SNOOZED");
                ASSERT_EQ(m_alert->stateToString(Alert::State::STOPPING), "STOPPING");
                ASSERT_EQ(m_alert->stateToString(Alert::State::STOPPED), "STOPPED");
                ASSERT_EQ(m_alert->stateToString(Alert::State::COMPLETED), "COMPLETED");
            }
            TEST_F(AlertTest, test_stopReasonToString) {
                ASSERT_EQ(m_alert->stopReasonToString(Alert::StopReason::UNSET), "UNSET");
                ASSERT_EQ(m_alert->stopReasonToString(Alert::StopReason::AVS_STOP), "AVS_STOP");
                ASSERT_EQ(m_alert->stopReasonToString(Alert::StopReason::LOCAL_STOP), "LOCAL_STOP");
                ASSERT_EQ(m_alert->stopReasonToString(Alert::StopReason::SHUTDOWN), "SHUTDOWN");
            }
            TEST_F(AlertTest, test_parseFromJsonStatusToString) {
                ASSERT_EQ(m_alert->parseFromJsonStatusToString(Alert::ParseFromJsonStatus::OK), "OK");
                ASSERT_EQ(m_alert->parseFromJsonStatusToString(Alert::ParseFromJsonStatus::MISSING_REQUIRED_PROPERTY),"MISSING_REQUIRED_PROPERTY");
                ASSERT_EQ(m_alert->parseFromJsonStatusToString(Alert::ParseFromJsonStatus::INVALID_VALUE), "INVALID_VALUE");
            }
            TEST_F(AlertTest, test_hasAssetHappy) {
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
                Alert::StaticData d;
                d.token = "aaa";
                d.dbId = 1;
                Alert::DynamicData e;
                e.assetConfiguration = c;
                ASSERT_TRUE(m_alert->setAlertData(&d, &e));
            }
            TEST_F(AlertTest, test_hasAssetBgAssetIdNotFoundOnAssets) {
                unordered_map<string, Alert::Asset> assets;
                assets["A"] = Alert::Asset("A", "http://test.com/a");
                assets["B"] = Alert::Asset("B", "http://test.com/a");
                vector<string> playOrderItems;
                playOrderItems.push_back("A");
                playOrderItems.push_back("B");
                Alert::AssetConfiguration c;
                c.assets = assets;
                c.assetPlayOrderItems = playOrderItems;
                c.backgroundAssetId = "C";
                c.loopPause = std::chrono::milliseconds(100);
                Alert::StaticData d;
                d.token = "aaa";
                d.dbId = 1;
                Alert::DynamicData e;
                e.assetConfiguration = c;
                ASSERT_FALSE(m_alert->setAlertData(&d, &e));
            }
            TEST_F(AlertTest, test_hasAssetOrderItemNotFoundOnAssets) {
                unordered_map<string, Alert::Asset> assets;
                assets["A"] = Alert::Asset("A", "http://test.com/a");
                assets["B"] = Alert::Asset("B", "http://test.com/a");
                vector<string> playOrderItems;
                playOrderItems.push_back("A");
                playOrderItems.push_back("B");
                playOrderItems.push_back("C");
                Alert::AssetConfiguration c;
                c.assets = assets;
                c.assetPlayOrderItems = playOrderItems;
                c.backgroundAssetId = "A";
                c.loopPause = milliseconds(100);
                Alert::StaticData d;
                d.token = "aaa";
                d.dbId = 1;
                Alert::DynamicData e;
                e.assetConfiguration = c;
                ASSERT_FALSE(m_alert->setAlertData(&d, &e));
            }
            TEST_F(AlertTest, test_focusChangeDuringActivation) {
                m_alert->reset();
                ASSERT_EQ(m_alert->getState(), Alert::State::SET);
                EXPECT_CALL(*(m_renderer.get()), start(_, _, _, _, _, _, _)).WillRepeatedly(Return());
                EXPECT_CALL(*(m_renderer.get()), stop()).WillOnce(Return());
                m_alert->setFocusState(avsCommon::avs::FocusState::BACKGROUND);
                m_alert->activate();
                ASSERT_EQ(m_alert->getState(), Alert::State::ACTIVATING);
                m_alert->setFocusState(avsCommon::avs::FocusState::FOREGROUND);
                m_alert->onRendererStateChange(RendererObserverInterface::State::STARTED, "started");
            }
        }
    }
}