#include <chrono>
#include <future>
#include <memory>
#include <thread>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <json/document.h>
#include <json/en.h>
#include <json/stringbuffer.h>
#include <json/writer.h>
#include <sdkinterfaces/MockContextManager.h>
#include <json/JSONUtils.h>
#include <util/string/StringUtils.h>
#include "AudioActivityTracker.h"

namespace alexaClientSDK {
    namespace afml {
        namespace test {
            using namespace std;
            using namespace chrono;
            using namespace avsCommon;
            using namespace avs;
            using namespace rapidjson;
            using namespace sdkInterfaces;
            using namespace sdkInterfaces::test;
            using namespace utils;
            using namespace json;
            using namespace jsonUtils;
            using namespace testing;
            static milliseconds MY_WAIT_TIMEOUT(1000);
            static const std::string NAMESPACE_AUDIO_ACTIVITY_TRACKER("AudioActivityTracker");
            static const NamespaceAndName NAMESPACE_AND_NAME_STATE{NAMESPACE_AUDIO_ACTIVITY_TRACKER, "ActivityState"};
            static const unsigned int PROVIDE_STATE_TOKEN_TEST{1};
            static const std::string DIALOG_CHANNEL_NAME{"Dialog"};
            static unsigned int DIALOG_CHANNEL_PRIORITY{100};
            static const std::string DIALOG_INTERFACE_NAME{"SpeechSynthesizer"};
            static const std::string AIP_INTERFACE_NAME{"SpeechRecognizer"};
            static const std::string CONTENT_CHANNEL_NAME{"Content"};
            static const std::string CONTENT_INTERFACE_NAME{"AudioPlayer"};
            static unsigned int CONTENT_CHANNEL_PRIORITY{300};
            static const milliseconds SHORT_TIMEOUT_MS = milliseconds(5);
            class MockChannelObserver : public ChannelObserverInterface {
            public:
                void onFocusChanged(FocusState state, MixingBehavior behavior) override {}
            };
            class AudioActivityTrackerTest : public Test {
            public:
                AudioActivityTrackerTest();
                void SetUp() override;
                void TearDown() override;
                shared_ptr<AudioActivityTracker> m_audioActivityTracker;
                shared_ptr<MockContextManager> m_mockContextManager;
                shared_ptr<Channel> m_dialogChannel;
                shared_ptr<Channel> m_contentChannel;
                void verifyState(const std::string& providedState, const vector<Channel::State>& channels);
                void provideUpdate(const vector<Channel::State>& channels);
                SetStateResult wakeOnSetState();
                promise<void> m_wakeSetStatePromise;
                future<void> m_wakeSetStateFuture;
            };
            AudioActivityTrackerTest::AudioActivityTrackerTest() : m_wakeSetStatePromise{}, m_wakeSetStateFuture{m_wakeSetStatePromise.get_future()} {}
            void AudioActivityTrackerTest::SetUp() {
                m_mockContextManager = make_shared<NiceMock<MockContextManager>>();
                m_audioActivityTracker = AudioActivityTracker::create(m_mockContextManager);
                ASSERT_TRUE(m_mockContextManager != nullptr);
                m_dialogChannel = make_shared<Channel>(DIALOG_CHANNEL_NAME, DIALOG_CHANNEL_PRIORITY);
                ASSERT_TRUE(m_dialogChannel != nullptr);
                m_contentChannel = make_shared<Channel>(CONTENT_CHANNEL_NAME, CONTENT_CHANNEL_PRIORITY);
                ASSERT_TRUE(m_contentChannel != nullptr);
            }
            void AudioActivityTrackerTest::TearDown() {
                m_audioActivityTracker->shutdown();
            }
            void AudioActivityTrackerTest::verifyState(const std::string& providedState, const vector<Channel::State>& channels) {
                Document jsonContent;
                jsonContent = jsonContent.Parse(providedState.data());
                rapidjson::Value v(providedState.data(), providedState.length());
                for (auto& channel : channels) {
                    Value::ConstMemberIterator channelNode;
                    auto channelName = string::stringToLowerCase(channel.name);
                    ASSERT_TRUE(jsonUtils::findNode(v, channelName, &channelNode));
                    std::string interfaceName;
                    std::string expectedInterfaceName = channel.interfaceName;
                    if (AIP_INTERFACE_NAME == expectedInterfaceName) expectedInterfaceName = DIALOG_INTERFACE_NAME;
                    ASSERT_TRUE(jsonUtils::retrieveValue(channelNode->value, "interface", &interfaceName));
                    ASSERT_EQ(interfaceName, expectedInterfaceName);
                    int64_t idleTime;
                    bool isChannelActive = FocusState::NONE != channel.focusState;
                    if (AIP_INTERFACE_NAME == channel.interfaceName) isChannelActive = false;
                    ASSERT_TRUE(jsonUtils::retrieveValue(channelNode->value, "idleTimeInMilliseconds", &idleTime));
                    if (isChannelActive) { ASSERT_EQ(idleTime, 0); }
                    else { ASSERT_NE(idleTime, 0); }
                }
            }
            void AudioActivityTrackerTest::provideUpdate(const vector<Channel::State>& channels) {
                EXPECT_CALL(
                    *(m_mockContextManager.get()),
                    setState(NAMESPACE_AND_NAME_STATE, _, StateRefreshPolicy::SOMETIMES, PROVIDE_STATE_TOKEN_TEST))
                    .Times(1).WillOnce(DoAll(Invoke([this, &channels](const NamespaceAndName& namespaceAndName, const std::string& jsonState,
                                       const StateRefreshPolicy& refreshPolicy, const unsigned int stateRequestToken) { verifyState(jsonState, channels); }),
                                    InvokeWithoutArgs(this, &AudioActivityTrackerTest::wakeOnSetState)));
                m_audioActivityTracker->notifyOfActivityUpdates(channels);
                this_thread::sleep_for(SHORT_TIMEOUT_MS);
                m_audioActivityTracker->provideState(NAMESPACE_AND_NAME_STATE, PROVIDE_STATE_TOKEN_TEST);
                ASSERT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
            }
            SetStateResult AudioActivityTrackerTest::wakeOnSetState() {
                m_wakeSetStatePromise.set_value();
                return SetStateResult::SUCCESS;
            }
            TEST_F(AudioActivityTrackerTest, test_noActivityUpdate) {
                EXPECT_CALL(*(m_mockContextManager.get()),setState(NAMESPACE_AND_NAME_STATE, "", StateRefreshPolicy::SOMETIMES, PROVIDE_STATE_TOKEN_TEST))
                    .Times(1).WillOnce(InvokeWithoutArgs(this, &AudioActivityTrackerTest::wakeOnSetState));
                m_audioActivityTracker->provideState(NAMESPACE_AND_NAME_STATE, PROVIDE_STATE_TOKEN_TEST);
                ASSERT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
            }
            TEST_F(AudioActivityTrackerTest, test_emptyActivityUpdate) {
                const vector<Channel::State> channels;
                EXPECT_CALL(*(m_mockContextManager.get()),setState(NAMESPACE_AND_NAME_STATE, "", StateRefreshPolicy::SOMETIMES, PROVIDE_STATE_TOKEN_TEST))
                    .Times(1).WillOnce(InvokeWithoutArgs(this, &AudioActivityTrackerTest::wakeOnSetState));
                m_audioActivityTracker->notifyOfActivityUpdates(channels);
                m_audioActivityTracker->provideState(NAMESPACE_AND_NAME_STATE, PROVIDE_STATE_TOKEN_TEST);
                ASSERT_TRUE(std::future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
            }
            TEST_F(AudioActivityTrackerTest, test_oneActiveChannel) {
                vector<Channel::State> channels;
                m_dialogChannel->setFocus(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                channels.push_back(m_dialogChannel->getState());
                provideUpdate(channels);
            }
            TEST_F(AudioActivityTrackerTest, test_oneActiveChannelWithAIPAsInterface) {
                vector<Channel::State> channels;
                auto mockObserver = make_shared<MockChannelObserver>();
                auto aipActivity = FocusManagerInterface::Activity::create(AIP_INTERFACE_NAME, mockObserver);
                m_dialogChannel->setPrimaryActivity(aipActivity);
                m_dialogChannel->setFocus(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                channels.push_back(m_dialogChannel->getState());
                EXPECT_CALL(*(m_mockContextManager.get()),setState(NAMESPACE_AND_NAME_STATE, "", StateRefreshPolicy::SOMETIMES, PROVIDE_STATE_TOKEN_TEST))
                    .Times(1).WillOnce(InvokeWithoutArgs(this, &AudioActivityTrackerTest::wakeOnSetState));
                m_audioActivityTracker->notifyOfActivityUpdates(channels);
                this_thread::sleep_for(SHORT_TIMEOUT_MS);
                m_audioActivityTracker->provideState(NAMESPACE_AND_NAME_STATE, PROVIDE_STATE_TOKEN_TEST);
                ASSERT_TRUE(std::future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
            }
            TEST_F(AudioActivityTrackerTest, test_oneActiveChannelWithDefaultAndAIPAsInterfaces) {
                vector<Channel::State> channels;
                m_dialogChannel->setFocus(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                channels.push_back(m_dialogChannel->getState());
                provideUpdate(channels);
            }
            TEST_F(AudioActivityTrackerTest, test_twoActiveChannels) {
                vector<Channel::State> channels;
                m_dialogChannel->setFocus(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                m_contentChannel->setFocus(FocusState::BACKGROUND, MixingBehavior::MUST_PAUSE);
                channels.push_back(m_dialogChannel->getState());
                channels.push_back(m_contentChannel->getState());
                provideUpdate(channels);
            }
            TEST_F(AudioActivityTrackerTest, test_oneActiveOneIdleChannels) {
                vector<Channel::State> channels;
                m_dialogChannel->setFocus(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                m_contentChannel->setFocus(FocusState::BACKGROUND, MixingBehavior::MUST_PAUSE);
                m_dialogChannel->setFocus(FocusState::NONE, MixingBehavior::MUST_STOP);
                m_contentChannel->setFocus(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                channels.push_back(m_dialogChannel->getState());
                channels.push_back(m_contentChannel->getState());
                provideUpdate(channels);
            }
        }
    }
}