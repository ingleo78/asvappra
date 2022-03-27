#include <memory>
#include <gtest/gtest.h>
#include <avs/FocusState.h>
#include <sdkinterfaces/MockFocusManagerObserver.h>
#include <configuration/ConfigurationNode.h>
#include <interrupt_model/InterruptModel.h>
#include "FocusManager.h"

namespace alexaClientSDK {
    namespace afml {
        namespace test {
            using namespace std;
            using namespace chrono;
            using namespace avsCommon;
            using namespace sdkInterfaces;
            using namespace sdkInterfaces::test;
            using namespace avs;
            using namespace utils;
            using namespace configuration;
            using namespace testing;
            using JSONStream = vector<shared_ptr<istream>>;
            static const auto SHORT_TIMEOUT = milliseconds(50);
            static const auto DEFAULT_TIMEOUT = seconds(15);
            static const auto NO_ACTIVITY_UPDATE_TIMEOUT = milliseconds(250);
            static const string DIALOG_CHANNEL_NAME = "Dialog";
            static const string ALERTS_CHANNEL_NAME = "Alert";
            static const string CONTENT_CHANNEL_NAME = "Content";
            static const string INCORRECT_CHANNEL_NAME = "aksdjfl;aksdjfl;akdsjf";
            static const string VIRTUAL_CHANNEL_NAME = "VirtualChannel";
            static const unsigned int VIRTUAL_CHANNEL_PRIORITY = 25;
            static const unsigned int DIALOG_CHANNEL_PRIORITY = 10;
            static const unsigned int ALERTS_CHANNEL_PRIORITY = 20;
            static const unsigned int CONTENT_CHANNEL_PRIORITY = 30;
            static const string DIALOG_INTERFACE_NAME = "dialog";
            static const string ALERTS_INTERFACE_NAME = "alerts";
            static const string CONTENT_INTERFACE_NAME = "content";
            static const string DIFFERENT_CONTENT_INTERFACE_NAME = "different content";
            static const string DIFFERENT_DIALOG_INTERFACE_NAME = "different dialog";
            static const string VIRTUAL_INTERFACE_NAME = "virtual";
            static const string INTERRUPT_MODEL_ROOT_KEY = "interruptModel";
            static const string INTERRUPT_MODEL_CONFIG_JSON = R"({
            "interruptModel" : {
                            "Dialog" : {
                            },
                            "Communications" : {
                                "contentType":
                                {
                                    "MIXABLE" : {
                                        "incomingChannel" : {
                                            "Dialog" : {
                                                "incomingContentType" : {
                                                    "MIXABLE" : "MAY_DUCK"
                                                }
                                            }
                                        }
                                    },
                                    "NONMIXABLE" : {
                                        "incomingChannel" : {
                                            "Dialog" : {
                                                "incomingContentType" : {
                                                    "MIXABLE" : "MAY_PAUSE"
                                                }
                                            }
                                        }
                                    }
                                }
                            },
                            "Alert" : {
                                "contentType" :
                                {
                                    "MIXABLE" : {
                                        "incomingChannel" : {
                                            "Dialog" : {
                                                  "incomingContentType" : {
                                                    "MIXABLE" : "MAY_DUCK"
                                                }
                                            },
                                            "Communications" : {
                                                  "incomingContentType" : {
                                                    "MIXABLE" : "MAY_DUCK",
                                                    "NONMIXABLE" : "MAY_DUCK"
                                                }
                                            }
                                        }
                                    }
                                }
                            },
                            "VirtualChannel" : {
                                "contentType" :
                                {
                                    "MIXABLE" : {
                                        "incomingChannel" : {
                                            "Dialog" : {
                                                  "incomingContentType" : {
                                                    "MIXABLE" : "MAY_DUCK"
                                                }
                                            },
                                            "Communications" : {
                                                  "incomingContentType" : {
                                                    "MIXABLE" : "MAY_DUCK",
                                                    "NONMIXABLE" : "MAY_DUCK"
                                                }
                                            }
                                        }
                                    }
                                }
                            },
                            "Content" : {
                                "contentType" :
                                {
                                    "MIXABLE" : {
                                        "incomingChannel" : {
                                            "Dialog" : {
                                                "incomingContentType" : {
                                                    "MIXABLE" : "MAY_DUCK"
                                                }
                                            },
                                            "Communications" : {
                                                "incomingContentType" : {
                                                    "MIXABLE" : "MAY_DUCK",
                                                    "NONMIXABLE" : "MUST_PAUSE"
                                                }
                                            },
                                            "Alert" : {
                                                "incomingContentType" : {
                                                    "MIXABLE" : "MAY_DUCK"
                                                }
                                            },
                                            "VirtualChannel" : {
                                                "incomingContentType" : {
                                                    "MIXABLE" : "MAY_DUCK"
                                                }
                                            }
                                        }
                                    },
                                    "NONMIXABLE" : {
                                        "incomingChannel" : {
                                            "Dialog" : {
                                                "incomingContentType" : {
                                                    "MIXABLE" : "MUST_PAUSE"
                                                }
                                            },
                                            "Communications" : {
                                                "incomingContentType" : {
                                                    "MIXABLE" : "MUST_PAUSE",
                                                    "NONMIXABLE" : "MUST_PAUSE"
                                                }
                                            },
                                            "Alert" : {
                                                "incomingContentType" : {
                                                    "MIXABLE" : "MUST_PAUSE"
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                }
            )";
            class TestClient : public ChannelObserverInterface, public enable_shared_from_this<TestClient> {
            public:
                TestClient(const string& channelName, const string& interfaceName) : m_channelName(channelName), m_interfaceName(interfaceName),
                                                                                     m_focusState(FocusState::NONE), m_mixingBehavior(MixingBehavior::UNDEFINED),
                                                                                     m_focusChangeCallbackInvoked(false), m_mixingBehaviorChanged(false) {}
                shared_ptr<FocusManagerInterface::Activity> createActivity(ContentType contentType = ContentType::NONMIXABLE,
                                                                           milliseconds patience = milliseconds(0)) {
                    m_contentType = contentType;
                    return FocusManagerInterface::Activity::create(m_interfaceName, shared_from_this(), patience, contentType);
                }
                const string& getChannelName() const {
                    return m_channelName;
                }
                const string& getInterfaceName() const {
                    return m_interfaceName;
                }
                void onFocusChanged(FocusState focusState, MixingBehavior behavior) override {
                    unique_lock<std::mutex> lock(m_mutex);
                    m_focusState = focusState;
                    m_mixingBehaviorChanged = (m_mixingBehavior != behavior);
                    m_mixingBehavior = behavior;
                    m_focusChangeCallbackInvoked = true;
                    m_focusChanged.notify_one();
                }
                struct testClientInfo {
                    FocusState focusState;
                    MixingBehavior mixingBehavior;
                    bool focusChanged;
                    bool mixingBehaviorChanged;
                    testClientInfo(FocusState state, MixingBehavior behavior, bool focusChg, bool mixingBehaviorChg) : focusState{state}, mixingBehavior{behavior},
                                   focusChanged{focusChg}, mixingBehaviorChanged{mixingBehaviorChg} {}
                    testClientInfo() : focusState{FocusState::NONE}, mixingBehavior{MixingBehavior::UNDEFINED}, focusChanged{false}, mixingBehaviorChanged{false} {}
                };
                testClientInfo waitForFocusOrMixingBehaviorChange(std::chrono::milliseconds timeout) {
                    unique_lock<mutex> lock(m_mutex);
                    auto success = m_focusChanged.wait_for(lock, timeout, [this]() { return m_focusChangeCallbackInvoked || m_mixingBehaviorChanged; });
                    testClientInfo ret;
                    if (!success) {
                        ret.focusChanged = false;
                        ret.mixingBehaviorChanged = false;
                    } else {
                        m_focusChangeCallbackInvoked = false;
                        ret.focusChanged = true;
                        ret.mixingBehaviorChanged = m_mixingBehaviorChanged;
                        m_mixingBehaviorChanged = false;
                    }
                    ret.focusState = m_focusState;
                    ret.mixingBehavior = m_mixingBehavior;
                    return ret;
                }
            private:
                string m_channelName;
                string m_interfaceName;
                ContentType m_contentType;
                FocusState m_focusState;
                MixingBehavior m_mixingBehavior;
                mutex m_mutex;
                condition_variable m_focusChanged;
                bool m_focusChangeCallbackInvoked;
                bool m_mixingBehaviorChanged;
            };
            class MockActivityTrackerInterface : public ActivityTrackerInterface {
            public:
                MockActivityTrackerInterface() : m_activityUpdatesOccurred{false} {}
                struct ExpectedChannelStateResult {
                    const string name;
                    const string interfaceName;
                    const FocusState focusState;
                };
                void notifyOfActivityUpdates(const vector<Channel::State>& channelStates) override {
                    unique_lock<mutex> lock(m_mutex);
                    m_updatedChannels.clear();
                    for (auto& channel : channelStates) {
                        m_updatedChannels[channel.interfaceName] = channel;
                    }
                    m_activityUpdatesOccurred = true;
                    m_activityChanged.notify_one();
                }
                void waitForActivityUpdates(milliseconds timeout, const vector<ExpectedChannelStateResult>& expected) {
                    unique_lock<mutex> lock(m_mutex);
                    bool success = m_activityChanged.wait_for(lock, timeout, [this, &expected]() {
                        if (m_activityUpdatesOccurred) {
                            EXPECT_EQ(m_updatedChannels.size(), expected.size());
                            auto count = 0;
                            for (auto& expectedChannel : expected) {
                                auto& channel = m_updatedChannels[expectedChannel.interfaceName];
                                EXPECT_EQ(channel.name, expectedChannel.name);
                                EXPECT_EQ(channel.interfaceName, expectedChannel.interfaceName);
                                EXPECT_EQ(channel.focusState, expectedChannel.focusState);
                                count++;
                            }
                        }
                        return m_activityUpdatesOccurred;
                    });
                    if (success) m_activityUpdatesOccurred = false;
                    ASSERT_TRUE(success);
                }
                bool waitForNoActivityUpdates(milliseconds timeout) {
                    unique_lock<mutex> lock(m_mutex);
                    m_activityChanged.wait_for(lock, timeout);
                    return m_activityUpdatesOccurred;
                }
            private:
                unordered_map<string, Channel::State> m_updatedChannels;
                mutex m_mutex;
                condition_variable m_activityChanged;
                bool m_activityUpdatesOccurred;
            };
            class FocusChangeManager {
            public:
                TestClient::testClientInfo getWaitResult(shared_ptr<TestClient> client) {
                    return client->waitForFocusOrMixingBehaviorChange(DEFAULT_TIMEOUT);
                }
                void assertFocusChange(shared_ptr<TestClient> client, FocusState expectedFocusState) {
                    auto waitResult = getWaitResult(client);
                    ASSERT_TRUE(waitResult.focusChanged);
                    ASSERT_EQ(expectedFocusState, waitResult.focusState);
                }
                void assertNoFocusChange(shared_ptr<TestClient> client) {
                    auto ret = client->waitForFocusOrMixingBehaviorChange(SHORT_TIMEOUT);
                    ASSERT_FALSE(ret.focusChanged);
                }
                void assertMixingBehaviorChange(shared_ptr<TestClient> client, MixingBehavior behavior) {
                    auto ret = client->waitForFocusOrMixingBehaviorChange(SHORT_TIMEOUT);
                    ASSERT_TRUE(ret.mixingBehaviorChanged);
                    ASSERT_EQ(ret.mixingBehavior, behavior);
                }
                void assertNoMixingBehaviorChange(shared_ptr<TestClient> client) {
                    auto ret = client->waitForFocusOrMixingBehaviorChange(SHORT_TIMEOUT);
                    ASSERT_FALSE(ret.mixingBehaviorChanged);
                }
                void assertNoMixingBehaviorOrFocusChange(shared_ptr<TestClient> client) {
                    auto ret = client->waitForFocusOrMixingBehaviorChange(SHORT_TIMEOUT);
                    ASSERT_FALSE(ret.mixingBehaviorChanged);
                    ASSERT_FALSE(ret.focusChanged);
                }
                void assertMixingBehaviorAndFocusChange(
                    std::shared_ptr<TestClient> client,
                    FocusState expectedFocusState,
                    MixingBehavior behavior) {
                    auto ret = client->waitForFocusOrMixingBehaviorChange(SHORT_TIMEOUT);
                    ASSERT_TRUE(ret.mixingBehaviorChanged);
                    ASSERT_TRUE(ret.focusChanged);
                    ASSERT_EQ(expectedFocusState, ret.focusState);
                    ASSERT_EQ(behavior, ret.mixingBehavior);
                }
            };
            class FocusManagerTest : public Test, public FocusChangeManager {
            protected:
                shared_ptr<FocusManager> m_focusManager;
                shared_ptr<TestClient> dialogClient;
                shared_ptr<TestClient> anotherDialogClient;
                shared_ptr<TestClient> alertsClient;
                shared_ptr<TestClient> contentClient;
                shared_ptr<TestClient> anotherContentClient;
                shared_ptr<TestClient> virtualClient;
                shared_ptr<MockActivityTrackerInterface> m_activityTracker;
                shared_ptr<interruptModel::InterruptModel> m_interruptModel;
                ConfigurationNode generateInterruptModelConfig() {
                    auto stream = shared_ptr<std::istream>(new istringstream(INTERRUPT_MODEL_CONFIG_JSON));
                    vector<shared_ptr<istream>> jsonStream({stream});
                    ConfigurationNode::initialize(jsonStream);
                    return ConfigurationNode::getRoot();
                }
                virtual void SetUp() {
                    m_activityTracker = make_shared<MockActivityTrackerInterface>();
                    FocusManager::ChannelConfiguration dialogChannelConfig{DIALOG_CHANNEL_NAME, DIALOG_CHANNEL_PRIORITY};
                    FocusManager::ChannelConfiguration alertsChannelConfig{ALERTS_CHANNEL_NAME, ALERTS_CHANNEL_PRIORITY};
                    FocusManager::ChannelConfiguration contentChannelConfig{CONTENT_CHANNEL_NAME, CONTENT_CHANNEL_PRIORITY};
                    FocusManager::ChannelConfiguration virtualChannelConfig{VIRTUAL_CHANNEL_NAME, VIRTUAL_CHANNEL_PRIORITY};
                    vector<FocusManager::ChannelConfiguration> channelConfigurations{dialogChannelConfig, alertsChannelConfig, contentChannelConfig};
                    dialogClient = make_shared<TestClient>(DIALOG_CHANNEL_NAME, DIALOG_INTERFACE_NAME);
                    alertsClient = make_shared<TestClient>(ALERTS_CHANNEL_NAME, ALERTS_INTERFACE_NAME);
                    contentClient = make_shared<TestClient>(CONTENT_CHANNEL_NAME, CONTENT_INTERFACE_NAME);
                    anotherContentClient = make_shared<TestClient>(CONTENT_CHANNEL_NAME, DIFFERENT_CONTENT_INTERFACE_NAME);
                    anotherDialogClient = make_shared<TestClient>(DIALOG_CHANNEL_NAME, DIFFERENT_DIALOG_INTERFACE_NAME);
                    virtualClient = make_shared<TestClient>(VIRTUAL_CHANNEL_NAME, VIRTUAL_INTERFACE_NAME);
                    vector<FocusManager::ChannelConfiguration> virtualChannelConfigurations{virtualChannelConfig};
                    m_interruptModel = interruptModel::InterruptModel::create(generateInterruptModelConfig()[INTERRUPT_MODEL_ROOT_KEY]);
                    m_focusManager = make_shared<FocusManager>(channelConfigurations, m_activityTracker, virtualChannelConfigurations, m_interruptModel);
                }
                bool acquireChannelHelper(shared_ptr<TestClient> client, ContentType contentType = ContentType::NONMIXABLE,
                                          milliseconds patience = milliseconds(0)) {
                    auto activity = client->createActivity(contentType, patience);
                    return m_focusManager->acquireChannel(client->getChannelName(), activity);
                }
            };
            TEST_F(FocusManagerTest, test_acquireInvalidChannelName) {
                ASSERT_FALSE(m_focusManager->acquireChannel(INCORRECT_CHANNEL_NAME, dialogClient, DIALOG_INTERFACE_NAME));
            }
            TEST_F(FocusManagerTest, test_acquireChannelWithNoOtherChannelsActive) {
                ASSERT_TRUE(acquireChannelHelper(dialogClient, ContentType::MIXABLE));
                assertMixingBehaviorAndFocusChange(dialogClient, FocusState::FOREGROUND, MixingBehavior::PRIMARY);
            }
            TEST_F(FocusManagerTest, test_acquireLowerPriorityChannelWithOneHigherPriorityChannelTaken) {
                ASSERT_TRUE(acquireChannelHelper(dialogClient, ContentType::MIXABLE));
                ASSERT_TRUE(acquireChannelHelper(alertsClient, ContentType::MIXABLE));
                assertMixingBehaviorAndFocusChange(dialogClient, FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                assertMixingBehaviorAndFocusChange(alertsClient, FocusState::BACKGROUND, MixingBehavior::MAY_DUCK);
            }
            TEST_F(FocusManagerTest, test_aquireLowerPriorityChannelWithTwoHigherPriorityChannelsTaken) {
                ASSERT_TRUE(acquireChannelHelper(dialogClient, ContentType::MIXABLE));
                ASSERT_TRUE(acquireChannelHelper(alertsClient, ContentType::MIXABLE));
                ASSERT_TRUE(acquireChannelHelper(contentClient, ContentType::MIXABLE));
                assertMixingBehaviorAndFocusChange(dialogClient, FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                assertMixingBehaviorAndFocusChange(alertsClient, FocusState::BACKGROUND, MixingBehavior::MAY_DUCK);
                assertMixingBehaviorAndFocusChange(contentClient, FocusState::BACKGROUND, MixingBehavior::MAY_DUCK);
            }
            TEST_F(FocusManagerTest, acquireVirtualChannelWithTwoLowerPriorityChannelsTaken) {
                ASSERT_TRUE(acquireChannelHelper(dialogClient, ContentType::MIXABLE));
                assertMixingBehaviorAndFocusChange(dialogClient, FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                ASSERT_TRUE(acquireChannelHelper(contentClient, ContentType::MIXABLE));
                ASSERT_TRUE(acquireChannelHelper(virtualClient, ContentType::MIXABLE));
                assertMixingBehaviorAndFocusChange(virtualClient, FocusState::BACKGROUND, MixingBehavior::MAY_DUCK);
                assertMixingBehaviorAndFocusChange(contentClient, FocusState::BACKGROUND, MixingBehavior::MAY_DUCK);
            }
            TEST_F(FocusManagerTest, test_acquireHigherPriorityChannelWithOneLowerPriorityChannelTaken) {
                ASSERT_TRUE(acquireChannelHelper(contentClient, ContentType::MIXABLE));
                assertMixingBehaviorAndFocusChange(contentClient, FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                ASSERT_TRUE(acquireChannelHelper(dialogClient, ContentType::MIXABLE));
                assertMixingBehaviorAndFocusChange(contentClient, FocusState::BACKGROUND, MixingBehavior::MAY_DUCK);
                assertMixingBehaviorAndFocusChange(dialogClient, FocusState::FOREGROUND, MixingBehavior::PRIMARY);
            }
            TEST_F(FocusManagerTest, test_kickOutActivityOnSameChannel) {
                ASSERT_TRUE(acquireChannelHelper(dialogClient));
                assertMixingBehaviorAndFocusChange(dialogClient, FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                ASSERT_TRUE(acquireChannelHelper(anotherDialogClient));
                assertMixingBehaviorAndFocusChange(dialogClient, FocusState::NONE, MixingBehavior::MUST_STOP);
                assertMixingBehaviorAndFocusChange(anotherDialogClient, FocusState::FOREGROUND, MixingBehavior::PRIMARY);
            }
            TEST_F(FocusManagerTest, test_simpleReleaseChannel) {
                ASSERT_TRUE(acquireChannelHelper(dialogClient));
                assertMixingBehaviorAndFocusChange(dialogClient, FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                ASSERT_TRUE(m_focusManager->releaseChannel(DIALOG_CHANNEL_NAME, dialogClient).get());
                assertMixingBehaviorAndFocusChange(dialogClient, FocusState::NONE, MixingBehavior::MUST_STOP);
            }
            TEST_F(FocusManagerTest, test_simpleReleaseChannelWithIncorrectObserver) {
                ASSERT_TRUE(acquireChannelHelper(dialogClient));
                assertMixingBehaviorAndFocusChange(dialogClient, FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                ASSERT_FALSE(m_focusManager->releaseChannel(CONTENT_CHANNEL_NAME, dialogClient).get());
                ASSERT_FALSE(m_focusManager->releaseChannel(DIALOG_CHANNEL_NAME, contentClient).get());
                assertNoMixingBehaviorOrFocusChange(dialogClient);
                assertNoMixingBehaviorOrFocusChange(contentClient);
            }
            TEST_F(FocusManagerTest, test_releaseForegroundChannelWhileBackgroundChannelTaken) {
                ASSERT_TRUE(acquireChannelHelper(dialogClient, ContentType::MIXABLE));
                assertMixingBehaviorAndFocusChange(dialogClient, FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                ASSERT_TRUE(acquireChannelHelper(contentClient, ContentType::MIXABLE));
                assertMixingBehaviorAndFocusChange(contentClient, FocusState::BACKGROUND, MixingBehavior::MAY_DUCK);
                assertNoMixingBehaviorOrFocusChange(dialogClient);
                ASSERT_TRUE(m_focusManager->releaseChannel(DIALOG_CHANNEL_NAME, dialogClient).get());
                assertMixingBehaviorAndFocusChange(dialogClient, FocusState::NONE, MixingBehavior::MUST_STOP);
                assertMixingBehaviorAndFocusChange(contentClient, FocusState::FOREGROUND, MixingBehavior::PRIMARY);
            }
            TEST_F(FocusManagerTest, test_simpleNonTargetedStop) {
                ASSERT_TRUE(acquireChannelHelper(dialogClient, ContentType::MIXABLE));
                assertMixingBehaviorAndFocusChange(dialogClient, FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                m_focusManager->stopForegroundActivity();
                assertMixingBehaviorAndFocusChange(dialogClient, FocusState::NONE, MixingBehavior::MUST_STOP);
            }
            TEST_F(FocusManagerTest, test_threeNonTargetedStopsWithThreeActivitiesHappening) {
                ASSERT_TRUE(acquireChannelHelper(dialogClient, ContentType::MIXABLE));
                assertMixingBehaviorAndFocusChange(dialogClient, FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                ASSERT_TRUE(acquireChannelHelper(alertsClient, ContentType::MIXABLE));
                assertMixingBehaviorAndFocusChange(alertsClient, FocusState::BACKGROUND, MixingBehavior::MAY_DUCK);
                ASSERT_TRUE(acquireChannelHelper(contentClient, ContentType::MIXABLE));
                assertMixingBehaviorAndFocusChange(contentClient, FocusState::BACKGROUND, MixingBehavior::MAY_DUCK);
                assertNoMixingBehaviorOrFocusChange(alertsClient);
                m_focusManager->stopForegroundActivity();
                assertMixingBehaviorAndFocusChange(dialogClient, FocusState::NONE, MixingBehavior::MUST_STOP);
                assertMixingBehaviorAndFocusChange(alertsClient, FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                assertNoMixingBehaviorOrFocusChange(contentClient);
                m_focusManager->stopForegroundActivity();
                assertMixingBehaviorAndFocusChange(alertsClient, FocusState::NONE, MixingBehavior::MUST_STOP);
                assertMixingBehaviorAndFocusChange(contentClient, FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                m_focusManager->stopForegroundActivity();
                assertMixingBehaviorAndFocusChange(contentClient, FocusState::NONE, MixingBehavior::MUST_STOP);
            }
            TEST_F(FocusManagerTest, test_stopForegroundActivityAndAcquireDifferentChannel) {
                ASSERT_TRUE(acquireChannelHelper(dialogClient));
                assertMixingBehaviorAndFocusChange(dialogClient, FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                m_focusManager->stopForegroundActivity();
                assertMixingBehaviorAndFocusChange(dialogClient, FocusState::NONE, MixingBehavior::MUST_STOP);
                ASSERT_TRUE(acquireChannelHelper(contentClient));
                assertMixingBehaviorAndFocusChange(contentClient, FocusState::FOREGROUND, MixingBehavior::PRIMARY);
            }
            TEST_F(FocusManagerTest, test_stopForegroundActivityAndAcquireSameChannel) {
                ASSERT_TRUE(acquireChannelHelper(dialogClient));
                assertMixingBehaviorAndFocusChange(dialogClient, FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                m_focusManager->stopForegroundActivity();
                assertMixingBehaviorAndFocusChange(dialogClient, FocusState::NONE, MixingBehavior::MUST_STOP);
                ASSERT_TRUE(acquireChannelHelper(dialogClient));
                assertMixingBehaviorAndFocusChange(dialogClient, FocusState::FOREGROUND, MixingBehavior::PRIMARY);
            }
            TEST_F(FocusManagerTest, test_stopAllActivitiesWithSingleChannel) {
                ASSERT_TRUE(acquireChannelHelper(dialogClient));
                assertMixingBehaviorAndFocusChange(dialogClient, FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                m_focusManager->stopAllActivities();
                assertMixingBehaviorAndFocusChange(dialogClient, FocusState::NONE, MixingBehavior::MUST_STOP);
                assertNoMixingBehaviorOrFocusChange(contentClient);
                assertNoMixingBehaviorOrFocusChange(alertsClient);
                ASSERT_TRUE(acquireChannelHelper(dialogClient));
                assertMixingBehaviorAndFocusChange(dialogClient, FocusState::FOREGROUND, MixingBehavior::PRIMARY);
            }
            TEST_F(FocusManagerTest, test_stopAllActivitiesWithThreeChannels) {
                ASSERT_TRUE(acquireChannelHelper(dialogClient, ContentType::MIXABLE));
                assertMixingBehaviorAndFocusChange(dialogClient, FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                ASSERT_TRUE(acquireChannelHelper(contentClient));
                assertMixingBehaviorAndFocusChange(contentClient, FocusState::BACKGROUND, MixingBehavior::MUST_PAUSE);
                ASSERT_TRUE(acquireChannelHelper(alertsClient, ContentType::MIXABLE));
                assertMixingBehaviorAndFocusChange(alertsClient, FocusState::BACKGROUND, MixingBehavior::MAY_DUCK);
                m_focusManager->stopAllActivities();
                assertMixingBehaviorAndFocusChange(dialogClient, FocusState::NONE, MixingBehavior::MUST_STOP);
                assertMixingBehaviorAndFocusChange(contentClient, FocusState::NONE, MixingBehavior::MUST_STOP);
                assertMixingBehaviorAndFocusChange(alertsClient, FocusState::NONE, MixingBehavior::MUST_STOP);
                ASSERT_TRUE(acquireChannelHelper(dialogClient));
                assertMixingBehaviorAndFocusChange(dialogClient, FocusState::FOREGROUND, MixingBehavior::PRIMARY);
            }
            TEST_F(FocusManagerTest, stopAllActivitiesWithSingleChannel) {
                ASSERT_TRUE(acquireChannelHelper(dialogClient));
                assertMixingBehaviorAndFocusChange(dialogClient, FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                m_focusManager->stopAllActivities();
                assertMixingBehaviorAndFocusChange(dialogClient, FocusState::NONE, MixingBehavior::MUST_STOP);
                assertNoMixingBehaviorOrFocusChange(contentClient);
                assertNoMixingBehaviorOrFocusChange(alertsClient);
                ASSERT_TRUE(acquireChannelHelper(dialogClient));
                assertMixingBehaviorAndFocusChange(dialogClient, FocusState::FOREGROUND, MixingBehavior::PRIMARY);
            }
            TEST_F(FocusManagerTest, stopAllActivitiesWithThreeChannels) {
                ASSERT_TRUE(acquireChannelHelper(dialogClient, ContentType::MIXABLE));
                assertMixingBehaviorAndFocusChange(dialogClient, FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                ASSERT_TRUE(acquireChannelHelper(contentClient));
                assertMixingBehaviorAndFocusChange(contentClient, FocusState::BACKGROUND, MixingBehavior::MUST_PAUSE);
                ASSERT_TRUE(acquireChannelHelper(alertsClient, ContentType::MIXABLE));
                assertMixingBehaviorAndFocusChange(alertsClient, FocusState::BACKGROUND, MixingBehavior::MAY_DUCK);
                assertNoMixingBehaviorOrFocusChange(contentClient);
                m_focusManager->stopAllActivities();
                assertMixingBehaviorAndFocusChange(dialogClient, FocusState::NONE, MixingBehavior::MUST_STOP);
                assertMixingBehaviorAndFocusChange(contentClient, FocusState::NONE, MixingBehavior::MUST_STOP);
                assertMixingBehaviorAndFocusChange(alertsClient, FocusState::NONE, MixingBehavior::MUST_STOP);
                ASSERT_TRUE(acquireChannelHelper(dialogClient));
                assertMixingBehaviorAndFocusChange(dialogClient, FocusState::FOREGROUND, MixingBehavior::PRIMARY);
            }
            TEST_F(FocusManagerTest, test_releaseBackgroundChannelWhileTwoChannelsTaken) {
                ASSERT_TRUE(acquireChannelHelper(dialogClient, ContentType::MIXABLE));
                assertMixingBehaviorAndFocusChange(dialogClient, FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                ASSERT_TRUE(acquireChannelHelper(contentClient));
                assertMixingBehaviorAndFocusChange(contentClient, FocusState::BACKGROUND, MixingBehavior::MUST_PAUSE);
                ASSERT_TRUE(m_focusManager->releaseChannel(CONTENT_CHANNEL_NAME, contentClient).get());
                assertMixingBehaviorAndFocusChange(contentClient, FocusState::NONE, MixingBehavior::MUST_STOP);
                assertNoMixingBehaviorOrFocusChange(dialogClient);
            }
            TEST_F(FocusManagerTest, test_kickOutActivityOnSameChannelWhileOtherChannelsActive) {
                ASSERT_TRUE(acquireChannelHelper(dialogClient, ContentType::MIXABLE));
                assertMixingBehaviorAndFocusChange(dialogClient, FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                ASSERT_TRUE(acquireChannelHelper(contentClient));
                assertMixingBehaviorAndFocusChange(contentClient, FocusState::BACKGROUND, MixingBehavior::MUST_PAUSE);
                ASSERT_TRUE(acquireChannelHelper(anotherDialogClient, ContentType::MIXABLE));
                assertMixingBehaviorAndFocusChange(dialogClient, FocusState::NONE, MixingBehavior::MUST_STOP);
                assertMixingBehaviorAndFocusChange(anotherDialogClient, FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                assertNoMixingBehaviorOrFocusChange(contentClient);
            }
            TEST_F(FocusManagerTest, test_addObserver) {
                vector<shared_ptr<MockFocusManagerObserver>> observers;
                observers.push_back(make_shared<MockFocusManagerObserver>());
                observers.push_back(make_shared<MockFocusManagerObserver>());
                for (auto& observer : observers) {
                    m_focusManager->addObserver(observer);
                }
                for (auto& observer : observers) {
                    observer->expectFocusChange(DIALOG_CHANNEL_NAME, FocusState::FOREGROUND);
                }
                ASSERT_TRUE(acquireChannelHelper(dialogClient));
                for (auto& observer : observers) {
                    observer->expectFocusChange(CONTENT_CHANNEL_NAME, FocusState::BACKGROUND);
                }
                ASSERT_TRUE(acquireChannelHelper(contentClient));
                for (auto& observer : observers) {
                    ASSERT_TRUE(observer->waitForFocusChanges(DEFAULT_TIMEOUT));
                }
                for (auto& observer : observers) {
                    observer->expectFocusChange(DIALOG_CHANNEL_NAME, FocusState::NONE);
                    observer->expectFocusChange(CONTENT_CHANNEL_NAME, FocusState::FOREGROUND);
                }
                m_focusManager->stopForegroundActivity();
                for (auto& observer : observers) {
                    ASSERT_TRUE(observer->waitForFocusChanges(DEFAULT_TIMEOUT));
                }
            }
            TEST_F(FocusManagerTest, test_removeObserver) {
                vector<shared_ptr<MockFocusManagerObserver>> allObservers;
                allObservers.push_back(make_shared<StrictMock<MockFocusManagerObserver>>());
                allObservers.push_back(make_shared<StrictMock<MockFocusManagerObserver>>());
                for (auto& observer : allObservers) {
                    m_focusManager->addObserver(observer);
                }
                auto activeObservers = allObservers;
                for (auto& observer : activeObservers) {
                    observer->expectFocusChange(DIALOG_CHANNEL_NAME, FocusState::FOREGROUND);
                }
                ASSERT_TRUE(acquireChannelHelper(dialogClient));
                for (auto& observer : allObservers) {
                    ASSERT_TRUE(observer->waitForFocusChanges(DEFAULT_TIMEOUT));
                }
                m_focusManager->removeObserver(activeObservers.back());
                activeObservers.pop_back();
                for (auto& observer : activeObservers) {
                    observer->expectFocusChange(CONTENT_CHANNEL_NAME, FocusState::BACKGROUND);
                }
                ASSERT_TRUE(acquireChannelHelper(contentClient));
                for (auto& observer : allObservers) {
                    ASSERT_TRUE(observer->waitForFocusChanges(DEFAULT_TIMEOUT));
                }
                for (auto& observer : activeObservers) {
                    m_focusManager->removeObserver(observer);
                }
                activeObservers.clear();
                m_focusManager->stopForegroundActivity();
                for (auto& observer : allObservers) {
                    ASSERT_TRUE(observer->waitForFocusChanges(DEFAULT_TIMEOUT));
                }
            }
            TEST_F(FocusManagerTest, test_activityTracker) {
                const vector<MockActivityTrackerInterface::ExpectedChannelStateResult> test1 = {
                    {CONTENT_CHANNEL_NAME, CONTENT_INTERFACE_NAME, FocusState::FOREGROUND}};
                ASSERT_TRUE(acquireChannelHelper(contentClient));
                m_activityTracker->waitForActivityUpdates(DEFAULT_TIMEOUT, test1);
                const vector<MockActivityTrackerInterface::ExpectedChannelStateResult> test2 = {
                    {CONTENT_CHANNEL_NAME, CONTENT_INTERFACE_NAME, FocusState::BACKGROUND},
                    {ALERTS_CHANNEL_NAME, ALERTS_INTERFACE_NAME, FocusState::FOREGROUND}};
                ASSERT_TRUE(acquireChannelHelper(alertsClient, ContentType::MIXABLE));
                m_activityTracker->waitForActivityUpdates(DEFAULT_TIMEOUT, test2);
                const vector<MockActivityTrackerInterface::ExpectedChannelStateResult> test3 = {
                    {ALERTS_CHANNEL_NAME, ALERTS_INTERFACE_NAME, FocusState::BACKGROUND},
                    {DIALOG_CHANNEL_NAME, DIALOG_INTERFACE_NAME, FocusState::FOREGROUND}};
                ASSERT_TRUE(acquireChannelHelper(dialogClient, ContentType::MIXABLE));
                m_activityTracker->waitForActivityUpdates(DEFAULT_TIMEOUT, test3);
                const vector<MockActivityTrackerInterface::ExpectedChannelStateResult> test4 = {
                    {CONTENT_CHANNEL_NAME, CONTENT_INTERFACE_NAME, FocusState::NONE}};
                ASSERT_TRUE(m_focusManager->releaseChannel(CONTENT_CHANNEL_NAME, contentClient).get());
                m_activityTracker->waitForActivityUpdates(DEFAULT_TIMEOUT, test4);
                const vector<MockActivityTrackerInterface::ExpectedChannelStateResult> test5 = {
                    {DIALOG_CHANNEL_NAME, DIALOG_INTERFACE_NAME, FocusState::NONE},
                    {DIALOG_CHANNEL_NAME, DIFFERENT_DIALOG_INTERFACE_NAME, FocusState::FOREGROUND}};
                ASSERT_TRUE(acquireChannelHelper(anotherDialogClient));
                m_activityTracker->waitForActivityUpdates(DEFAULT_TIMEOUT, test5);
                const vector<MockActivityTrackerInterface::ExpectedChannelStateResult> test6 = {
                    {ALERTS_CHANNEL_NAME, ALERTS_INTERFACE_NAME, FocusState::FOREGROUND},
                    {DIALOG_CHANNEL_NAME, DIFFERENT_DIALOG_INTERFACE_NAME, FocusState::NONE}};
                ASSERT_TRUE(m_focusManager->releaseChannel(DIALOG_CHANNEL_NAME, anotherDialogClient).get());
                m_activityTracker->waitForActivityUpdates(DEFAULT_TIMEOUT, test6);
                const vector<MockActivityTrackerInterface::ExpectedChannelStateResult> test7 = {
                    {ALERTS_CHANNEL_NAME, ALERTS_INTERFACE_NAME, FocusState::NONE}};
                ASSERT_TRUE(m_focusManager->releaseChannel(ALERTS_CHANNEL_NAME, alertsClient).get());
                m_activityTracker->waitForActivityUpdates(DEFAULT_TIMEOUT, test7);
                ASSERT_TRUE(acquireChannelHelper(virtualClient));
                ASSERT_FALSE(m_activityTracker->waitForNoActivityUpdates(NO_ACTIVITY_UPDATE_TIMEOUT));
                ASSERT_TRUE(m_focusManager->releaseChannel(VIRTUAL_CHANNEL_NAME, virtualClient).get());
                ASSERT_FALSE(m_activityTracker->waitForNoActivityUpdates(NO_ACTIVITY_UPDATE_TIMEOUT));
                const vector<MockActivityTrackerInterface::ExpectedChannelStateResult> test8 = {
                    {CONTENT_CHANNEL_NAME, CONTENT_INTERFACE_NAME, FocusState::FOREGROUND}};
                ASSERT_TRUE(acquireChannelHelper(contentClient));
                m_activityTracker->waitForActivityUpdates(DEFAULT_TIMEOUT, test8);
                const vector<MockActivityTrackerInterface::ExpectedChannelStateResult> test9 = {
                    {CONTENT_CHANNEL_NAME, CONTENT_INTERFACE_NAME, FocusState::BACKGROUND}};
                ASSERT_TRUE(acquireChannelHelper(virtualClient));
                m_activityTracker->waitForActivityUpdates(DEFAULT_TIMEOUT, test9);
            }
            class ChannelTest : public Test, public FocusChangeManager {
            protected:
                shared_ptr<TestClient> clientA;
                shared_ptr<TestClient> clientB;
                shared_ptr<TestClient> clientC;
                shared_ptr<Channel> testChannel;
                shared_ptr<Channel> contentChannel;
                virtual void SetUp() {
                    clientA = make_shared<TestClient>(CONTENT_CHANNEL_NAME, "ClientA_Interface");
                    clientB = make_shared<TestClient>(CONTENT_CHANNEL_NAME, "ClientB_Interface");
                    clientC = make_shared<TestClient>(CONTENT_CHANNEL_NAME, "ClientC_Interface");
                    testChannel = make_shared<Channel>(DIALOG_CHANNEL_NAME, DIALOG_CHANNEL_PRIORITY);
                    contentChannel = make_shared<Channel>(CONTENT_CHANNEL_NAME, CONTENT_CHANNEL_PRIORITY);
                }
                struct ActivityUpdateElem {
                    string m_interfaceName;
                    FocusState m_focusState;
                    ActivityUpdateElem(string interfaceName, FocusState focus) : m_interfaceName{interfaceName}, m_focusState{focus} {}
                };
                void checkActivityUpdates(shared_ptr<Channel> channel, vector<ActivityUpdateElem>& incoming) {
                    auto activityUpdates = channel->getActivityUpdates();
                    ASSERT_EQ(incoming.size(), activityUpdates.size());
                    for (size_t i = 0; i < incoming.size(); i++) {
                        ASSERT_EQ(activityUpdates.at(i).interfaceName, incoming.at(i).m_interfaceName);
                        ASSERT_EQ(activityUpdates.at(i).focusState, incoming.at(i).m_focusState);
                    }
                    incoming.clear();
                }
            };
            TEST_F(ChannelTest, test_getName) {
                ASSERT_EQ(testChannel->getName(), DIALOG_CHANNEL_NAME);
            }
            TEST_F(ChannelTest, test_getPriority) {
                ASSERT_EQ(testChannel->getPriority(), DIALOG_CHANNEL_PRIORITY);
            }
            TEST_F(ChannelTest, test_setObserverThenSetFocus) {
                auto Activity_A = clientA->createActivity();
                testChannel->setPrimaryActivity(Activity_A);
                ASSERT_TRUE(testChannel->setFocus(FocusState::FOREGROUND, MixingBehavior::PRIMARY));
                assertMixingBehaviorAndFocusChange(clientA, FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                ASSERT_TRUE(testChannel->setFocus(FocusState::BACKGROUND, MixingBehavior::MUST_PAUSE));
                assertMixingBehaviorAndFocusChange(clientA, FocusState::BACKGROUND, MixingBehavior::MUST_PAUSE);
                ASSERT_TRUE(testChannel->setFocus(FocusState::NONE, MixingBehavior::MUST_STOP));
                assertMixingBehaviorAndFocusChange(clientA, FocusState::NONE, MixingBehavior::MUST_STOP);
                ASSERT_FALSE(testChannel->setFocus(FocusState::NONE, MixingBehavior::MUST_STOP));
            }
            TEST_F(ChannelTest, test_priorityComparison) {
                shared_ptr<Channel> lowerPriorityChannel = make_shared<Channel>(CONTENT_CHANNEL_NAME, CONTENT_CHANNEL_PRIORITY);
                ASSERT_TRUE(*testChannel > *lowerPriorityChannel);
                ASSERT_FALSE(*lowerPriorityChannel > *testChannel);
            }
            TEST_F(ChannelTest, test_isChannelActive) {
                ASSERT_FALSE(testChannel->isActive());
                auto Activity_A = clientA->createActivity();
                testChannel->setPrimaryActivity(Activity_A);
                ASSERT_TRUE(testChannel->isActive());
                auto Activity_B = clientB->createActivity(avsCommon::avs::ContentType::NONMIXABLE, std::chrono::seconds(2));
                testChannel->setPrimaryActivity(Activity_B);
                ASSERT_TRUE(testChannel->isActive());
                testChannel->releaseActivity(clientA);
                ASSERT_TRUE(testChannel->isActive());
                testChannel->releaseActivity(clientB);
                ASSERT_FALSE(testChannel->isActive());
            }
            TEST_F(ChannelTest, test_getTimeAtIdle) {
                auto startTime = testChannel->getState().timeAtIdle;
                ASSERT_TRUE(testChannel->setFocus(FocusState::FOREGROUND, MixingBehavior::PRIMARY));
                auto afterForegroundTime = testChannel->getState().timeAtIdle;
                ASSERT_EQ(startTime, afterForegroundTime);
                ASSERT_TRUE(testChannel->setFocus(FocusState::BACKGROUND, MixingBehavior::MUST_PAUSE));
                auto afterBackgroundTime = testChannel->getState().timeAtIdle;
                ASSERT_EQ(afterBackgroundTime, afterForegroundTime);
                ASSERT_TRUE(testChannel->setFocus(FocusState::NONE, MixingBehavior::MUST_STOP));
                auto afterNoneTime = testChannel->getState().timeAtIdle;
                ASSERT_GT(afterNoneTime, afterBackgroundTime);
            }
            TEST_F(ChannelTest, test_MultiActivity_NewActivityKicksExistingActivity) {
                vector<ActivityUpdateElem> expectedUpdates;
                auto Activity_A = clientA->createActivity();
                contentChannel->setPrimaryActivity(Activity_A);
                ASSERT_TRUE(contentChannel->getState().interfaceName == Activity_A->getInterface());
                contentChannel->setFocus(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                expectedUpdates.push_back({ActivityUpdateElem(clientA->getInterfaceName(), FocusState::FOREGROUND)});
                checkActivityUpdates(contentChannel, expectedUpdates);
                auto Activity_B = clientB->createActivity();
                contentChannel->setPrimaryActivity(Activity_B);
                contentChannel->setFocus(FocusState::FOREGROUND, MixingBehavior::PRIMARY, true);
                expectedUpdates.push_back({ActivityUpdateElem(clientA->getInterfaceName(), FocusState::NONE)});
                expectedUpdates.push_back({ActivityUpdateElem(clientB->getInterfaceName(), FocusState::FOREGROUND)});
                checkActivityUpdates(contentChannel, expectedUpdates);
                ASSERT_TRUE(contentChannel->getState().interfaceName == Activity_B->getInterface());
                ASSERT_EQ(contentChannel->getActivity(Activity_A->getInterface()), nullptr);
                contentChannel->releaseActivity(clientB->getInterfaceName());
                contentChannel->setFocus(FocusState::NONE, MixingBehavior::MUST_STOP);
                expectedUpdates.push_back({ActivityUpdateElem(clientB->getInterfaceName(), FocusState::NONE)});
                checkActivityUpdates(contentChannel, expectedUpdates);
                ASSERT_EQ(contentChannel->getActivity(Activity_B->getInterface()), nullptr);
            }
            TEST_F(ChannelTest, test_MultiActivity_IncomingActivityWithPatience1) {
                vector<ActivityUpdateElem> expectedUpdates;
                auto Activity_A = clientA->createActivity();
                contentChannel->setPrimaryActivity(Activity_A);
                ASSERT_TRUE(contentChannel->getState().interfaceName == Activity_A->getInterface());
                contentChannel->setFocus(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                expectedUpdates.push_back({ActivityUpdateElem(clientA->getInterfaceName(), FocusState::FOREGROUND)});
                checkActivityUpdates(contentChannel, expectedUpdates);
                auto Activity_B = clientB->createActivity(ContentType::MIXABLE, std::chrono::seconds(5));
                contentChannel->setPrimaryActivity(Activity_B);
                contentChannel->setFocus(FocusState::FOREGROUND, MixingBehavior::PRIMARY, true);
                expectedUpdates.push_back({ActivityUpdateElem(clientA->getInterfaceName(), FocusState::NONE)});
                expectedUpdates.push_back({ActivityUpdateElem(clientB->getInterfaceName(), FocusState::FOREGROUND)});
                checkActivityUpdates(contentChannel, expectedUpdates);
                ASSERT_TRUE(contentChannel->getState().interfaceName == Activity_B->getInterface());
                ASSERT_NE(contentChannel->getActivity(Activity_A->getInterface()), nullptr);
                contentChannel->releaseActivity(clientB->getInterfaceName());
                expectedUpdates.push_back({ActivityUpdateElem(clientB->getInterfaceName(), FocusState::NONE)});
                expectedUpdates.push_back({ActivityUpdateElem(clientA->getInterfaceName(), FocusState::FOREGROUND)});
                checkActivityUpdates(contentChannel, expectedUpdates);
                ASSERT_EQ(contentChannel->getActivity(Activity_B->getInterface()), nullptr);
                ASSERT_TRUE(contentChannel->getState().interfaceName == Activity_A->getInterface());
                contentChannel->releaseActivity(clientA->getInterfaceName());
                expectedUpdates.push_back({ActivityUpdateElem(clientA->getInterfaceName(), FocusState::NONE)});
                checkActivityUpdates(contentChannel, expectedUpdates);
                ASSERT_EQ(contentChannel->getActivity(Activity_A->getInterface()), nullptr);
            }
            TEST_F(ChannelTest, test_MultiActivity_IncomingActivityWithPatience2) {
                vector<ActivityUpdateElem> expectedUpdates;
                auto Activity_A = clientA->createActivity();
                contentChannel->setPrimaryActivity(Activity_A);
                ASSERT_TRUE(contentChannel->getState().interfaceName == Activity_A->getInterface());
                contentChannel->setFocus(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                expectedUpdates.push_back({ActivityUpdateElem(clientA->getInterfaceName(), FocusState::FOREGROUND)});
                checkActivityUpdates(contentChannel, expectedUpdates);
                auto Activity_B = clientB->createActivity(ContentType::MIXABLE, std::chrono::seconds(5));
                contentChannel->setPrimaryActivity(Activity_B);
                contentChannel->setFocus(FocusState::FOREGROUND, MixingBehavior::PRIMARY, true);
                expectedUpdates.push_back({ActivityUpdateElem(clientA->getInterfaceName(), FocusState::NONE)});
                expectedUpdates.push_back({ActivityUpdateElem(clientB->getInterfaceName(), FocusState::FOREGROUND)});
                checkActivityUpdates(contentChannel, expectedUpdates);
                ASSERT_TRUE(contentChannel->getState().interfaceName == Activity_B->getInterface());
                ASSERT_NE(contentChannel->getActivity(Activity_A->getInterface()), nullptr);
                contentChannel->releaseActivity(clientA->getInterfaceName());
                contentChannel->setFocus(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                expectedUpdates.push_back({ActivityUpdateElem(clientB->getInterfaceName(), FocusState::FOREGROUND)});
                checkActivityUpdates(contentChannel, expectedUpdates);
                ASSERT_EQ(contentChannel->getActivity(Activity_A->getInterface()), nullptr);
                ASSERT_TRUE(contentChannel->getState().interfaceName == Activity_B->getInterface());
                contentChannel->releaseActivity(clientB->getInterfaceName());
                expectedUpdates.push_back({ActivityUpdateElem(clientB->getInterfaceName(), FocusState::NONE)});
                checkActivityUpdates(contentChannel, expectedUpdates);
                ASSERT_EQ(contentChannel->getActivity(Activity_B->getInterface()), nullptr);
            }
            TEST_F(ChannelTest, test_MultiActivity_IncomingActivityWithPatience3) {
                vector<ActivityUpdateElem> expectedUpdates;
                auto Activity_A = clientA->createActivity();
                contentChannel->setPrimaryActivity(Activity_A);
                ASSERT_TRUE(contentChannel->getState().interfaceName == Activity_A->getInterface());
                contentChannel->setFocus(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                expectedUpdates.push_back({ActivityUpdateElem(clientA->getInterfaceName(), FocusState::FOREGROUND)});
                checkActivityUpdates(contentChannel, expectedUpdates);
                auto Activity_B = clientB->createActivity(ContentType::MIXABLE, std::chrono::seconds(1));
                contentChannel->setPrimaryActivity(Activity_B);
                contentChannel->setFocus(FocusState::FOREGROUND, MixingBehavior::PRIMARY, true);
                expectedUpdates.push_back({ActivityUpdateElem(clientA->getInterfaceName(), FocusState::NONE)});
                expectedUpdates.push_back({ActivityUpdateElem(clientB->getInterfaceName(), FocusState::FOREGROUND)});
                checkActivityUpdates(contentChannel, expectedUpdates);
                ASSERT_TRUE(contentChannel->getState().interfaceName == Activity_B->getInterface());
                ASSERT_NE(contentChannel->getActivity(Activity_A->getInterface()), nullptr);
                std::this_thread::sleep_for(std::chrono::seconds(2));
                ASSERT_EQ(contentChannel->getActivity(Activity_A->getInterface()), nullptr);
                ASSERT_TRUE(contentChannel->getState().interfaceName == Activity_B->getInterface());
                contentChannel->releaseActivity(clientB->getInterfaceName());
                expectedUpdates.push_back({ActivityUpdateElem(clientB->getInterfaceName(), FocusState::NONE)});
                checkActivityUpdates(contentChannel, expectedUpdates);
                ASSERT_EQ(contentChannel->getActivity(Activity_B->getInterface()), nullptr);
            }
            TEST_F(ChannelTest, test_MultiActivity_IncomingActivityWithPatience4) {
                vector<ActivityUpdateElem> expectedUpdates;
                auto Activity_A = clientA->createActivity();
                contentChannel->setPrimaryActivity(Activity_A);
                ASSERT_TRUE(contentChannel->getState().interfaceName == Activity_A->getInterface());
                contentChannel->setFocus(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                expectedUpdates.push_back({ActivityUpdateElem(clientA->getInterfaceName(), FocusState::FOREGROUND)});
                checkActivityUpdates(contentChannel, expectedUpdates);
                auto Activity_B = clientB->createActivity(ContentType::MIXABLE, std::chrono::seconds(5));
                contentChannel->setPrimaryActivity(Activity_B);
                contentChannel->setFocus(FocusState::FOREGROUND, MixingBehavior::PRIMARY, true);
                expectedUpdates.push_back({ActivityUpdateElem(clientA->getInterfaceName(), FocusState::NONE)});
                expectedUpdates.push_back({ActivityUpdateElem(clientB->getInterfaceName(), FocusState::FOREGROUND)});
                checkActivityUpdates(contentChannel, expectedUpdates);
                ASSERT_TRUE(contentChannel->getState().interfaceName == Activity_B->getInterface());
                ASSERT_NE(contentChannel->getActivity(Activity_A->getInterface()), nullptr);
                auto Activity_C = clientC->createActivity(ContentType::MIXABLE, std::chrono::seconds(5));
                contentChannel->setPrimaryActivity(Activity_C);
                contentChannel->setFocus(FocusState::FOREGROUND, MixingBehavior::PRIMARY, true);
                expectedUpdates.push_back({ActivityUpdateElem(clientB->getInterfaceName(), FocusState::NONE)});
                expectedUpdates.push_back({ActivityUpdateElem(clientC->getInterfaceName(), FocusState::FOREGROUND)});
                checkActivityUpdates(contentChannel, expectedUpdates);
                ASSERT_TRUE(contentChannel->getState().interfaceName == Activity_C->getInterface());
                ASSERT_EQ(contentChannel->getActivity(Activity_A->getInterface()), nullptr);
                ASSERT_NE(contentChannel->getActivity(Activity_B->getInterface()), nullptr);
                contentChannel->releaseActivity(clientC->getInterfaceName());
                expectedUpdates.push_back({ActivityUpdateElem(clientC->getInterfaceName(), FocusState::NONE)});
                expectedUpdates.push_back({ActivityUpdateElem(clientB->getInterfaceName(), FocusState::FOREGROUND)});
                checkActivityUpdates(contentChannel, expectedUpdates);
                ASSERT_EQ(contentChannel->getActivity(Activity_A->getInterface()), nullptr);
                ASSERT_EQ(contentChannel->getActivity(Activity_C->getInterface()), nullptr);
                ASSERT_TRUE(contentChannel->getState().interfaceName == Activity_B->getInterface());
            }
            TEST_F(ChannelTest, test_MultiActivity_IncomingActivityWithPatience5) {
                vector<ActivityUpdateElem> expectedUpdates;
                auto Activity_A = clientA->createActivity();
                contentChannel->setPrimaryActivity(Activity_A);
                ASSERT_TRUE(contentChannel->getState().interfaceName == Activity_A->getInterface());
                contentChannel->setFocus(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                expectedUpdates.push_back({ActivityUpdateElem(clientA->getInterfaceName(), FocusState::FOREGROUND)});
                checkActivityUpdates(contentChannel, expectedUpdates);
                auto Activity_B = clientB->createActivity(ContentType::MIXABLE, std::chrono::seconds(5));
                contentChannel->setPrimaryActivity(Activity_B);
                contentChannel->setFocus(FocusState::FOREGROUND, MixingBehavior::PRIMARY, true);
                expectedUpdates.push_back({ActivityUpdateElem(clientA->getInterfaceName(), FocusState::NONE)});
                expectedUpdates.push_back({ActivityUpdateElem(clientB->getInterfaceName(), FocusState::FOREGROUND)});
                checkActivityUpdates(contentChannel, expectedUpdates);
                ASSERT_TRUE(contentChannel->getState().interfaceName == Activity_B->getInterface());
                ASSERT_NE(contentChannel->getActivity(Activity_A->getInterface()), nullptr);
                auto Activity_C = clientC->createActivity(ContentType::MIXABLE, std::chrono::seconds(0));
                contentChannel->setPrimaryActivity(Activity_C);
                contentChannel->setFocus(FocusState::FOREGROUND, MixingBehavior::PRIMARY, true);
                expectedUpdates.push_back({ActivityUpdateElem(clientB->getInterfaceName(), FocusState::NONE)});
                expectedUpdates.push_back({ActivityUpdateElem(clientC->getInterfaceName(), FocusState::FOREGROUND)});
                checkActivityUpdates(contentChannel, expectedUpdates);
                ASSERT_TRUE(contentChannel->getState().interfaceName == Activity_C->getInterface());
                ASSERT_EQ(contentChannel->getActivity(Activity_A->getInterface()), nullptr);
                ASSERT_EQ(contentChannel->getActivity(Activity_B->getInterface()), nullptr);
                contentChannel->releaseActivity(clientC->getInterfaceName());
                expectedUpdates.push_back({ActivityUpdateElem(clientC->getInterfaceName(), FocusState::NONE)});
                checkActivityUpdates(contentChannel, expectedUpdates);
                ASSERT_EQ(contentChannel->getActivity(Activity_C->getInterface()), nullptr);
            }
        }
    }
}