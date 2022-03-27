#include <algorithm>
#include <memory>
#include <set>
#include <vector>
#include <avs/attachment/MockAttachmentManager.h>
#include <sdkinterfaces/MockChannelVolumeInterface.h>
#include <sdkinterfaces/MockContextManager.h>
#include <sdkinterfaces/MockDirectiveHandlerResult.h>
#include <sdkinterfaces/MockExceptionEncounteredSender.h>
#include <sdkinterfaces/MockMessageSender.h>
#include <sdkinterfaces/MockSpeakerInterface.h>
#include <sdkinterfaces/SpeakerManagerObserverInterface.h>
#include <memory/Memory.h>
#include <metrics/MockMetricRecorder.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <json/document.h>
#include <json/en.h>
#include <json/stringbuffer.h>
#include <json/writer.h>
#include "SpeakerManager.h"
#include "SpeakerManagerConstants.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace speakerManager {
            namespace test {
                using namespace chrono;
                using namespace attachment;
                using namespace speakerConstants;
                using namespace memory;
                using namespace testing;
                using namespace attachment::test;
                using namespace sdkInterfaces::test;
                using namespace metrics::test;
                using SMI = SpeakerManagerInterface;
                static std::chrono::milliseconds TIMEOUT(1000);
                static const std::string MESSAGE_ID("messageId");
                static const std::string VOLUME_PAYLOAD =
                    "{"
                    "\"volume\":" +
                    std::to_string(AVS_SET_VOLUME_MAX) +
                    ""
                    "}";
                static const std::string MUTE_PAYLOAD =
                    "{"
                    "\"mute\":" +
                    MUTE_STRING +
                    ""
                    "}";
                static const std::string UNMUTE_PAYLOAD =
                    "{"
                    "\"mute\":" +
                    UNMUTE_STRING +
                    ""
                    "}";
            #ifndef ENABLE_MAXVOLUME_SETTING
                static const int8_t VALID_MAXIMUM_VOLUME_LIMIT = AVS_SET_VOLUME_MAX - 10;
                static const int8_t INVALID_MAXIMUM_VOLUME_LIMIT = AVS_SET_VOLUME_MAX + 10;
            #endif
                class MockObserver : public SpeakerManagerObserverInterface {
                public:
                    MOCK_METHOD3(onSpeakerSettingsChanged, void(const Source&, const Type&, const SpeakerSettings&));
                };
                class SpeakerManagerTest : public TestWithParam<vector<Type>> {
                public:
                    void SetUp();
                    void TearDown();
                    void cleanUp();
                    void wakeOnSetCompleted();
                    set<ChannelVolumeInterface::Type> getUniqueTypes(vector<shared_ptr<ChannelVolumeInterface>>& groups);
                #ifndef ENABLE_MAXVOLUME_SETTING
                    void createAndSendVolumeDirective(const string directiveName, const int8_t volume);
                #endif
                    vector<shared_ptr<ChannelVolumeInterface>> createChannelVolumeInterfaces() {
                        auto channelVolumeInterface = make_shared<NiceMock<MockChannelVolumeInterface>>();
                        channelVolumeInterface->DelegateToReal();
                        return {channelVolumeInterface};
                    }
                    SpeakerManagerTest() : m_wakeSetCompletedPromise{}, m_wakeSetCompletedFuture{m_wakeSetCompletedPromise.get_future()} {}
                protected:
                    promise<void> m_wakeSetCompletedPromise;
                    future<void> m_wakeSetCompletedFuture;
                    shared_ptr<MetricRecorderInterface> m_metricRecorder;
                    shared_ptr<NiceMock<MockContextManager>> m_mockContextManager;
                    shared_ptr<StrictMock<MockMessageSender>> m_mockMessageSender;
                    shared_ptr<StrictMock<MockExceptionEncounteredSender>> m_mockExceptionSender;
                    unique_ptr<StrictMock<MockDirectiveHandlerResult>> m_mockDirectiveHandlerResult;
                    shared_ptr<NiceMock<MockObserver>> m_observer;
                    shared_ptr<SpeakerManager> m_speakerManager;
                };
                void SpeakerManagerTest::SetUp() {
                    m_metricRecorder = make_shared<NiceMock<MockMetricRecorder>>();
                    m_mockContextManager = make_shared<NiceMock<MockContextManager>>();
                    m_mockMessageSender = make_shared<StrictMock<MockMessageSender>>();
                    m_mockExceptionSender = make_shared<StrictMock<MockExceptionEncounteredSender>>();
                    m_mockDirectiveHandlerResult = make_unique<StrictMock<MockDirectiveHandlerResult>>();
                    m_observer = make_shared<NiceMock<MockObserver>>();
                }
                void SpeakerManagerTest::TearDown() {
                    if (m_speakerManager) {
                        m_speakerManager->shutdown();
                        m_speakerManager.reset();
                    }
                }
                void SpeakerManagerTest::wakeOnSetCompleted() {
                    m_wakeSetCompletedPromise.set_value();
                }
                set<Type> SpeakerManagerTest::getUniqueTypes(
                    vector<shared_ptr<ChannelVolumeInterface>>& groups) {
                    set<Type> types;
                    for (auto item : groups) {
                        types.insert(item->getSpeakerType());
                    }
                    return types;
                }
            #ifndef ENABLE_MAXVOLUME_SETTING
                void SpeakerManagerTest::createAndSendVolumeDirective(const string directiveName, const int8_t volume) {
                    EXPECT_CALL(*(m_mockDirectiveHandlerResult.get()), setCompleted()).Times(1)
                        .WillOnce(InvokeWithoutArgs(this, &SpeakerManagerTest::wakeOnSetCompleted));
                    static int id = 1;
                    const string messageId = MESSAGE_ID + to_string(id++);
                    string payload = "{\"volume\":" + to_string(volume) + "}";
                    auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(SET_VOLUME.nameSpace, directiveName, messageId);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, payload, attachmentManager,"");
                    m_speakerManager->CapabilityAgent::preHandleDirective(directive,move(m_mockDirectiveHandlerResult));
                    m_speakerManager->CapabilityAgent::handleDirective(messageId);
                    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
                }
                static int8_t getSpeakerVolume(shared_ptr<ChannelVolumeInterface> channelVolumeInterface) {
                    SpeakerSettings speakerSettings;
                    channelVolumeInterface->getSpeakerSettings(&speakerSettings);
                    return speakerSettings.volume;
                }
            #endif
                string generateVolumeStateJson(SpeakerSettings settings) {
                    Document state(kObjectType);
                    rapidjson::Value _VOLUME_KEY{VOLUME_KEY, strlen(VOLUME_KEY)};
                    rapidjson::Value _MUTED_KEY{MUTED_KEY, strlen(MUTED_KEY)};
                    state.AddMember(_VOLUME_KEY, settings.volume, state.GetAllocator());
                    state.AddMember(_MUTED_KEY, settings.mute, state.GetAllocator());
                    StringBuffer buffer;
                    rapidjson::Writer<StringBuffer> writer(buffer);
                    if (!state.Accept(writer)) return "";
                    return buffer.GetString();
                }
                TEST_F(SpeakerManagerTest, test_nullContextManager) {
                    auto channelVolumeInterfaces = createChannelVolumeInterfaces();
                    m_speakerManager = SpeakerManager::create(channelVolumeInterfaces, nullptr, m_mockMessageSender,
                                                              m_mockExceptionSender, m_metricRecorder);
                    ASSERT_EQ(m_speakerManager, nullptr);
                }
                TEST_F(SpeakerManagerTest, test_nullMessageSender) {
                    auto channelVolumeInterfaces = createChannelVolumeInterfaces();
                    m_speakerManager = SpeakerManager::create(channelVolumeInterfaces, m_mockContextManager, nullptr,
                                                              m_mockExceptionSender, m_metricRecorder);
                    ASSERT_EQ(m_speakerManager, nullptr);
                }
                TEST_F(SpeakerManagerTest, test_nullExceptionSender) {
                    auto channelVolumeInterfaces = createChannelVolumeInterfaces();
                    m_speakerManager = SpeakerManager::create(channelVolumeInterfaces, m_mockContextManager, m_mockMessageSender,
                                                              nullptr, m_metricRecorder);
                    ASSERT_EQ(m_speakerManager, nullptr);
                }
                TEST_F(SpeakerManagerTest, test_noChannelVolumeInterfaces) {
                    m_speakerManager = SpeakerManager::create({}, m_mockContextManager, m_mockMessageSender,
                                                              m_mockExceptionSender, m_metricRecorder);
                    ASSERT_NE(m_speakerManager, nullptr);
                }
                TEST_F(SpeakerManagerTest, test_contextManagerSetStateConstructor) {
                    EXPECT_CALL(*m_mockContextManager,setState(VOLUME_STATE, generateVolumeStateJson(DEFAULT_SETTINGS),StateRefreshPolicy::NEVER, _))
                        .Times(Exactly(1));
                    auto groups = createChannelVolumeInterfaces();
                    m_speakerManager = SpeakerManager::create(groups, m_mockContextManager, m_mockMessageSender,
                                                              m_mockExceptionSender, m_metricRecorder);
                }
                TEST_F(SpeakerManagerTest, test_setVolumeUnderBounds) {
                    auto channelVolumeInterface = make_shared<NiceMock<MockChannelVolumeInterface>>();
                    channelVolumeInterface->DelegateToReal();
                    EXPECT_CALL(*channelVolumeInterface, setUnduckedVolume(_)).Times(Exactly(0));
                    m_speakerManager = SpeakerManager::create({channelVolumeInterface}, m_mockContextManager,
                                                              m_mockMessageSender, m_mockExceptionSender,
                                                              m_metricRecorder);
                    EXPECT_CALL(*m_observer, onSpeakerSettingsChanged(_, _, _)).Times(Exactly(0));
                    m_speakerManager->addSpeakerManagerObserver(m_observer);
                    SMI::NotificationProperties properties;
                    future<bool> future = m_speakerManager->setVolume(Type::AVS_SPEAKER_VOLUME, AVS_SET_VOLUME_MIN - 1, properties);
                    ASSERT_FALSE(future.get());
                }
                TEST_F(SpeakerManagerTest, test_setVolumeOverBounds) {
                    auto channelVolumeInterface = make_shared<NiceMock<MockChannelVolumeInterface>>();
                    channelVolumeInterface->DelegateToReal();
                    EXPECT_CALL(*channelVolumeInterface, setUnduckedVolume(_)).Times(Exactly(0));
                    m_speakerManager = SpeakerManager::create({channelVolumeInterface}, m_mockContextManager,
                                                              m_mockMessageSender, m_mockExceptionSender,
                                                              m_metricRecorder);
                    EXPECT_CALL(*m_observer, onSpeakerSettingsChanged(_, _, _)).Times(Exactly(0));
                    m_speakerManager->addSpeakerManagerObserver(m_observer);
                    SMI::NotificationProperties properties;
                    future<bool> future = m_speakerManager->setVolume(Type::AVS_SPEAKER_VOLUME, AVS_SET_VOLUME_MAX + 1, properties);
                    ASSERT_FALSE(future.get());
                }
                TEST_F(SpeakerManagerTest, test_adjustVolumeUnderBounds) {
                    auto channelVolumeInterface = make_shared<NiceMock<MockChannelVolumeInterface>>();
                    channelVolumeInterface->DelegateToReal();
                    EXPECT_CALL(*channelVolumeInterface, setUnduckedVolume(_)).Times(Exactly(0));
                    m_speakerManager = SpeakerManager::create({channelVolumeInterface}, m_mockContextManager,
                                                              m_mockMessageSender, m_mockExceptionSender,
                                                              m_metricRecorder);
                    EXPECT_CALL(*m_observer, onSpeakerSettingsChanged(_, _, _)).Times(Exactly(0));
                    m_speakerManager->addSpeakerManagerObserver(m_observer);
                    SpeakerManagerInterface::NotificationProperties properties;
                    future<bool> future = m_speakerManager->adjustVolume(Type::AVS_SPEAKER_VOLUME, AVS_ADJUST_VOLUME_MIN - 1, properties);
                    ASSERT_FALSE(future.get());
                }
                TEST_F(SpeakerManagerTest, test_adjustVolumeOverBounds) {
                    auto channelVolumeInterface = make_shared<NiceMock<MockChannelVolumeInterface>>();
                    channelVolumeInterface->DelegateToReal();
                    EXPECT_CALL(*channelVolumeInterface, setUnduckedVolume(_)).Times(Exactly(0));
                    m_speakerManager = SpeakerManager::create({channelVolumeInterface}, m_mockContextManager,
                                                              m_mockMessageSender, m_mockExceptionSender,
                                                              m_metricRecorder);
                    EXPECT_CALL(*m_observer, onSpeakerSettingsChanged(_, _, _)).Times(Exactly(0));
                    m_speakerManager->addSpeakerManagerObserver(m_observer);
                    SMI::NotificationProperties properties;
                    future<bool> future = m_speakerManager->adjustVolume(Type::AVS_SPEAKER_VOLUME, AVS_ADJUST_VOLUME_MAX + 1, properties);
                    ASSERT_FALSE(future.get());
                }
                TEST_F(SpeakerManagerTest, test_setVolumeOutOfSync) {
                    auto channelVolumeInterface = make_shared<NiceMock<MockChannelVolumeInterface>>();
                    channelVolumeInterface->DelegateToReal();
                    auto channelVolumeInterface2 = make_shared<NiceMock<MockChannelVolumeInterface>>();
                    EXPECT_CALL(*channelVolumeInterface2, getSpeakerType()).WillRepeatedly(Return(Type::AVS_SPEAKER_VOLUME));
                    EXPECT_CALL(*channelVolumeInterface2, setUnduckedVolume(_)).WillRepeatedly(Return(true));
                    EXPECT_CALL(*channelVolumeInterface2, getSpeakerSettings(_)).WillRepeatedly(Return(false));
                    m_speakerManager = SpeakerManager::create({channelVolumeInterface, channelVolumeInterface2},
                                                              m_mockContextManager,m_mockMessageSender,
                                                              m_mockExceptionSender,m_metricRecorder);
                    EXPECT_CALL(*m_observer, onSpeakerSettingsChanged(_, _, _)).Times(Exactly(0));
                    m_speakerManager->addSpeakerManagerObserver(m_observer);
                    SpeakerManagerInterface::NotificationProperties properties;
                    future<bool> future = m_speakerManager->setVolume(Type::AVS_SPEAKER_VOLUME, AVS_SET_VOLUME_MAX, properties);
                    ASSERT_FALSE(future.get());
                }
                TEST_F(SpeakerManagerTest, test_adjustVolumeOutOfSync) {
                    auto channelVolumeInterface = make_shared<NiceMock<MockChannelVolumeInterface>>();
                    channelVolumeInterface->DelegateToReal();
                    auto channelVolumeInterface2 = make_shared<NiceMock<MockChannelVolumeInterface>>();
                    EXPECT_CALL(*channelVolumeInterface2, getSpeakerType()).WillRepeatedly(Return(Type::AVS_SPEAKER_VOLUME));
                    EXPECT_CALL(*channelVolumeInterface2, setUnduckedVolume(_)).WillRepeatedly(Return(true));
                    EXPECT_CALL(*channelVolumeInterface2, getSpeakerSettings(_)).WillRepeatedly(Return(false));
                    m_speakerManager = SpeakerManager::create({channelVolumeInterface, channelVolumeInterface2},
                                                              m_mockContextManager,m_mockMessageSender,
                                                              m_mockExceptionSender,m_metricRecorder);
                    EXPECT_CALL(*m_observer, onSpeakerSettingsChanged(_, _, _)).Times(Exactly(0));
                    m_speakerManager->addSpeakerManagerObserver(m_observer);
                    SMI::NotificationProperties properties;
                    future<bool> future = m_speakerManager->adjustVolume(Type::AVS_SPEAKER_VOLUME, AVS_ADJUST_VOLUME_MAX, properties);
                    ASSERT_FALSE(future.get());
                }
                TEST_F(SpeakerManagerTest, test_eventNotSentWhenAdjustVolumeUnchanged) {
                    auto channelVolumeInterface = make_shared<NiceMock<MockChannelVolumeInterface>>();
                    channelVolumeInterface->DelegateToReal();
                    auto groupVec = vector<shared_ptr<ChannelVolumeInterface>>{channelVolumeInterface};
                    m_speakerManager = SpeakerManager::create(groupVec, m_mockContextManager, m_mockMessageSender,
                                                              m_mockExceptionSender, m_metricRecorder);
                    SpeakerSettings expectedSettings{AVS_SET_VOLUME_MIN, UNMUTE};
                    m_speakerManager->addSpeakerManagerObserver(m_observer);
                    SMI::NotificationProperties properties;
                    for (auto type : getUniqueTypes(groupVec)) {
                        EXPECT_CALL(*m_observer,onSpeakerSettingsChanged(Source::LOCAL_API, type, expectedSettings)).Times(Exactly(1));
                        if (ChannelVolumeInterface::Type::AVS_SPEAKER_VOLUME == type) {
                            EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(Exactly(0));
                            EXPECT_CALL(*m_mockContextManager, setState(VOLUME_STATE, _, StateRefreshPolicy::NEVER, _)).Times(AnyNumber());
                            EXPECT_CALL(*m_mockContextManager,setState(VOLUME_STATE, generateVolumeStateJson(expectedSettings),
                                        StateRefreshPolicy::NEVER, _)).Times(Exactly(1));
                        }
                        future<bool> future = m_speakerManager->adjustVolume(type, AVS_ADJUST_VOLUME_MIN, properties);
                        ASSERT_TRUE(future.get());
                    }
                }
                TEST_F(SpeakerManagerTest, test_eventNotSentWhenSetVolumeUnchanged) {
                    auto channelVolumeInterface = make_shared<NiceMock<MockChannelVolumeInterface>>();
                    channelVolumeInterface->DelegateToReal();
                    EXPECT_CALL(*channelVolumeInterface, setUnduckedVolume(AVS_SET_VOLUME_MIN)).Times(Exactly(1));
                    auto groupVec = vector<shared_ptr<ChannelVolumeInterface>>{channelVolumeInterface};
                    m_speakerManager = SpeakerManager::create(groupVec, m_mockContextManager, m_mockMessageSender,
                                                              m_mockExceptionSender, m_metricRecorder);
                    SpeakerSettings expectedSettings{AVS_SET_VOLUME_MIN, UNMUTE};
                    m_speakerManager->addSpeakerManagerObserver(m_observer);
                    SMI::NotificationProperties properties;
                    for (auto type : getUniqueTypes(groupVec)) {
                        EXPECT_CALL(*m_observer,onSpeakerSettingsChanged(Source::LOCAL_API, type, expectedSettings)).Times(Exactly(1));
                        if (ChannelVolumeInterface::Type::AVS_SPEAKER_VOLUME == type) {
                            EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(Exactly(0));
                            EXPECT_CALL(*m_mockContextManager, setState(VOLUME_STATE, _, StateRefreshPolicy::NEVER, _))
                                .Times(AnyNumber());
                            EXPECT_CALL(*m_mockContextManager,setState(VOLUME_STATE, generateVolumeStateJson(expectedSettings),
                                        StateRefreshPolicy::NEVER, _)).Times(Exactly(1));
                        }
                        future<bool> future = m_speakerManager->setVolume(type, AVS_SET_VOLUME_MIN, properties);
                        ASSERT_TRUE(future.get());
                    }
                }
                TEST_F(SpeakerManagerTest, test_setMuteOutOfSync) {
                    auto channelVolumeInterface = make_shared<NiceMock<MockChannelVolumeInterface>>();
                    channelVolumeInterface->DelegateToReal();
                    auto channelVolumeInterface2 = make_shared<NiceMock<MockChannelVolumeInterface>>();
                    EXPECT_CALL(*channelVolumeInterface2, getSpeakerType())
                        .WillRepeatedly(Return(ChannelVolumeInterface::Type::AVS_SPEAKER_VOLUME));
                    EXPECT_CALL(*channelVolumeInterface2, setMute(_)).WillRepeatedly(Return(true));
                    EXPECT_CALL(*channelVolumeInterface2, getSpeakerSettings(_)).WillRepeatedly(Return(false));
                    m_speakerManager = SpeakerManager::create({channelVolumeInterface, channelVolumeInterface2},
                                                              m_mockContextManager,m_mockMessageSender,
                                                              m_mockExceptionSender,m_metricRecorder);
                    EXPECT_CALL(*m_observer, onSpeakerSettingsChanged(_, _, _)).Times(Exactly(0));
                    m_speakerManager->addSpeakerManagerObserver(m_observer);
                    SMI::NotificationProperties properties;
                    future<bool> future = m_speakerManager->setMute(Type::AVS_SPEAKER_VOLUME, MUTE, properties);
                    ASSERT_FALSE(future.get());
                }
                TEST_F(SpeakerManagerTest, test_getSpeakerSettingsSpeakersOutOfSync) {
                    auto channelVolumeInterface = make_shared<NiceMock<MockChannelVolumeInterface>>();
                    channelVolumeInterface->DelegateToReal();
                    auto channelVolumeInterface2 = make_shared<NiceMock<MockChannelVolumeInterface>>();
                    EXPECT_CALL(*channelVolumeInterface2, getSpeakerType()).WillRepeatedly(Return(Type::AVS_SPEAKER_VOLUME));
                    EXPECT_CALL(*channelVolumeInterface2, getSpeakerSettings(_)).WillRepeatedly(Return(false));
                    m_speakerManager = SpeakerManager::create({channelVolumeInterface, channelVolumeInterface2},
                                                              m_mockContextManager,m_mockMessageSender,
                                                              m_mockExceptionSender,m_metricRecorder);
                    EXPECT_CALL(*m_observer, onSpeakerSettingsChanged(_, _, _)).Times(Exactly(0));
                    m_speakerManager->addSpeakerManagerObserver(m_observer);
                    SpeakerInterface::SpeakerSettings settings;
                    future<bool> future = m_speakerManager->getSpeakerSettings(ChannelVolumeInterface::Type::AVS_SPEAKER_VOLUME, &settings);
                    ASSERT_FALSE(future.get());
                }
                TEST_F(SpeakerManagerTest, test_getConfiguration) {
                    auto channelVolumeInterfaceVec = createChannelVolumeInterfaces();
                    m_speakerManager = SpeakerManager::create(channelVolumeInterfaceVec, m_mockContextManager,
                                                              m_mockMessageSender, m_mockExceptionSender,
                                                              m_metricRecorder);
                    auto configuration = m_speakerManager->getConfiguration();
                    auto audioNonBlockingPolicy = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, false);
                    ASSERT_EQ(configuration[SET_VOLUME], audioNonBlockingPolicy);
                    ASSERT_EQ(configuration[ADJUST_VOLUME], audioNonBlockingPolicy);
                    ASSERT_EQ(configuration[SET_MUTE], audioNonBlockingPolicy);
                }
                TEST_F(SpeakerManagerTest, test_addNullObserver) {
                    auto channelVolumeInterfaceVec = createChannelVolumeInterfaces();
                    m_speakerManager = SpeakerManager::create(channelVolumeInterfaceVec, m_mockContextManager,
                                                              m_mockMessageSender, m_mockExceptionSender,
                                                              m_metricRecorder);
                    m_speakerManager->addSpeakerManagerObserver(nullptr);
                    EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(Exactly(2));
                    SpeakerManagerInterface::NotificationProperties properties;
                    m_speakerManager->setVolume(Type::AVS_SPEAKER_VOLUME, AVS_SET_VOLUME_MAX, properties).wait();
                    m_speakerManager->adjustVolume(Type::AVS_SPEAKER_VOLUME, AVS_ADJUST_VOLUME_MAX, properties).wait();
                    m_speakerManager->setMute(Type::AVS_SPEAKER_VOLUME, MUTE, properties).wait();
                }
                TEST_F(SpeakerManagerTest, test_removeSpeakerManagerObserver) {
                    auto channelVolumeInterfaceVec = createChannelVolumeInterfaces();
                    EXPECT_CALL(*m_observer, onSpeakerSettingsChanged(_, _, _)).Times(Exactly(0));
                    EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(Exactly(2));
                    m_speakerManager = SpeakerManager::create(channelVolumeInterfaceVec, m_mockContextManager,
                                                              m_mockMessageSender, m_mockExceptionSender,
                                                              m_metricRecorder);
                    m_speakerManager->addSpeakerManagerObserver(m_observer);
                    m_speakerManager->removeSpeakerManagerObserver(m_observer);
                    SMI::NotificationProperties properties;
                    m_speakerManager->setVolume(Type::AVS_SPEAKER_VOLUME, AVS_SET_VOLUME_MAX, properties).wait();
                    m_speakerManager->adjustVolume(Type::AVS_SPEAKER_VOLUME, AVS_ADJUST_VOLUME_MAX, properties).wait();
                    m_speakerManager->setMute(Type::AVS_SPEAKER_VOLUME, MUTE, properties).wait();
                }
                TEST_F(SpeakerManagerTest, test_removeNullObserver) {
                    auto channelVolumeInterfaceVec = createChannelVolumeInterfaces();
                    EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(Exactly(2));
                    m_speakerManager = SpeakerManager::create(channelVolumeInterfaceVec, m_mockContextManager,
                                                              m_mockMessageSender, m_mockExceptionSender,
                                                              m_metricRecorder);
                    m_speakerManager->removeSpeakerManagerObserver(nullptr);
                    SMI::NotificationProperties properties;
                    m_speakerManager->setVolume(Type::AVS_SPEAKER_VOLUME, AVS_SET_VOLUME_MAX, properties).wait();
                    m_speakerManager->adjustVolume(Type::AVS_SPEAKER_VOLUME, AVS_ADJUST_VOLUME_MAX, properties).wait();
                    m_speakerManager->setMute(Type::AVS_SPEAKER_VOLUME, MUTE, properties).wait();
                }
                TEST_F(SpeakerManagerTest, test_retryAndApplySettingsForSetVolume) {
                    auto channelVolumeInterfaceVec = createChannelVolumeInterfaces();
                    m_speakerManager = SpeakerManager::create(channelVolumeInterfaceVec, m_mockContextManager,
                                                              m_mockMessageSender, m_mockExceptionSender,
                                                              m_metricRecorder);
                    SpeakerManagerInterface::NotificationProperties properties;
                    future<bool> future = m_speakerManager->setVolume(Type::AVS_SPEAKER_VOLUME, AVS_SET_VOLUME_MIN, properties);
                    ASSERT_TRUE(future.get());
                }
                TEST_F(SpeakerManagerTest, test_retryAndApplySettingsForAdjustVolume) {
                    auto channelVolumeInterfaceVec = createChannelVolumeInterfaces();
                    m_speakerManager = SpeakerManager::create(channelVolumeInterfaceVec, m_mockContextManager,
                                                              m_mockMessageSender, m_mockExceptionSender,
                                                              m_metricRecorder);
                    future<bool> future = m_speakerManager->adjustVolume(Type::AVS_SPEAKER_VOLUME, AVS_SET_VOLUME_MIN,
                                                                         SMI::NotificationProperties());
                    ASSERT_TRUE(future.get());
                }
                TEST_F(SpeakerManagerTest, test_retryAndApplySettingsForSetMute) {
                    auto channelVolumeInterfaceVec = createChannelVolumeInterfaces();
                    m_speakerManager = SpeakerManager::create(channelVolumeInterfaceVec, m_mockContextManager,
                                                              m_mockMessageSender, m_mockExceptionSender,
                                                              m_metricRecorder);
                    EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(Exactly(1));
                    SpeakerManagerInterface::NotificationProperties properties;
                    future<bool> future = m_speakerManager->setMute(Type::AVS_SPEAKER_VOLUME, MUTE, properties);
                    ASSERT_TRUE(future.get());
                }
            #ifndef ENABLE_MAXVOLUME_SETTING
                TEST_F(SpeakerManagerTest, test_setMaximumVolumeLimit) {
                    auto avsChannelVolumeInterface = make_shared<NiceMock<MockChannelVolumeInterface>>(Type::AVS_SPEAKER_VOLUME);
                    avsChannelVolumeInterface->DelegateToReal();
                    auto alertsChannelVolumeInterface = make_shared<NiceMock<MockChannelVolumeInterface>>(Type::AVS_SPEAKER_VOLUME);
                    alertsChannelVolumeInterface->DelegateToReal();
                    avsChannelVolumeInterface->setUnduckedVolume(VALID_MAXIMUM_VOLUME_LIMIT - 1);
                    alertsChannelVolumeInterface->setUnduckedVolume(VALID_MAXIMUM_VOLUME_LIMIT - 1);
                    EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(Exactly(1));
                    EXPECT_CALL(*avsChannelVolumeInterface, setUnduckedVolume(_)).Times(AtLeast(1));
                    EXPECT_CALL(*alertsChannelVolumeInterface, setUnduckedVolume(_)).Times(AtLeast(1));
                    EXPECT_CALL(*m_observer, onSpeakerSettingsChanged(_, _, _)).Times(0);
                    m_speakerManager = SpeakerManager::create({avsChannelVolumeInterface, alertsChannelVolumeInterface}, m_mockContextManager,
                                                              m_mockMessageSender, m_mockExceptionSender, m_metricRecorder);
                    SMI::NotificationProperties properties;
                    EXPECT_TRUE(m_speakerManager->setMaximumVolumeLimit(VALID_MAXIMUM_VOLUME_LIMIT).get());
                    EXPECT_TRUE(m_speakerManager->setVolume(Type::AVS_SPEAKER_VOLUME, VALID_MAXIMUM_VOLUME_LIMIT + 1, properties).get());
                    EXPECT_FALSE(m_speakerManager->adjustVolume(Type::AVS_SPEAKER_VOLUME, VALID_MAXIMUM_VOLUME_LIMIT + 1, properties).get());
                    EXPECT_EQ(getSpeakerVolume(avsChannelVolumeInterface), VALID_MAXIMUM_VOLUME_LIMIT);
                    EXPECT_EQ(getSpeakerVolume(alertsChannelVolumeInterface), VALID_MAXIMUM_VOLUME_LIMIT);
                    EXPECT_TRUE(m_speakerManager->adjustVolume(Type::AVS_SPEAKER_VOLUME, 2, properties).get());
                    EXPECT_EQ(getSpeakerVolume(alertsChannelVolumeInterface), VALID_MAXIMUM_VOLUME_LIMIT);
                }
                TEST_F(SpeakerManagerTest, testSetMaximumVolumeLimitWhileVolumeIsHigher) {
                    auto avsChannelVolumeInterface = make_shared<NiceMock<MockChannelVolumeInterface>>(Type::AVS_SPEAKER_VOLUME);
                    auto alertsChannelVolumeInterface = make_shared<NiceMock<MockChannelVolumeInterface>>(Type::AVS_SPEAKER_VOLUME);
                    avsChannelVolumeInterface->DelegateToReal();
                    alertsChannelVolumeInterface->DelegateToReal();
                    EXPECT_TRUE(avsChannelVolumeInterface->setUnduckedVolume(VALID_MAXIMUM_VOLUME_LIMIT + 1));
                    EXPECT_TRUE(alertsChannelVolumeInterface->setUnduckedVolume(VALID_MAXIMUM_VOLUME_LIMIT + 1));
                    EXPECT_CALL(*avsChannelVolumeInterface, setUnduckedVolume(VALID_MAXIMUM_VOLUME_LIMIT)).Times(1);
                    EXPECT_CALL(*alertsChannelVolumeInterface, setUnduckedVolume(VALID_MAXIMUM_VOLUME_LIMIT)).Times(1);
                    EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(Exactly(1));
                    m_speakerManager = SpeakerManager::create({avsChannelVolumeInterface, alertsChannelVolumeInterface}, m_mockContextManager,
                                                              m_mockMessageSender, m_mockExceptionSender, m_metricRecorder);
                    EXPECT_TRUE(m_speakerManager->setMaximumVolumeLimit(VALID_MAXIMUM_VOLUME_LIMIT).get());
                    EXPECT_EQ(getSpeakerVolume(avsChannelVolumeInterface), VALID_MAXIMUM_VOLUME_LIMIT);
                    EXPECT_EQ(getSpeakerVolume(alertsChannelVolumeInterface), VALID_MAXIMUM_VOLUME_LIMIT);
                }
                TEST_F(SpeakerManagerTest, testAVSSetVolumeHigherThanLimit) {
                    getConsoleLogger()->setLevel(Level::DEBUG9);
                    auto avsChannelVolumeInterface = make_shared<NiceMock<MockChannelVolumeInterface>>(Type::AVS_SPEAKER_VOLUME);
                    auto alertsChannelVolumeInterface = make_shared<NiceMock<MockChannelVolumeInterface>>(Type::AVS_SPEAKER_VOLUME);
                    avsChannelVolumeInterface->DelegateToReal();
                    alertsChannelVolumeInterface->DelegateToReal();
                    EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(Exactly(1));
                    EXPECT_TRUE(avsChannelVolumeInterface->setUnduckedVolume(VALID_MAXIMUM_VOLUME_LIMIT - 1));
                    EXPECT_TRUE(alertsChannelVolumeInterface->setUnduckedVolume(VALID_MAXIMUM_VOLUME_LIMIT - 1));
                    m_speakerManager = SpeakerManager::create({avsChannelVolumeInterface, alertsChannelVolumeInterface}, m_mockContextManager,
                                                              m_mockMessageSender, m_mockExceptionSender, m_metricRecorder);
                    EXPECT_TRUE(m_speakerManager->setMaximumVolumeLimit(VALID_MAXIMUM_VOLUME_LIMIT).get());
                    createAndSendVolumeDirective(SET_VOLUME.name, VALID_MAXIMUM_VOLUME_LIMIT + 1);
                    ASSERT_EQ(getSpeakerVolume(avsChannelVolumeInterface), VALID_MAXIMUM_VOLUME_LIMIT);
                    ASSERT_EQ(getSpeakerVolume(alertsChannelVolumeInterface), VALID_MAXIMUM_VOLUME_LIMIT);
                }
                TEST_F(SpeakerManagerTest, testSetMaximumVolumeLimitWithInvalidValue) {
                    auto avsChannelVolumeInterface = createChannelVolumeInterfaces();
                    m_speakerManager = SpeakerManager::create(avsChannelVolumeInterface, m_mockContextManager, m_mockMessageSender,
                                                              m_mockExceptionSender, m_metricRecorder);
                    EXPECT_FALSE(m_speakerManager->setMaximumVolumeLimit(INVALID_MAXIMUM_VOLUME_LIMIT).get());
                }
            #endif
                INSTANTIATE_TEST_CASE_P(Parameterized, SpeakerManagerTest, Values(vector<Type>{Type::AVS_SPEAKER_VOLUME}, vector<Type>{Type::AVS_ALERTS_VOLUME},
                                        vector<Type>{Type::AVS_SPEAKER_VOLUME, Type::AVS_SPEAKER_VOLUME}, vector<Type>{Type::AVS_ALERTS_VOLUME, Type::AVS_ALERTS_VOLUME,},
                                        vector<Type>{Type::AVS_SPEAKER_VOLUME, Type::AVS_ALERTS_VOLUME, Type::AVS_SPEAKER_VOLUME, Type::AVS_ALERTS_VOLUME}));
                TEST_P(SpeakerManagerTest, test_setVolume) {
                    vector<shared_ptr<ChannelVolumeInterface>> groupVec;
                    for (auto& typeOfSpeaker : GetParam()) {
                        auto group = make_shared<NiceMock<MockChannelVolumeInterface>>(typeOfSpeaker);
                        group->DelegateToReal();
                        EXPECT_CALL(*group, setUnduckedVolume(AVS_SET_VOLUME_MAX)).Times(Exactly(1));
                        groupVec.push_back(group);
                    }
                    m_speakerManager = SpeakerManager::create(groupVec, m_mockContextManager, m_mockMessageSender,
                                                              m_mockExceptionSender, m_metricRecorder);
                    SpeakerSettings expectedSettings{AVS_SET_VOLUME_MAX, UNMUTE};
                    m_speakerManager->addSpeakerManagerObserver(m_observer);
                    SMI::NotificationProperties properties(Source::DIRECTIVE);
                    for (auto type : getUniqueTypes(groupVec)) {
                        EXPECT_CALL(*m_observer,onSpeakerSettingsChanged(Source::DIRECTIVE, type, expectedSettings)).Times(Exactly(1));
                        if (ChannelVolumeInterface::Type::AVS_SPEAKER_VOLUME == type) {
                            EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(Exactly(1));
                            EXPECT_CALL(*m_mockContextManager, setState(VOLUME_STATE, _, StateRefreshPolicy::NEVER, _)).Times(AnyNumber());
                            EXPECT_CALL(*m_mockContextManager,setState(VOLUME_STATE, generateVolumeStateJson(expectedSettings),
                                        StateRefreshPolicy::NEVER, _)).Times(Exactly(1));
                        }
                        future<bool> future = m_speakerManager->setVolume(type, AVS_SET_VOLUME_MAX, properties);
                        ASSERT_TRUE(future.get());
                    }
                }
                TEST_P(SpeakerManagerTest, test_adjustVolume) {
                    vector<shared_ptr<ChannelVolumeInterface>> groupVec;
                    for (auto& typeOfSpeaker : GetParam()) {
                        auto group = make_shared<NiceMock<MockChannelVolumeInterface>>(typeOfSpeaker);
                        group->DelegateToReal();
                        groupVec.push_back(group);
                    }
                    m_speakerManager = SpeakerManager::create(groupVec, m_mockContextManager, m_mockMessageSender,
                                                              m_mockExceptionSender, m_metricRecorder);
                    SpeakerInterface::SpeakerSettings expectedSettings{AVS_SET_VOLUME_MAX, UNMUTE};
                    m_speakerManager->addSpeakerManagerObserver(m_observer);
                    SpeakerManagerInterface::NotificationProperties properties(Source::DIRECTIVE);
                    for (auto type : getUniqueTypes(groupVec)) {
                        EXPECT_CALL(*m_observer,onSpeakerSettingsChanged(Source::DIRECTIVE, type, expectedSettings)).Times(Exactly(1));
                        if (ChannelVolumeInterface::Type::AVS_SPEAKER_VOLUME == type) {
                            EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(Exactly(1));
                            EXPECT_CALL(*m_mockContextManager, setState(VOLUME_STATE, _, StateRefreshPolicy::NEVER, _)).Times(AnyNumber());
                            EXPECT_CALL(*m_mockContextManager,setState(VOLUME_STATE, generateVolumeStateJson(expectedSettings),
                                        StateRefreshPolicy::NEVER, _)).Times(Exactly(1));
                        }
                        future<bool> future = m_speakerManager->adjustVolume(type, AVS_ADJUST_VOLUME_MAX, properties);
                        ASSERT_TRUE(future.get());
                    }
                }
                TEST_P(SpeakerManagerTest, test_setMute) {
                    vector<shared_ptr<ChannelVolumeInterface>> groupVec;
                    for (auto& typeOfSpeaker : GetParam()) {
                        auto group = make_shared<NiceMock<MockChannelVolumeInterface>>(typeOfSpeaker);
                        group->DelegateToReal();
                        EXPECT_CALL(*group, setMute(MUTE)).Times(Exactly(1));
                        groupVec.push_back(group);
                    }
                    m_speakerManager = SpeakerManager::create(groupVec, m_mockContextManager, m_mockMessageSender,
                                                              m_mockExceptionSender, m_metricRecorder);
                    SpeakerSettings expectedSettings{DEFAULT_SETTINGS.volume, MUTE};
                    m_speakerManager->addSpeakerManagerObserver(m_observer);
                    SMI::NotificationProperties properties(SpeakerManagerObserverInterface::Source::DIRECTIVE);
                    for (auto type : getUniqueTypes(groupVec)) {
                        EXPECT_CALL(*m_observer,onSpeakerSettingsChanged(Source::DIRECTIVE, type, expectedSettings)).Times(Exactly(1));
                        if (ChannelVolumeInterface::Type::AVS_SPEAKER_VOLUME == type) {
                            EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(Exactly(1));
                            EXPECT_CALL(*m_mockContextManager, setState(VOLUME_STATE, _, StateRefreshPolicy::NEVER, _)).Times(AnyNumber());
                            EXPECT_CALL(*m_mockContextManager,setState(VOLUME_STATE, generateVolumeStateJson(expectedSettings),
                                        StateRefreshPolicy::NEVER, _)).Times(Exactly(1));
                        }
                        future<bool> future = m_speakerManager->setMute(type, MUTE, properties);
                        ASSERT_TRUE(future.get());
                    }
                }
                TEST_P(SpeakerManagerTest, test_getSpeakerSettings) {
                    std::vector<std::shared_ptr<ChannelVolumeInterface>> groupVec;
                    for (auto& typeOfSpeaker : GetParam()) {
                        auto group = std::make_shared<NiceMock<MockChannelVolumeInterface>>(typeOfSpeaker);
                        group->DelegateToReal();
                        groupVec.push_back(group);
                    }
                    auto uniqueTypes = getUniqueTypes(groupVec);
                    m_speakerManager = SpeakerManager::create(groupVec, m_mockContextManager, m_mockMessageSender,
                                                              m_mockExceptionSender, m_metricRecorder);
                    EXPECT_CALL(*m_observer, onSpeakerSettingsChanged(_, _, _)).Times(Exactly(0));
                    m_speakerManager->addSpeakerManagerObserver(m_observer);
                    for (auto type : uniqueTypes) {
                        SpeakerSettings settings;
                        future<bool> future = m_speakerManager->getSpeakerSettings(type, &settings);
                        ASSERT_TRUE(future.get());
                        ASSERT_EQ(settings.volume, DEFAULT_SETTINGS.volume);
                        ASSERT_EQ(settings.mute, DEFAULT_SETTINGS.mute);
                    }
                }
                TEST_P(SpeakerManagerTest, test_setVolumeDirective) {
                    vector<shared_ptr<ChannelVolumeInterface>> groupVec;
                    int eventsSent = 0;
                    SpeakerSettings expectedSettings{AVS_SET_VOLUME_MAX, UNMUTE};
                    for (auto& typeOfSpeaker : GetParam()) {
                        auto group = make_shared<NiceMock<MockChannelVolumeInterface>>(typeOfSpeaker);
                        group->DelegateToReal();
                        int timesCalled = 0;
                        if (typeOfSpeaker == ChannelVolumeInterface::Type::AVS_SPEAKER_VOLUME) timesCalled = 1;
                        SpeakerInterface::SpeakerSettings temp;
                        group->getSpeakerSettings(&temp);
                        if (temp.mute) { EXPECT_CALL(*group, setMute(UNMUTE)).Times(Exactly(timesCalled)); }
                        EXPECT_CALL(*group, setUnduckedVolume(AVS_SET_VOLUME_MAX)).Times(Exactly(timesCalled));
                        groupVec.push_back(group);
                    }
                    auto uniqueTypes = getUniqueTypes(groupVec);
                    if (uniqueTypes.count(Type::AVS_SPEAKER_VOLUME)) {
                        eventsSent = 1;
                        EXPECT_CALL(*m_observer,onSpeakerSettingsChanged(Source::DIRECTIVE, Type::AVS_SPEAKER_VOLUME,expectedSettings))
                            .Times(Exactly(1));
                        EXPECT_CALL(*m_mockContextManager, setState(VOLUME_STATE, _, StateRefreshPolicy::NEVER, _)).Times(AnyNumber());
                        EXPECT_CALL(*m_mockContextManager,setState(VOLUME_STATE, generateVolumeStateJson(expectedSettings),
                                    StateRefreshPolicy::NEVER, _)).Times(Exactly(1));
                    } else {
                        EXPECT_CALL(*m_observer, onSpeakerSettingsChanged(_, _, _)).Times(Exactly(0));
                        EXPECT_CALL(*m_mockContextManager, setState(VOLUME_STATE, _, StateRefreshPolicy::NEVER, _)).Times(Exactly(0));
                    }
                    EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(Exactly(eventsSent));
                    EXPECT_CALL(*(m_mockDirectiveHandlerResult.get()), setCompleted()).Times(1)
                        .WillOnce(InvokeWithoutArgs(this, &SpeakerManagerTest::wakeOnSetCompleted));
                    m_speakerManager = SpeakerManager::create(groupVec, m_mockContextManager, m_mockMessageSender,
                                                              m_mockExceptionSender, m_metricRecorder);
                    m_speakerManager->addSpeakerManagerObserver(m_observer);
                    auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(SET_VOLUME.nameSpace, SET_VOLUME.name, MESSAGE_ID);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, VOLUME_PAYLOAD,
                                                                              attachmentManager, "");
                    m_speakerManager->CapabilityAgent::preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    m_speakerManager->CapabilityAgent::handleDirective(MESSAGE_ID);
                    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
                }
                TEST_P(SpeakerManagerTest, test_adjustVolumeDirective) {
                    vector<shared_ptr<ChannelVolumeInterface>> groupVec;
                    int eventsSent = 0;
                    SpeakerSettings expectedSettings{AVS_SET_VOLUME_MAX, UNMUTE};
                    for (auto& typeOfSpeaker : GetParam()) {
                        auto group = make_shared<NiceMock<MockChannelVolumeInterface>>(typeOfSpeaker);
                        group->DelegateToReal();
                        int timesCalled = 0;
                        if (typeOfSpeaker == Type::AVS_SPEAKER_VOLUME) timesCalled = 1;
                        SpeakerInterface::SpeakerSettings temp;
                        group->getSpeakerSettings(&temp);
                        if (temp.mute) { EXPECT_CALL(*group, setMute(UNMUTE)).Times(Exactly(timesCalled)); }
                        EXPECT_CALL(*group, setUnduckedVolume(AVS_SET_VOLUME_MAX)).Times(Exactly(timesCalled));
                        groupVec.push_back(group);
                    }
                    auto uniqueTypes = getUniqueTypes(groupVec);
                    if (uniqueTypes.count(Type::AVS_SPEAKER_VOLUME)) {
                        eventsSent = 1;
                        EXPECT_CALL(*m_observer,onSpeakerSettingsChanged(Source::DIRECTIVE,Type::AVS_SPEAKER_VOLUME, expectedSettings))
                            .Times(Exactly(1));
                        EXPECT_CALL(*m_mockContextManager, setState(VOLUME_STATE, _, StateRefreshPolicy::NEVER, _)).Times(AnyNumber());
                        EXPECT_CALL(*m_mockContextManager,setState(VOLUME_STATE, generateVolumeStateJson(expectedSettings),
                                    StateRefreshPolicy::NEVER, _)).Times(Exactly(1));
                    } else {
                        EXPECT_CALL(*m_observer, onSpeakerSettingsChanged(_, _, _)).Times(Exactly(0));
                        EXPECT_CALL(*m_mockContextManager, setState(VOLUME_STATE, _, StateRefreshPolicy::NEVER, _)).Times(Exactly(0));
                    }
                    EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(Exactly(eventsSent));
                    EXPECT_CALL(*(m_mockDirectiveHandlerResult.get()), setCompleted()).Times(1)
                        .WillOnce(InvokeWithoutArgs(this, &SpeakerManagerTest::wakeOnSetCompleted));
                    m_speakerManager = SpeakerManager::create(groupVec, m_mockContextManager, m_mockMessageSender,
                                                              m_mockExceptionSender, m_metricRecorder);
                    m_speakerManager->addSpeakerManagerObserver(m_observer);
                    auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(ADJUST_VOLUME.nameSpace, ADJUST_VOLUME.name, MESSAGE_ID);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, VOLUME_PAYLOAD,
                                                                              attachmentManager, "");
                    m_speakerManager->CapabilityAgent::preHandleDirective(directive,move(m_mockDirectiveHandlerResult));
                    m_speakerManager->CapabilityAgent::handleDirective(MESSAGE_ID);
                    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
                }
                TEST_P(SpeakerManagerTest, test_setMuteDirective) {
                    vector<shared_ptr<ChannelVolumeInterface>> groupVec;
                    int eventsSent = 0;
                    SpeakerSettings expectedSettings{DEFAULT_SETTINGS.volume, MUTE};
                    for (auto& typeOfSpeaker : GetParam()) {
                        auto group = make_shared<NiceMock<MockChannelVolumeInterface>>(typeOfSpeaker);
                        group->DelegateToReal();
                        int timesCalled = 0;
                        if (typeOfSpeaker == Type::AVS_SPEAKER_VOLUME) timesCalled = 1;
                        EXPECT_CALL(*group, setMute(MUTE)).Times(Exactly(timesCalled));
                        groupVec.push_back(group);
                    }
                    auto uniqueTypes = getUniqueTypes(groupVec);
                    if (uniqueTypes.count(ChannelVolumeInterface::Type::AVS_SPEAKER_VOLUME)) {
                        eventsSent = 1;
                        EXPECT_CALL(*m_observer,onSpeakerSettingsChanged(Source::DIRECTIVE, Type::AVS_SPEAKER_VOLUME,expectedSettings))
                            .Times(Exactly(1));
                        EXPECT_CALL(*m_mockContextManager, setState(VOLUME_STATE, _, StateRefreshPolicy::NEVER, _)).Times(AnyNumber());
                        EXPECT_CALL(*m_mockContextManager,setState(VOLUME_STATE, generateVolumeStateJson(expectedSettings),
                                    StateRefreshPolicy::NEVER, _)).Times(Exactly(1));
                    } else {
                        EXPECT_CALL(*m_observer, onSpeakerSettingsChanged(_, _, _)).Times(Exactly(0));
                        EXPECT_CALL(*m_mockContextManager, setState(VOLUME_STATE, _, StateRefreshPolicy::NEVER, _)).Times(Exactly(0));
                    }
                    EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(Exactly(eventsSent));
                    EXPECT_CALL(*(m_mockDirectiveHandlerResult.get()), setCompleted()).Times(1)
                        .WillOnce(InvokeWithoutArgs(this, &SpeakerManagerTest::wakeOnSetCompleted));
                    m_speakerManager = SpeakerManager::create(groupVec, m_mockContextManager, m_mockMessageSender,
                                       m_mockExceptionSender, m_metricRecorder);
                    m_speakerManager->addSpeakerManagerObserver(m_observer);
                    auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(SET_MUTE.nameSpace, SET_MUTE.name, MESSAGE_ID);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, MUTE_PAYLOAD,
                                                                              attachmentManager, "");
                    m_speakerManager->CapabilityAgent::preHandleDirective(directive,move(m_mockDirectiveHandlerResult));
                    m_speakerManager->CapabilityAgent::handleDirective(MESSAGE_ID);
                    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
                }
                TEST_P(SpeakerManagerTest, test_setVolumeDirectiveWhenMuted) {
                    vector<shared_ptr<ChannelVolumeInterface>> groupVec;
                    for (auto& typeOfSpeaker : GetParam()) {
                        auto group = make_shared<NiceMock<MockChannelVolumeInterface>>(typeOfSpeaker);
                        group->DelegateToReal();
                        EXPECT_CALL(*group, setUnduckedVolume(AVS_SET_VOLUME_MIN)).Times(1);
                        EXPECT_CALL(*group, setMute(MUTE)).Times(1);
                        if (typeOfSpeaker == Type::AVS_SPEAKER_VOLUME) {
                            EXPECT_CALL(*group, setMute(UNMUTE)).Times(1);
                            EXPECT_CALL(*group, setUnduckedVolume(MIN_UNMUTE_VOLUME)).Times(1);
                        }
                        groupVec.push_back(group);
                    }
                    m_speakerManager = SpeakerManager::create(groupVec, m_mockContextManager, m_mockMessageSender,
                                                              m_mockExceptionSender, m_metricRecorder);
                    m_speakerManager->addSpeakerManagerObserver(m_observer);
                    SMI::NotificationProperties properties(Source::LOCAL_API, false, false);
                    for (auto type : getUniqueTypes(groupVec)) {
                        m_speakerManager->setVolume(type, AVS_SET_VOLUME_MIN, properties);
                    }
                    for (auto type : getUniqueTypes(groupVec)) {
                        future<bool> future = m_speakerManager->setMute(type, MUTE, properties);
                    }
                    auto uniqueTypes = getUniqueTypes(groupVec);
                    int eventsSent = 0;
                    SpeakerSettings unMuteSettings{MIN_UNMUTE_VOLUME, UNMUTE};
                    if (uniqueTypes.count(ChannelVolumeInterface::Type::AVS_SPEAKER_VOLUME)) {
                        eventsSent = 2;
                        EXPECT_CALL(*m_observer,onSpeakerSettingsChanged(Source::DIRECTIVE, Type::AVS_SPEAKER_VOLUME,
                                    SpeakerSettings{MIN_UNMUTE_VOLUME, MUTE})).Times(Exactly(1));
                        EXPECT_CALL(*m_observer,onSpeakerSettingsChanged(Source::DIRECTIVE, Type::AVS_SPEAKER_VOLUME, unMuteSettings))
                            .Times(Exactly(1));
                        EXPECT_CALL(*m_mockContextManager, setState(VOLUME_STATE, _, StateRefreshPolicy::NEVER, _)).Times(AnyNumber());
                        EXPECT_CALL(*m_mockContextManager,setState(VOLUME_STATE, generateVolumeStateJson(unMuteSettings),
                                    StateRefreshPolicy::NEVER, _)).Times(Exactly(1));
                    } else {
                        EXPECT_CALL(*m_observer, onSpeakerSettingsChanged(_, _, _)).Times(0);
                        EXPECT_CALL(*m_mockContextManager, setState(VOLUME_STATE, _, StateRefreshPolicy::NEVER, _)).Times(0);
                    }
                    EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(eventsSent);
                    EXPECT_CALL(*(m_mockDirectiveHandlerResult.get()), setCompleted()).Times(1)
                        .WillOnce(InvokeWithoutArgs(this, &SpeakerManagerTest::wakeOnSetCompleted));
                    auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(SET_MUTE.nameSpace, SET_MUTE.name, MESSAGE_ID);
                    shared_ptr<AVSDirective> directive =AVSDirective::create("", avsMessageHeader, UNMUTE_PAYLOAD,
                                                                             attachmentManager, "");
                    m_speakerManager->CapabilityAgent::preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    m_speakerManager->CapabilityAgent::handleDirective(MESSAGE_ID);
                    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
                }
            }
        }
    }
}