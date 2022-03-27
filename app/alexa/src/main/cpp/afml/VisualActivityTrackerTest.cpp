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
#include <json/JSONUtils.h>
#include <sdkinterfaces/FocusManagerInterface.h>
#include <sdkinterfaces/MockContextManager.h>
#include "VisualActivityTracker.h"

namespace alexaClientSDK {
    namespace afml {
        namespace test {
            using namespace std;
            using namespace chrono;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace sdkInterfaces::test;
            using namespace utils;
            using namespace json;
            using namespace testing;
            using namespace rapidjson;
            static milliseconds MY_WAIT_TIMEOUT(1000);
            static const string NAMESPACE_AUDIO_ACTIVITY_TRACKER("VisualActivityTracker");
            static const NamespaceAndName NAMESPACE_AND_NAME_STATE{NAMESPACE_AUDIO_ACTIVITY_TRACKER, "ActivityState"};
            static const unsigned int PROVIDE_STATE_TOKEN_TEST{1};
            static const string VISUAL_CHANNEL_NAME{"Visual"};
            static const string VISUAL_INTERFACE_NAME{"TempateRuntime"};
            static unsigned int VISUAL_CHANNEL_PRIORITY{100};
            static const string INVALID_CHANNEL_NAME{"Invalid"};
            static unsigned int INVALID_CHANNEL_PRIORITY{300};
            static const milliseconds SHORT_TIMEOUT_MS = std::chrono::milliseconds(5);
            class VisualActivityTrackerTest : public Test {
            public:
                VisualActivityTrackerTest();
                void SetUp() override;
                void TearDown() override;
                shared_ptr<VisualActivityTracker> m_VisualActivityTracker;
                shared_ptr<MockContextManager> m_mockContextManager;
                shared_ptr<Channel> m_visualChannel;
                void verifyState(const string& providedState, const vector<Channel::State>& channels);
                void provideUpdate(const vector<Channel::State>& channels);
                SetStateResult wakeOnSetState();
                promise<void> m_wakeSetStatePromise;
                future<void> m_wakeSetStateFuture;
            };
            VisualActivityTrackerTest::VisualActivityTrackerTest() : m_wakeSetStateFuture{m_wakeSetStatePromise.get_future()} {}
            void VisualActivityTrackerTest::SetUp() {
                m_mockContextManager = make_shared<NiceMock<MockContextManager>>();
                m_VisualActivityTracker = VisualActivityTracker::create(m_mockContextManager);
                ASSERT_TRUE(m_mockContextManager != nullptr);
                m_visualChannel = make_shared<Channel>(VISUAL_CHANNEL_NAME, VISUAL_CHANNEL_PRIORITY);
                ASSERT_TRUE(m_visualChannel != nullptr);
            }
            void VisualActivityTrackerTest::TearDown() {
                m_VisualActivityTracker->shutdown();
            }
            void VisualActivityTrackerTest::verifyState(const string& providedState, const vector<Channel::State>& channels) {
                Document jsonContent;
                jsonContent.Parse(providedState.data());
                if (channels.size() == 0) {
                    ASSERT_TRUE(providedState.empty());
                    return;
                }
                for (auto& channel : channels) {
                    if (FocusManagerInterface::VISUAL_CHANNEL_NAME != channel.name) {
                        ASSERT_TRUE(providedState.empty());
                        return;
                    }
                }
                const auto& channel = channels.back();
                if (FocusState::NONE == channel.focusState) {
                    ASSERT_TRUE(providedState.empty());
                    return;
                }
                rapidjson::Value::ConstMemberIterator focusNode;
                rapidjson::Value node(providedState.data(), providedState.length());
                ASSERT_TRUE(jsonUtils::findNode(node, "focused", &focusNode));
                std::string interfaceName;
                ASSERT_TRUE(jsonUtils::retrieveValue(focusNode->value, "interface", &interfaceName));
                ASSERT_EQ(interfaceName, channel.interfaceName);
            }
            void VisualActivityTrackerTest::provideUpdate(const std::vector<Channel::State>& channels) {
                EXPECT_CALL(*(m_mockContextManager.get()),setState(NAMESPACE_AND_NAME_STATE, _, StateRefreshPolicy::SOMETIMES, PROVIDE_STATE_TOKEN_TEST))
                    .Times(1).WillOnce(DoAll(Invoke([this, &channels](const NamespaceAndName& namespaceAndName, const string& jsonState,
                                       const StateRefreshPolicy& refreshPolicy, const unsigned int stateRequestToken) { verifyState(jsonState, channels); }),
                                   InvokeWithoutArgs(this, &VisualActivityTrackerTest::wakeOnSetState)));
                m_VisualActivityTracker->notifyOfActivityUpdates(channels);
                this_thread::sleep_for(SHORT_TIMEOUT_MS);
                m_VisualActivityTracker->provideState(NAMESPACE_AND_NAME_STATE, PROVIDE_STATE_TOKEN_TEST);
                ASSERT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
            }
            SetStateResult VisualActivityTrackerTest::wakeOnSetState() {
                m_wakeSetStatePromise.set_value();
                return SetStateResult::SUCCESS;
            }
            TEST_F(VisualActivityTrackerTest, test_noActivityUpdate) {
                EXPECT_CALL(*(m_mockContextManager.get()),setState(NAMESPACE_AND_NAME_STATE, "", StateRefreshPolicy::SOMETIMES, PROVIDE_STATE_TOKEN_TEST))
                    .Times(1).WillOnce(InvokeWithoutArgs(this, &VisualActivityTrackerTest::wakeOnSetState));
                m_VisualActivityTracker->provideState(NAMESPACE_AND_NAME_STATE, PROVIDE_STATE_TOKEN_TEST);
                ASSERT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
            }
            TEST_F(VisualActivityTrackerTest, test_emptyActivityUpdate) {
                vector<Channel::State> channels;
                provideUpdate(channels);
            }
            TEST_F(VisualActivityTrackerTest, test_oneIdleChannel) {
                vector<Channel::State> channels;
                m_visualChannel->setFocus(FocusState::NONE, MixingBehavior::MUST_STOP);
                channels.push_back(m_visualChannel->getState());
                provideUpdate(channels);
            }
            TEST_F(VisualActivityTrackerTest, test_oneActiveChannel) {
                vector<Channel::State> channels;
                m_visualChannel->setFocus(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                channels.push_back(m_visualChannel->getState());
                provideUpdate(channels);
            }
            TEST_F(VisualActivityTrackerTest, test_invalidChannelActivityUpdate) {
                vector<Channel::State> channels;
                auto invalidChannel = make_shared<Channel>(INVALID_CHANNEL_NAME, INVALID_CHANNEL_PRIORITY);
                m_visualChannel->setFocus(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                channels.push_back(m_visualChannel->getState());
                channels.push_back(invalidChannel->getState());
                provideUpdate(channels);
            }
            TEST_F(VisualActivityTrackerTest, test_validChannelTwoActivityUpdates) {
                vector<Channel::State> channels;
                m_visualChannel->setFocus(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                channels.push_back(m_visualChannel->getState());
                m_visualChannel->setFocus(FocusState::BACKGROUND, MixingBehavior::MUST_PAUSE);
                channels.push_back(m_visualChannel->getState());
                provideUpdate(channels);
            }
        }
    }
}