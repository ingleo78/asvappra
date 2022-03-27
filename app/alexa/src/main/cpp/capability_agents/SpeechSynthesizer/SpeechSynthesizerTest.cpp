#include <chrono>
#include <future>
#include <memory>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sdkinterfaces/MockExceptionEncounteredSender.h>
#include <avs/attachment/AttachmentManagerInterface.h>
#include <sdkinterfaces/MockContextManager.h>
#include <sdkinterfaces/MockFocusManager.h>
#include <sdkinterfaces/MockMessageSender.h>
#include <sdkinterfaces/MockDirectiveSequencer.h>
#include <sdkinterfaces/MockDirectiveHandlerResult.h>
#include <sdkinterfaces/MockPowerResourceManager.h>
#include <avs/attachment/AttachmentManager.h>
#include <media_player/MockMediaPlayer.h>
#include <metrics/MockMetricRecorder.h>
#include <json/JSONGenerator.h>
#include <captions/MockCaptionManager.h>
#include "SpeechSynthesizer.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace speechSynthesizer {
            namespace test {
                using namespace captions;
                using namespace logger;
                using namespace testing;
                using namespace rapidjson;
                using namespace sdkInterfaces::test;
                using namespace mediaPlayer::test;
                using namespace metrics::test;
                using namespace captions::test;
                using PowerResourceLevel = PowerResourceManagerInterface::PowerResourceLevel;
                using SpeechSynthesizerState = SpeechSynthesizerObserverInterface::SpeechSynthesizerState;
                using SourceId = MediaPlayerInterface::SourceId;
                using AttachmentType = AttachmentManager::AttachmentType;
                static const milliseconds MY_WAIT_TIMEOUT(1000);
                static const MediaPlayerState DEFAULT_MEDIA_PLAYER_STATE = {milliseconds(0)};
                static const milliseconds STATE_CHANGE_TIMEOUT(10000);
                static const string CHANNEL_NAME(FocusManagerInterface::DIALOG_CHANNEL_NAME);
                static const string NAMESPACE_SPEECH_SYNTHESIZER("SpeechSynthesizer");
                static const string NAME_SPEAK("Speak");
                static string SPEECH_STARTED_EVENT_NAME{"SpeechStarted"};
                static string SPEECH_FINISHED_EVENT_NAME{"SpeechFinished"};
                static string SPEECH_INTERRUPTED_EVENT_NAME{"SpeechInterrupted"};
                static const string NAME_RECOGNIZE("Recognize");
                static const NamespaceAndName NAMESPACE_AND_NAME_SPEECH_STATE{NAMESPACE_SPEECH_SYNTHESIZER, "SpeechState"};
                static const string MESSAGE_ID_TEST("MessageId_Test");
                static const string MESSAGE_ID_TEST_2("MessageId_Test_2");
                static const string DIALOG_REQUEST_ID_TEST("DialogRequestId_Test");
                static const string TOKEN_TEST("Token_Test");
                static const string FORMAT_TEST("AUDIO_MPEG");
                static const string URL_TEST("cid:Test");
                static const string CONTEXT_ID_TEST("ContextId_Test");
                static const string CONTEXT_ID_TEST_2("ContextId_Test_2");
                static const string CAPTION_CONTENT_SAMPLE = "WEBVTT\\n\\n1\\n00:00.000 --> 00:01.260\\nThe time is 2:17 PM.";
                static const string PAYLOAD_TEST = "{\"url\":\"" + URL_TEST + "\",\"format\":\"" + FORMAT_TEST + "\",\"token\":\""+
                                                   TOKEN_TEST + "\",\"caption\": {\"content\":\"" + CAPTION_CONTENT_SAMPLE + "\","
                                                   "\"type\":\"WEBVTT\"}}";
                static const string PAYLOAD_TEST_SINGLE_ANALYZER = "{\"url\":\"" + URL_TEST + "\",\"format\":\"" + FORMAT_TEST +
                                                                   "\",\"token\":\""+ TOKEN_TEST + "\",\"caption\": {\"content\":\"" +
                                                                   CAPTION_CONTENT_SAMPLE + "\",\"type\":\"WEBVTT\"},\"analyzers\""
                                                                   ":[{\"interface\":\"analyzername\", \"enabled\":\"YES\"}]}";
                static const string PAYLOAD_TEST_MULTIPLE_ANALYZER = "{\"url\":\"" + URL_TEST + "\",\"format\":\"" + FORMAT_TEST + "\","
                                                                     "\"token\":\""+ TOKEN_TEST + "\",\"caption\": {\"content\":\"" +
                                                                     CAPTION_CONTENT_SAMPLE + "\",\"type\":\"WEBVTT\"},\"analyzers\":["
                                                                     "{\"interface\":\"analyzername1\", \"enabled\":\"YES\"},{\"interface\""
                                                                     ":\"analyzername2\", \"enabled\":\"NO\"}]}";
                static const string FINISHED_STATE("FINISHED");
                static const string PLAYING_STATE{"PLAYING"};
                static const string INTERRUPTED_STATE{"INTERRUPTED"};
                static const long OFFSET_IN_MILLISECONDS_TEST{100};
                static const milliseconds OFFSET_IN_CHRONO_MILLISECONDS_TEST{100};
                static const string PLAYING_STATE_TEST = "{\"token\":\"" + TOKEN_TEST + "\",\"offsetInMilliseconds\":" +
                                                         to_string(OFFSET_IN_MILLISECONDS_TEST) + ",\"playerActivity\":\"" + PLAYING_STATE +
                                                         "\"}";
                static const string FINISHED_STATE_TEST = "{\"token\":\"" + TOKEN_TEST + "\",\"offsetInMilliseconds\":" + to_string(0) + ","
                                                          "\"playerActivity\":\"" + FINISHED_STATE + "\"}";
                static const string INTERRUPTED_STATE_TEST = "{\"token\":\"" + TOKEN_TEST + "\",\"offsetInMilliseconds\":" +
                                                             to_string(OFFSET_IN_MILLISECONDS_TEST) + ",\"playerActivity\":\"" +
                                                             INTERRUPTED_STATE + "\"}";
                static const string IDLE_STATE_TEST = "{\"token\":\"\",\"offsetInMilliseconds\":" + to_string(0) + ",\"playerActivity\":\"" +
                                                      FINISHED_STATE + "\"}";
                static const unsigned int PROVIDE_STATE_TOKEN_TEST{1};
                static const string COMPONENT_NAME("SpeechSynthesizer");
                struct SpeakTestInfo {
                    const string payload;
                    const string messageId;
                    const string token;
                };
                static SpeakTestInfo generateSpeakInfo(PlayBehavior playBehavior) {
                    JsonGenerator generator;
                    static int id = 0;
                    string idStr = "_" + to_string(id++);
                    string token = TOKEN_TEST + idStr;
                    generator.addMember("url", URL_TEST + idStr);
                    generator.addMember("format", FORMAT_TEST);
                    generator.addMember("playBehavior", playBehaviorToString(playBehavior));
                    generator.addMember("token", token);
                    return SpeakTestInfo{generator.toString(), (MESSAGE_ID_TEST + idStr), token};
                }
                static string generatePlayingState(const SpeakTestInfo& info) {
                    return string(PLAYING_STATE_TEST).replace(PLAYING_STATE_TEST.find(TOKEN_TEST), TOKEN_TEST.size(), info.token);
                }
                static string generateFinishedState(const SpeakTestInfo& info) {
                    return string(FINISHED_STATE_TEST).replace(FINISHED_STATE_TEST.find(TOKEN_TEST), TOKEN_TEST.size(), info.token);
                }
                static string generateInterruptedState(const SpeakTestInfo& info) {
                    return string(INTERRUPTED_STATE_TEST).replace(INTERRUPTED_STATE_TEST.find(TOKEN_TEST), TOKEN_TEST.size(), info.token);
                }
                class MockSpeechSynthesizerObserver : public SpeechSynthesizerObserverInterface {
                public:
                    MOCK_METHOD4(onStateChanged, void(SpeechSynthesizerState state, const SourceId mediaSourceId,
                                 const Optional<MediaPlayerState>& mediaPlayerState, const vector<AudioAnalyzerState>& audioAnalyzerState));
                };
                class SpeechSynthesizerTest : public Test {
                public:
                    SpeechSynthesizerTest();
                    void SetUp() override;
                    void TearDown() override;
                    shared_ptr<SpeechSynthesizer> m_speechSynthesizer;
                    shared_ptr<MockMediaPlayer> m_mockSpeechPlayer;
                    shared_ptr<MockContextManager> m_mockContextManager;
                    shared_ptr<MockSpeechSynthesizerObserver> m_mockSpeechSynthesizerObserver;
                    SetStateResult wakeOnSetState();
                    promise<void> m_wakeSetStatePromise;
                    future<void> m_wakeSetStateFuture;
                    shared_ptr<MockFocusManager> m_mockFocusManager;
                    bool wakeOnAcquireChannel();
                    promise<void> m_wakeAcquireChannelPromise;
                    future<void> m_wakeAcquireChannelFuture;
                    future<bool> wakeOnReleaseChannel();
                    promise<void> m_wakeReleaseChannelPromise;
                    future<void> m_wakeReleaseChannelFuture;
                    unique_ptr<MockDirectiveHandlerResult> m_mockDirHandlerResult;
                    void wakeOnSetCompleted();
                    promise<void> m_wakeSetCompletedPromise;
                    future<void> m_wakeSetCompletedFuture;
                    void wakeOnSetFailed();
                    promise<void> m_wakeSetFailedPromise;
                    future<void> m_wakeSetFailedFuture;
                    shared_ptr<MockMessageSender> m_mockMessageSender;
                    void wakeOnSendMessage();
                    promise<void> m_wakeSendMessagePromise;
                    future<void> m_wakeSendMessageFuture;
                    void wakeOnStopped();
                    promise<void> m_wakeStoppedPromise;
                    future<void> m_wakeStoppedFuture;
                    shared_ptr<MockExceptionEncounteredSender> m_mockExceptionSender;
                    shared_ptr<AttachmentManager> m_attachmentManager;
                    shared_ptr<MetricRecorderInterface> m_metricRecorder;
                    shared_ptr<DialogUXStateAggregator> m_dialogUXStateAggregator;
                    shared_ptr<MockCaptionManager> m_mockCaptionManager;
                    shared_ptr<MockPowerResourceManager> m_mockPowerResourceManager;
                    bool setupPendingSpeech(unique_ptr<DirectiveHandlerResultInterface> resultHandler, const SpeakTestInfo& info);
                    bool setupActiveSpeech(unique_ptr<DirectiveHandlerResultInterface> resultHandler, const SpeakTestInfo& info);
                };
                using SST = SpeechSynthesizerTest;
                SST::SpeechSynthesizerTest() : m_wakeSetStatePromise{},
                        m_wakeSetStateFuture{m_wakeSetStatePromise.get_future()},
                        m_wakeAcquireChannelPromise{},
                        m_wakeAcquireChannelFuture{m_wakeAcquireChannelPromise.get_future()},
                        m_wakeReleaseChannelPromise{},
                        m_wakeReleaseChannelFuture{m_wakeReleaseChannelPromise.get_future()},
                        m_wakeSetCompletedPromise{},
                        m_wakeSetCompletedFuture{m_wakeSetCompletedPromise.get_future()},
                        m_wakeSetFailedPromise{},
                        m_wakeSetFailedFuture{m_wakeSetFailedPromise.get_future()},
                        m_wakeSendMessagePromise{},
                        m_wakeSendMessageFuture{m_wakeSendMessagePromise.get_future()},
                        m_wakeStoppedPromise{},
                        m_wakeStoppedFuture{m_wakeStoppedPromise.get_future()} {
                }
                void SST::SetUp() {
                    m_metricRecorder = make_shared<NiceMock<MockMetricRecorder>>();
                    m_mockContextManager = make_shared<NiceMock<MockContextManager>>();
                    //m_mockFocusManager = make_shared<NiceMock<MockFocusManager>>();
                    m_mockMessageSender = make_shared<NiceMock<MockMessageSender>>();
                    m_mockExceptionSender = make_shared<NiceMock<MockExceptionEncounteredSender>>();
                    m_attachmentManager = make_shared<AttachmentManager>(AttachmentType::IN_PROCESS);
                    m_mockSpeechPlayer = MockMediaPlayer::create();
                    m_dialogUXStateAggregator = make_shared<DialogUXStateAggregator>();
                    m_mockCaptionManager = make_shared<NiceMock<MockCaptionManager>>();
                    m_mockPowerResourceManager = make_shared<MockPowerResourceManager>();
                    m_mockSpeechSynthesizerObserver = make_shared<MockSpeechSynthesizerObserver>();
                    m_speechSynthesizer = SpeechSynthesizer::create(m_mockSpeechPlayer,m_mockMessageSender,
                                                                    m_mockFocusManager,m_mockContextManager,
                                                                    m_mockExceptionSender,m_metricRecorder,
                                                                    m_dialogUXStateAggregator,m_mockCaptionManager,
                                                                    m_mockPowerResourceManager);
                    m_mockDirHandlerResult.reset(new MockDirectiveHandlerResult);
                    ASSERT_TRUE(m_speechSynthesizer);
                    m_speechSynthesizer->addObserver(m_dialogUXStateAggregator);
                }
                void SpeechSynthesizerTest::TearDown() {
                    m_speechSynthesizer->shutdown();
                    m_mockSpeechPlayer->shutdown();
                }
                SetStateResult SpeechSynthesizerTest::wakeOnSetState() {
                    m_wakeSetStatePromise.set_value();
                    return SetStateResult::SUCCESS;
                }
                bool SpeechSynthesizerTest::wakeOnAcquireChannel() {
                    m_wakeAcquireChannelPromise.set_value();
                    return true;
                }
                future<bool> SpeechSynthesizerTest::wakeOnReleaseChannel() {
                    promise<bool> releaseChannelSuccess;
                    future<bool> returnValue = releaseChannelSuccess.get_future();
                    releaseChannelSuccess.set_value(true);
                    m_wakeReleaseChannelPromise.set_value();
                    return returnValue;
                }
                void SpeechSynthesizerTest::wakeOnSetCompleted() {
                    m_wakeSetCompletedPromise.set_value();
                }
                void SpeechSynthesizerTest::wakeOnSetFailed() {
                    m_wakeSetFailedPromise.set_value();
                }
                void SpeechSynthesizerTest::wakeOnSendMessage() {
                    m_wakeSendMessagePromise.set_value();
                }
                void SpeechSynthesizerTest::wakeOnStopped() {
                    m_wakeStoppedPromise.set_value();
                }
                static bool matchEvent(shared_ptr<MessageRequest> request, const string& expectedContent) {
                    return request && (request->getJsonContent().find(expectedContent) != string::npos);
                }
                MATCHER(IsStartedEvent, "") {
                    return matchEvent(arg, SPEECH_STARTED_EVENT_NAME);
                }
                MATCHER(IsFinishedEvent, "") {
                    return matchEvent(arg, SPEECH_FINISHED_EVENT_NAME);
                }
                MATCHER(IsInterruptedEvent, "") {
                    return matchEvent(arg, SPEECH_INTERRUPTED_EVENT_NAME);
                }
                TEST_F(SpeechSynthesizerTest, test_callingHandleImmediately) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, MESSAGE_ID_TEST,
                                                                          DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, PAYLOAD_TEST,
                                                                              m_attachmentManager, CONTEXT_ID_TEST);

                    /*EXPECT_CALL(*(m_mockFocusManager.get()), acquireChannel(CHANNEL_NAME, _)).Times(1)
                        .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnAcquireChannel));
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()),attachmentSetSource(A<shared_ptr<AttachmentReader>>(), nullptr))
                        .Times(AtLeast(1));*/
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()), play(_)).Times(AtLeast(1));
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()), getOffset(_)).Times(1).WillOnce(Return(OFFSET_IN_CHRONO_MILLISECONDS_TEST));
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()), getMediaPlayerState(_)).Times(AtLeast(2));
                    EXPECT_CALL(*(m_mockContextManager.get()),setState(NAMESPACE_AND_NAME_SPEECH_STATE, PLAYING_STATE_TEST,
                                StateRefreshPolicy::ALWAYS, 0)).Times(AtLeast(1)).WillOnce(InvokeWithoutArgs(this, &SST::wakeOnSetState));
                    EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_)).Times(AtLeast(1))
                        .WillRepeatedly(InvokeWithoutArgs(this, &SST::wakeOnSendMessage));
                    //EXPECT_CALL(*m_mockCaptionManager, onCaption(_, _)).Times(1);
                    EXPECT_CALL(*m_mockPowerResourceManager, acquirePowerResource(COMPONENT_NAME, PowerResourceLevel::ACTIVE_HIGH))
                        .Times(AtLeast(1));
                    vector<AudioAnalyzerState> data;
                    EXPECT_CALL(*m_mockSpeechSynthesizerObserver,onStateChanged(SpeechSynthesizerState::GAINING_FOCUS, _, _, _)).Times(1);
                    EXPECT_CALL(*m_mockSpeechSynthesizerObserver,onStateChanged(SpeechSynthesizerState::PLAYING, _, _, Eq(data))).Times(1);
                    m_speechSynthesizer->addObserver(m_mockSpeechSynthesizerObserver);
                    m_speechSynthesizer->handleDirectiveImmediately(directive);
                    ASSERT_TRUE(future_status::ready == m_wakeAcquireChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_speechSynthesizer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                    ASSERT_TRUE(m_mockSpeechPlayer->waitUntilPlaybackStarted());
                    ASSERT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                    ASSERT_TRUE(future_status::ready == m_wakeSendMessageFuture.wait_for(MY_WAIT_TIMEOUT));
                }
                TEST_F(SpeechSynthesizerTest, test_callingHandle) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, MESSAGE_ID_TEST,
                                                                          DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, PAYLOAD_TEST,
                                                                              m_attachmentManager, CONTEXT_ID_TEST);
                    //EXPECT_CALL(*(m_mockFocusManager.get()), acquireChannel(CHANNEL_NAME, _)).Times(1).WillOnce(InvokeWithoutArgs(this, &SST::wakeOnAcquireChannel));
                    //EXPECT_CALL(*(m_mockSpeechPlayer.get()),attachmentSetSource(A<shared_ptr<AttachmentReader>>(), nullptr)).Times(AtLeast(1));
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()), play(_)).Times(AtLeast(1));
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()), getOffset(_)).Times(1).WillOnce(Return(OFFSET_IN_CHRONO_MILLISECONDS_TEST));
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()), getMediaPlayerState(_)).Times(AtLeast(2));
                    EXPECT_CALL(*(m_mockContextManager.get()),setState(NAMESPACE_AND_NAME_SPEECH_STATE, PLAYING_STATE_TEST,
                                StateRefreshPolicy::ALWAYS, 0)).Times(AtLeast(1)).WillOnce(InvokeWithoutArgs(this, &SST::wakeOnSetState));
                    EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_)).Times(AtLeast(1)).WillRepeatedly(InvokeWithoutArgs(this, &SST::wakeOnSendMessage));
                    EXPECT_CALL(*(m_mockDirHandlerResult.get()), setFailed(_)).Times(1).WillOnce(InvokeWithoutArgs(this, &SST::wakeOnSetFailed));
                    //EXPECT_CALL(*m_mockCaptionManager, onCaption(_, _)).Times(1);
                    EXPECT_CALL(*m_mockPowerResourceManager, acquirePowerResource(COMPONENT_NAME, PowerResourceLevel::ACTIVE_HIGH))
                        .Times(AtLeast(1));
                    m_speechSynthesizer->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirHandlerResult));
                    m_speechSynthesizer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                    ASSERT_TRUE(future_status::ready == m_wakeAcquireChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_speechSynthesizer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                    ASSERT_TRUE(m_mockSpeechPlayer->waitUntilPlaybackStarted());
                    ASSERT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                    ASSERT_TRUE(future_status::ready == m_wakeSendMessageFuture.wait_for(MY_WAIT_TIMEOUT));
                }
                TEST_F(SpeechSynthesizerTest, test_callingCancel) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, MESSAGE_ID_TEST,
                                                                          DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, PAYLOAD_TEST,
                                                                              m_attachmentManager, CONTEXT_ID_TEST);
                    EXPECT_CALL(*(m_mockContextManager.get()), setState(_, _, _, _)).Times(0);
                    EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_)).Times(0);
                    m_speechSynthesizer->CapabilityAgent::preHandleDirective(directive, move(m_mockDirHandlerResult));
                    m_speechSynthesizer->CapabilityAgent::cancelDirective(MESSAGE_ID_TEST);
                }
                TEST_F(SpeechSynthesizerTest, test_callingCancelAfterHandle) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, MESSAGE_ID_TEST,
                                                                          DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, PAYLOAD_TEST,
                                                                              m_attachmentManager, CONTEXT_ID_TEST);
                    /*EXPECT_CALL(*(m_mockFocusManager.get()), acquireChannel(CHANNEL_NAME, _)).Times(1)
                        .WillOnce(InvokeWithoutArgs(this, &SST::wakeOnAcquireChannel));
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()),attachmentSetSource(A<shared_ptr<AttachmentReader>>(), nullptr))
                        .Times(AtLeast(1));*/
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()), play(_)).Times(AtLeast(1));
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()), getOffset(_)).Times(AtLeast(2))
                        .WillRepeatedly(Return(OFFSET_IN_CHRONO_MILLISECONDS_TEST));
                    EXPECT_CALL(*(m_mockContextManager.get()),setState(NAMESPACE_AND_NAME_SPEECH_STATE, PLAYING_STATE_TEST,
                                StateRefreshPolicy::ALWAYS, 0)).Times(AtLeast(1)).WillRepeatedly(InvokeWithoutArgs(this, &SST::wakeOnSetState));
                    EXPECT_CALL(*(m_mockContextManager.get()),setState(NAMESPACE_AND_NAME_SPEECH_STATE, INTERRUPTED_STATE_TEST,
                                StateRefreshPolicy::NEVER, 0)).Times(AtLeast(1)).WillRepeatedly(InvokeWithoutArgs(this, &SST::wakeOnSetState));
                    EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(IsStartedEvent())).WillOnce(InvokeWithoutArgs(this, &SST::wakeOnSendMessage));
                    /*EXPECT_CALL(*(m_mockFocusManager.get()), releaseChannel(CHANNEL_NAME, _)).Times(1)
                        .WillOnce(InvokeWithoutArgs(this, &ST::wakeOnReleaseChannel));*/
                    EXPECT_CALL(*(m_mockDirHandlerResult.get()), setFailed(_)).Times(1)
                        .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnSetFailed));
                    EXPECT_CALL(*m_mockPowerResourceManager, acquirePowerResource(COMPONENT_NAME, PowerResourceLevel::ACTIVE_HIGH))
                        .Times(AtLeast(1));
                    m_speechSynthesizer->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirHandlerResult));
                    m_speechSynthesizer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                    ASSERT_TRUE(future_status::ready == m_wakeAcquireChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_speechSynthesizer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                    ASSERT_TRUE(m_mockSpeechPlayer->waitUntilPlaybackStarted());
                    ASSERT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_wakeSetStatePromise = promise<void>();
                    m_wakeSetStateFuture = m_wakeSetStatePromise.get_future();
                    ASSERT_TRUE(future_status::ready == m_wakeSendMessageFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_wakeSendMessagePromise = promise<void>();
                    m_wakeSendMessageFuture = m_wakeSendMessagePromise.get_future();
                    m_speechSynthesizer->CapabilityAgent::cancelDirective(MESSAGE_ID_TEST);
                    EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(IsInterruptedEvent()))
                        .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnSendMessage));
                    EXPECT_CALL(*m_mockPowerResourceManager, releasePowerResource(COMPONENT_NAME)).Times(AtLeast(1));
                    ASSERT_TRUE(m_mockSpeechPlayer->waitUntilPlaybackStopped());
                    ASSERT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                    ASSERT_TRUE(future_status::ready == m_wakeReleaseChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                    ASSERT_TRUE(future_status::ready == m_wakeSendMessageFuture.wait_for(MY_WAIT_TIMEOUT));
                }
                TEST_F(SpeechSynthesizerTest, test_callingProvideStateWhenNotPlaying) {
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()), getOffset(_)).Times(0);
                    EXPECT_CALL(*(m_mockContextManager.get()),setState(NAMESPACE_AND_NAME_SPEECH_STATE, IDLE_STATE_TEST,
                                StateRefreshPolicy::NEVER, PROVIDE_STATE_TOKEN_TEST)).Times(1).WillOnce(InvokeWithoutArgs(this, &SST::wakeOnSetState));
                    m_speechSynthesizer->provideState(NAMESPACE_AND_NAME_SPEECH_STATE, PROVIDE_STATE_TOKEN_TEST);
                    ASSERT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                }
                TEST_F(SpeechSynthesizerTest, test_callingProvideStateWhenPlaying) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, PAYLOAD_TEST,
                                                                              m_attachmentManager, CONTEXT_ID_TEST);
                    //EXPECT_CALL(*(m_mockFocusManager.get()), acquireChannel(CHANNEL_NAME, _)).Times(1)
                    //    .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnAcquireChannel));
                    //EXPECT_CALL(*(m_mockSpeechPlayer.get()),attachmentSetSource(A<shared_ptr<AttachmentReader>>(), nullptr)).Times(AtLeast(1));
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()), play(_)).Times(AtLeast(1));
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()), getOffset(_)).Times(AtLeast(1)).WillRepeatedly(Return(OFFSET_IN_CHRONO_MILLISECONDS_TEST));
                    EXPECT_CALL(*(m_mockContextManager.get()),setState(NAMESPACE_AND_NAME_SPEECH_STATE, PLAYING_STATE_TEST, StateRefreshPolicy::ALWAYS, 0))
                        .Times(1).WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnSetState));
                    EXPECT_CALL(*(m_mockContextManager.get()),setState(NAMESPACE_AND_NAME_SPEECH_STATE, PLAYING_STATE_TEST,
                                StateRefreshPolicy::ALWAYS, PROVIDE_STATE_TOKEN_TEST)).Times(1).WillOnce(InvokeWithoutArgs(this, &SST::wakeOnSetState));
                    EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_)).Times(AtLeast(1))
                        .WillRepeatedly(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnSendMessage));
                    EXPECT_CALL(*m_mockPowerResourceManager, acquirePowerResource(COMPONENT_NAME, PowerResourceLevel::ACTIVE_HIGH))
                        .Times(AtLeast(1));
                    m_speechSynthesizer->CapabilityAgent::preHandleDirective(directive, move(m_mockDirHandlerResult));
                    m_speechSynthesizer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                    ASSERT_TRUE(future_status::ready == m_wakeAcquireChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_speechSynthesizer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                    ASSERT_TRUE(m_mockSpeechPlayer->waitUntilPlaybackStarted());
                    ASSERT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_wakeSetStatePromise = promise<void>();
                    m_wakeSetStateFuture = m_wakeSetStatePromise.get_future();
                    ASSERT_TRUE(future_status::ready == m_wakeSendMessageFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_speechSynthesizer->provideState(NAMESPACE_AND_NAME_SPEECH_STATE, PROVIDE_STATE_TOKEN_TEST);
                    ASSERT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                }
                TEST_F(SpeechSynthesizerTest, testTimer_bargeInWhilePlaying) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, MESSAGE_ID_TEST,
                                                                          DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, PAYLOAD_TEST,
                                                                              m_attachmentManager, CONTEXT_ID_TEST);

                    auto avsMessageHeader2 = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, MESSAGE_ID_TEST_2);
                    shared_ptr<AVSDirective> directive2 = AVSDirective::create("", avsMessageHeader2, PAYLOAD_TEST,
                                                                               m_attachmentManager, CONTEXT_ID_TEST_2);
                    /*EXPECT_CALL(*(m_mockFocusManager.get()), acquireChannel(CHANNEL_NAME, _)).Times(AtLeast(1))
                        .WillRepeatedly(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnAcquireChannel));
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()),attachmentSetSource(A<shared_ptr<AttachmentReader>>(), nullptr))
                        .Times(AtLeast(1));*/
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()), play(_)).Times(AtLeast(1));
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()), getOffset(_)).Times(AtLeast(2))
                        .WillRepeatedly(Return(OFFSET_IN_CHRONO_MILLISECONDS_TEST));
                    EXPECT_CALL(*(m_mockContextManager.get()),setState(NAMESPACE_AND_NAME_SPEECH_STATE, PLAYING_STATE_TEST,
                                StateRefreshPolicy::ALWAYS, 0)).Times(AtLeast(1)).WillRepeatedly(InvokeWithoutArgs(this, &SST::wakeOnSetState));
                    EXPECT_CALL(*(m_mockContextManager.get()),setState(NAMESPACE_AND_NAME_SPEECH_STATE, INTERRUPTED_STATE_TEST,
                                StateRefreshPolicy::NEVER, 0)).Times(AtLeast(1)).WillRepeatedly(InvokeWithoutArgs(this, &SST::wakeOnSetState));
                    EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(IsStartedEvent())).Times(1)
                        .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnSendMessage));
                    EXPECT_CALL(*(m_mockDirHandlerResult.get()), setFailed(_)).Times(1)
                        .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnSetFailed));
                    EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(IsInterruptedEvent())).Times(1)
                        .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnSendMessage));
                    /*EXPECT_CALL(*(m_mockFocusManager.get()), releaseChannel(CHANNEL_NAME, _)).Times(1)
                        .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnReleaseChannel));*/
                    EXPECT_CALL(*m_mockPowerResourceManager, acquirePowerResource(COMPONENT_NAME, PowerResourceLevel::ACTIVE_HIGH))
                        .Times(AtLeast(1));
                    m_speechSynthesizer->CapabilityAgent::preHandleDirective(directive, move(m_mockDirHandlerResult));
                    m_speechSynthesizer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                    ASSERT_TRUE(future_status::ready == m_wakeAcquireChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_wakeAcquireChannelPromise = promise<void>();
                    m_wakeAcquireChannelFuture = m_wakeAcquireChannelPromise.get_future();
                    m_speechSynthesizer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                    ASSERT_TRUE(m_mockSpeechPlayer->waitUntilPlaybackStarted());
                    ASSERT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_wakeSetStatePromise = promise<void>();
                    m_wakeSetStateFuture = m_wakeSetStatePromise.get_future();
                    ASSERT_TRUE(future_status::ready == m_wakeSendMessageFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_wakeSendMessagePromise = promise<void>();
                    m_wakeSendMessageFuture = m_wakeSendMessagePromise.get_future();
                    EXPECT_CALL(*m_mockPowerResourceManager, releasePowerResource(COMPONENT_NAME)).Times(AtLeast(1));
                    m_speechSynthesizer->CapabilityAgent::cancelDirective(MESSAGE_ID_TEST);
                    m_speechSynthesizer->handleDirectiveImmediately(directive2);
                    ASSERT_TRUE(m_mockSpeechPlayer->waitUntilPlaybackStopped());
                    ASSERT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                    ASSERT_TRUE(future_status::ready == m_wakeReleaseChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                    ASSERT_TRUE(future_status::ready == m_wakeAcquireChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                    ASSERT_TRUE(future_status::ready == m_wakeSendMessageFuture.wait_for(MY_WAIT_TIMEOUT));
                }
                TEST_F(SpeechSynthesizerTest, testTimer_notCallStopTwice) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, PAYLOAD_TEST,
                                                                              m_attachmentManager, CONTEXT_ID_TEST);
                    auto avsMessageHeader2 = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, MESSAGE_ID_TEST_2);
                    shared_ptr<AVSDirective> directive2 = AVSDirective::create("", avsMessageHeader2, PAYLOAD_TEST,
                                                                               m_attachmentManager, CONTEXT_ID_TEST_2);
                    /*EXPECT_CALL(*(m_mockFocusManager.get()), acquireChannel(CHANNEL_NAME, _)).Times(AtLeast(1))
                        .WillRepeatedly(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnAcquireChannel));
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()),attachmentSetSource(A<shared_ptr<AttachmentReader>>(), nullptr))
                        .Times(AtLeast(1));*/
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()), play(_)).Times(AtLeast(1));
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()), getOffset(_)).Times(AtLeast(1))
                        .WillRepeatedly(Return(OFFSET_IN_CHRONO_MILLISECONDS_TEST));
                    EXPECT_CALL(*(m_mockContextManager.get()),setState(NAMESPACE_AND_NAME_SPEECH_STATE, PLAYING_STATE_TEST,
                                StateRefreshPolicy::ALWAYS, 0)).Times(AtLeast(1)).WillRepeatedly(InvokeWithoutArgs(this, &SST::wakeOnSetState));
                    EXPECT_CALL(*(m_mockContextManager.get()),setState(NAMESPACE_AND_NAME_SPEECH_STATE, INTERRUPTED_STATE_TEST, StateRefreshPolicy::NEVER, 0))
                        .Times(AtLeast(1)).WillRepeatedly(InvokeWithoutArgs(this, &SST::wakeOnSetState));
                    EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(IsStartedEvent())).Times(AtLeast(1))
                        .WillRepeatedly(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnSendMessage));
                    EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(IsInterruptedEvent())).Times(AtLeast(1));
                    /*EXPECT_CALL(*(m_mockFocusManager.get()), releaseChannel(CHANNEL_NAME, _)).Times(AtLeast(1))
                        .WillRepeatedly(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnReleaseChannel));*/
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()), stop(_)).Times(1)
                        .WillOnce(Invoke([this](MediaPlayerInterface::SourceId id) {
                            wakeOnStopped();
                            m_speechSynthesizer->onPlaybackStopped(id, DEFAULT_MEDIA_PLAYER_STATE);
                            return true;
                        })).WillRepeatedly(Return(true));
                    EXPECT_CALL(*(m_mockDirHandlerResult.get()), setCompleted()).Times(AtLeast(0));
                    EXPECT_CALL(*m_mockPowerResourceManager, acquirePowerResource(COMPONENT_NAME, PowerResourceLevel::ACTIVE_HIGH))
                        .Times(AtLeast(1));
                    m_speechSynthesizer->CapabilityAgent::preHandleDirective(directive, move(m_mockDirHandlerResult));
                    m_speechSynthesizer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                    ASSERT_TRUE(future_status::ready == m_wakeAcquireChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_wakeAcquireChannelPromise = promise<void>();
                    m_wakeAcquireChannelFuture = m_wakeAcquireChannelPromise.get_future();
                    m_speechSynthesizer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                    ASSERT_TRUE(m_mockSpeechPlayer->waitUntilPlaybackStarted());
                    ASSERT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_wakeSetStatePromise = promise<void>();
                    m_wakeSetStateFuture = m_wakeSetStatePromise.get_future();
                    ASSERT_TRUE(future_status::ready == m_wakeSendMessageFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_wakeSendMessagePromise = promise<void>();
                    m_wakeSendMessageFuture = m_wakeSendMessagePromise.get_future();
                    EXPECT_CALL(*m_mockPowerResourceManager, releasePowerResource(COMPONENT_NAME)).Times(AtLeast(1));
                    m_speechSynthesizer->CapabilityAgent::cancelDirective(MESSAGE_ID_TEST);
                    ASSERT_TRUE(future_status::ready == m_wakeStoppedFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_speechSynthesizer->onFocusChanged(FocusState::BACKGROUND, MixingBehavior::MUST_PAUSE);
                    ASSERT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_wakeSetStatePromise = promise<void>();
                    m_wakeSetStateFuture = m_wakeSetStatePromise.get_future();
                    ASSERT_TRUE(future_status::ready == m_wakeReleaseChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_wakeReleaseChannelPromise = promise<void>();
                    m_wakeReleaseChannelFuture = m_wakeReleaseChannelPromise.get_future();
                    EXPECT_CALL(*m_mockPowerResourceManager, acquirePowerResource(COMPONENT_NAME, PowerResourceLevel::ACTIVE_HIGH))
                        .Times(AtLeast(1));
                    m_speechSynthesizer->handleDirectiveImmediately(directive2);
                    ASSERT_TRUE(future_status::ready == m_wakeAcquireChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_speechSynthesizer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                    ASSERT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_wakeSetStatePromise = promise<void>();
                    m_wakeSetStateFuture = m_wakeSetStatePromise.get_future();
                    ASSERT_TRUE(m_mockSpeechPlayer->waitUntilPlaybackStarted());
                }
                TEST_F(SpeechSynthesizerTest, testSlow_callingCancelBeforeOnFocusChanged) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, PAYLOAD_TEST,
                                                                              m_attachmentManager, CONTEXT_ID_TEST);
                    auto avsMessageHeader2 = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, MESSAGE_ID_TEST_2);
                    shared_ptr<AVSDirective> directive2 = AVSDirective::create("", avsMessageHeader2, PAYLOAD_TEST,
                                                                               m_attachmentManager, CONTEXT_ID_TEST_2);
                    //EXPECT_CALL(*m_mockFocusManager, acquireChannel(CHANNEL_NAME, _))
                    //    .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnAcquireChannel));
                    m_speechSynthesizer->CapabilityAgent::preHandleDirective(directive, move(m_mockDirHandlerResult));
                    m_speechSynthesizer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                    ASSERT_TRUE(future_status::ready == m_wakeAcquireChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_wakeAcquireChannelPromise = promise<void>();
                    m_wakeAcquireChannelFuture = m_wakeAcquireChannelPromise.get_future();
                    //EXPECT_CALL(*(m_mockFocusManager.get()), releaseChannel(CHANNEL_NAME, _)).Times(AtLeast(1))
                    //    .WillRepeatedly(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnReleaseChannel));
                    m_speechSynthesizer->CapabilityAgent::cancelDirective(MESSAGE_ID_TEST);
                    ASSERT_TRUE(future_status::ready == m_wakeReleaseChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_wakeReleaseChannelPromise = promise<void>();
                    m_wakeReleaseChannelFuture = m_wakeReleaseChannelPromise.get_future();
                    m_speechSynthesizer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                    m_speechSynthesizer->onFocusChanged(FocusState::NONE, MixingBehavior::MUST_STOP);
                    //EXPECT_CALL(*m_mockFocusManager, acquireChannel(CHANNEL_NAME, _))
                    //    .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnAcquireChannel));
                    EXPECT_CALL(*m_mockContextManager,setState(NAMESPACE_AND_NAME_SPEECH_STATE, PLAYING_STATE_TEST, StateRefreshPolicy::ALWAYS, 0))
                        .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnSetState));
                    //EXPECT_CALL(*m_mockSpeechPlayer,attachmentSetSource(A<shared_ptr<AttachmentReader>>(), nullptr));
                    EXPECT_CALL(*m_mockSpeechPlayer, play(_));
                    EXPECT_CALL(*m_mockSpeechPlayer, getOffset(_)).WillRepeatedly(Return(OFFSET_IN_CHRONO_MILLISECONDS_TEST));
                    EXPECT_CALL(*m_mockPowerResourceManager, acquirePowerResource(COMPONENT_NAME, PowerResourceLevel::ACTIVE_HIGH))
                        .Times(AtLeast(1));
                    m_speechSynthesizer->handleDirectiveImmediately(directive2);
                    ASSERT_TRUE(future_status::ready == m_wakeAcquireChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_speechSynthesizer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                    ASSERT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                    ASSERT_TRUE(m_mockSpeechPlayer->waitUntilPlaybackStarted());
                }
                TEST_F(SpeechSynthesizerTest, test_callingCancelBeforeOnExecuteStateChanged) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, PAYLOAD_TEST,
                                                                              m_attachmentManager, CONTEXT_ID_TEST);
                    auto avsMessageHeader2 = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, MESSAGE_ID_TEST_2);
                    shared_ptr<AVSDirective> directive2 = AVSDirective::create("", avsMessageHeader2, PAYLOAD_TEST,
                                                                               m_attachmentManager, CONTEXT_ID_TEST_2);
                    //EXPECT_CALL(*m_mockFocusManager, acquireChannel(CHANNEL_NAME, _))
                    //    .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnAcquireChannel));
                    m_speechSynthesizer->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirHandlerResult));
                    m_speechSynthesizer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                    ASSERT_TRUE(future_status::ready == m_wakeAcquireChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_wakeAcquireChannelPromise = std::promise<void>();
                    m_wakeAcquireChannelFuture = m_wakeAcquireChannelPromise.get_future();
                    m_speechSynthesizer->CapabilityAgent::cancelDirective(MESSAGE_ID_TEST);
                    m_speechSynthesizer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                    //EXPECT_CALL(*m_mockFocusManager, acquireChannel(CHANNEL_NAME, _))
                    //    .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnAcquireChannel));
                    EXPECT_CALL(*m_mockContextManager,setState(NAMESPACE_AND_NAME_SPEECH_STATE, PLAYING_STATE_TEST, StateRefreshPolicy::ALWAYS, 0))
                        .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnSetState));
                    //EXPECT_CALL(*m_mockSpeechPlayer,attachmentSetSource(A<shared_ptr<AttachmentReader>>(), nullptr));
                    EXPECT_CALL(*m_mockSpeechPlayer, play(_));
                    EXPECT_CALL(*m_mockSpeechPlayer, getOffset(_)).WillRepeatedly(Return(OFFSET_IN_CHRONO_MILLISECONDS_TEST));
                    EXPECT_CALL(*m_mockPowerResourceManager, acquirePowerResource(COMPONENT_NAME, PowerResourceLevel::ACTIVE_HIGH))
                        .Times(AtLeast(1));
                    m_speechSynthesizer->handleDirectiveImmediately(directive2);
                    ASSERT_TRUE(future_status::ready == m_wakeAcquireChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_speechSynthesizer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                    ASSERT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                    ASSERT_TRUE(m_mockSpeechPlayer->waitUntilPlaybackStarted());
                }
                TEST_F(SpeechSynthesizerTest, test_mediaPlayerFailedToStop) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, PAYLOAD_TEST,
                                                                              m_attachmentManager, CONTEXT_ID_TEST);
                    auto avsMessageHeader2 = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, MESSAGE_ID_TEST_2);
                    shared_ptr<AVSDirective> directive2 = AVSDirective::create("", avsMessageHeader2, PAYLOAD_TEST,
                                                                               m_attachmentManager, CONTEXT_ID_TEST_2);
                    /*EXPECT_CALL(*(m_mockFocusManager.get()), acquireChannel(CHANNEL_NAME, _)).Times(AtLeast(1))
                        .WillRepeatedly(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnAcquireChannel));
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()),attachmentSetSource(A<shared_ptr<AttachmentReader>>(), nullptr))
                        .Times(AtLeast(1));*/
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()), play(_)).Times(AtLeast(1));
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()), getOffset(_)).Times(AtLeast(1))
                        .WillRepeatedly(Return(OFFSET_IN_CHRONO_MILLISECONDS_TEST));
                    EXPECT_CALL(*(m_mockContextManager.get()),setState(NAMESPACE_AND_NAME_SPEECH_STATE, PLAYING_STATE_TEST,
                                StateRefreshPolicy::ALWAYS, 0)).Times(AtLeast(1)).WillRepeatedly(InvokeWithoutArgs(this, &SST::wakeOnSetState));
                    EXPECT_CALL(*(m_mockContextManager.get()),setState(NAMESPACE_AND_NAME_SPEECH_STATE, INTERRUPTED_STATE_TEST,
                                StateRefreshPolicy::NEVER, 0)).Times(AtLeast(1)).WillRepeatedly(InvokeWithoutArgs(this, &SST::wakeOnSetState));
                    EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(IsStartedEvent())).Times(AtLeast(1))
                        .WillRepeatedly(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnSendMessage));
                    //EXPECT_CALL(*(m_mockFocusManager.get()), releaseChannel(CHANNEL_NAME, _)).Times(AtLeast(1))
                    //    .WillRepeatedly(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnReleaseChannel));
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()), stop(_))
                        .WillOnce(Invoke([this](MediaPlayerInterface::SourceId id) {
                            wakeOnStopped();
                            return false;
                        }));
                    EXPECT_CALL(*(m_mockDirHandlerResult.get()), setFailed(_)).Times(AtLeast(0));
                    EXPECT_CALL(*m_mockPowerResourceManager, acquirePowerResource(COMPONENT_NAME, PowerResourceLevel::ACTIVE_HIGH))
                        .Times(AtLeast(1));
                    m_speechSynthesizer->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirHandlerResult));
                    m_speechSynthesizer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                    ASSERT_TRUE(future_status::ready == m_wakeAcquireChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_wakeAcquireChannelPromise = promise<void>();
                    m_wakeAcquireChannelFuture = m_wakeAcquireChannelPromise.get_future();
                    m_speechSynthesizer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                    ASSERT_TRUE(m_mockSpeechPlayer->waitUntilPlaybackStarted());
                    ASSERT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_wakeSetStatePromise = promise<void>();
                    m_wakeSetStateFuture = m_wakeSetStatePromise.get_future();
                    ASSERT_TRUE(future_status::ready == m_wakeSendMessageFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_wakeSendMessagePromise = promise<void>();
                    m_wakeSendMessageFuture = m_wakeSendMessagePromise.get_future();
                    EXPECT_CALL(*m_mockPowerResourceManager, releasePowerResource(COMPONENT_NAME)).Times(AtLeast(1));
                    m_speechSynthesizer->CapabilityAgent::cancelDirective(MESSAGE_ID_TEST);
                    ASSERT_TRUE(future_status::ready == m_wakeStoppedFuture.wait_for(MY_WAIT_TIMEOUT));
                    ASSERT_TRUE(future_status::ready == m_wakeReleaseChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_speechSynthesizer->onFocusChanged(FocusState::BACKGROUND, MixingBehavior::MUST_PAUSE);
                    ASSERT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_wakeSetStatePromise = promise<void>();
                    m_wakeSetStateFuture = m_wakeSetStatePromise.get_future();
                    ASSERT_TRUE(future_status::ready == m_wakeReleaseChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_wakeReleaseChannelPromise = promise<void>();
                    m_wakeReleaseChannelFuture = m_wakeReleaseChannelPromise.get_future();
                    EXPECT_CALL(*m_mockPowerResourceManager, acquirePowerResource(COMPONENT_NAME, PowerResourceLevel::ACTIVE_HIGH))
                        .Times(AtLeast(1));
                    m_speechSynthesizer->handleDirectiveImmediately(directive2);
                    ASSERT_TRUE(future_status::ready == m_wakeAcquireChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_speechSynthesizer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                    ASSERT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_wakeSetStatePromise = promise<void>();
                    m_wakeSetStateFuture = m_wakeSetStatePromise.get_future();
                    ASSERT_TRUE(m_mockSpeechPlayer->waitUntilPlaybackStarted());
                }
                TEST_F(SpeechSynthesizerTest, testTimer_mediaPlayerAlwaysFailToStop) {
                    auto speechSynthesizer = SpeechSynthesizer::create(m_mockSpeechPlayer,m_mockMessageSender,
                                                                       m_mockFocusManager,m_mockContextManager,
                                                                       m_mockExceptionSender,m_metricRecorder,
                                                                       m_dialogUXStateAggregator,m_mockCaptionManager);
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, MESSAGE_ID_TEST,
                                                                          DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, PAYLOAD_TEST,
                                                                              m_attachmentManager, CONTEXT_ID_TEST);

                    /*EXPECT_CALL(*m_mockFocusManager, acquireChannel(_, _)).Times(AtLeast(1))
                        .WillRepeatedly(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnAcquireChannel));
                    EXPECT_CALL(*m_mockSpeechPlayer, attachmentSetSource(_, _)).Times(AtLeast(1));*/
                    EXPECT_CALL(*m_mockSpeechPlayer, play(_)).Times(AtLeast(1));
                    EXPECT_CALL(*m_mockSpeechPlayer, getOffset(_)).WillRepeatedly(Return(OFFSET_IN_CHRONO_MILLISECONDS_TEST));
                    EXPECT_CALL(*m_mockContextManager, setState(_, _, _, _)).Times(AtLeast(1));
                    EXPECT_CALL(*m_mockMessageSender, sendMessage(_));
                    //EXPECT_CALL(*m_mockFocusManager, releaseChannel(_, _)).Times(AtLeast(1));
                    EXPECT_CALL(*m_mockSpeechPlayer, stop(_)).WillRepeatedly(Return(false));
                    EXPECT_CALL(*m_mockDirHandlerResult, setFailed(_));
                    speechSynthesizer->CapabilityAgent::preHandleDirective(directive, move(m_mockDirHandlerResult));
                    speechSynthesizer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                    EXPECT_TRUE(future_status::ready == m_wakeAcquireChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                    speechSynthesizer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                    ASSERT_TRUE(m_mockSpeechPlayer->waitUntilPlaybackStarted());
                    speechSynthesizer->shutdown();
                    speechSynthesizer.reset();
                }
                TEST_F(SpeechSynthesizerTest, testSlow_setStateTimeout) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, MESSAGE_ID_TEST,
                                                                          DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, PAYLOAD_TEST,
                                                                              m_attachmentManager, CONTEXT_ID_TEST);
                    /*EXPECT_CALL(*(m_mockFocusManager.get()), acquireChannel(CHANNEL_NAME, _)).Times(AtLeast(1))
                        .WillRepeatedly(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnAcquireChannel));
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()),attachmentSetSource(A<shared_ptr<AttachmentReader>>(), nullptr))
                        .Times(AtLeast(1));*/
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()), play(_)).Times(1).WillOnce(Return(true));
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()), getOffset(_)).Times(AtLeast(1))
                        .WillRepeatedly(Return(OFFSET_IN_CHRONO_MILLISECONDS_TEST));
                    EXPECT_CALL(*(m_mockExceptionSender.get()), sendExceptionEncountered(_, _, _)).Times(1);
                    EXPECT_CALL(*(m_mockContextManager.get()),setState(NAMESPACE_AND_NAME_SPEECH_STATE, PLAYING_STATE_TEST,
                                StateRefreshPolicy::ALWAYS, 0)).Times(AtLeast(1)).WillRepeatedly(InvokeWithoutArgs(this, &SST::wakeOnSetState));
                    EXPECT_CALL(*(m_mockContextManager.get()),setState(NAMESPACE_AND_NAME_SPEECH_STATE, FINISHED_STATE_TEST,
                                StateRefreshPolicy::NEVER, 0)).Times(AtLeast(1)).WillRepeatedly(InvokeWithoutArgs(this, &SST::wakeOnSetState));
                    EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_)).Times(0);
                    //EXPECT_CALL(*(m_mockFocusManager.get()), releaseChannel(CHANNEL_NAME, _)).Times(AtLeast(1))
                    //    .WillRepeatedly(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnReleaseChannel));
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()), stop(_)).Times(1).WillOnce(Return(true));
                    EXPECT_CALL(*(m_mockDirHandlerResult.get()), setFailed(_)).Times(1)
                        .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnSetFailed));
                    EXPECT_CALL(*m_mockPowerResourceManager, acquirePowerResource(COMPONENT_NAME, PowerResourceLevel::ACTIVE_HIGH))
                        .Times(AtLeast(1));
                    EXPECT_CALL(*m_mockPowerResourceManager, releasePowerResource(COMPONENT_NAME)).Times(AtLeast(1));
                    m_speechSynthesizer->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirHandlerResult));
                    m_speechSynthesizer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                    ASSERT_TRUE(future_status::ready == m_wakeAcquireChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_wakeAcquireChannelPromise = std::promise<void>();
                    m_wakeAcquireChannelFuture = m_wakeAcquireChannelPromise.get_future();
                    m_speechSynthesizer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                    ASSERT_TRUE(future_status::ready == m_wakeSetFailedFuture.wait_for(STATE_CHANGE_TIMEOUT));
                    m_speechSynthesizer->onPlaybackStarted(m_mockSpeechPlayer->getCurrentSourceId(), DEFAULT_MEDIA_PLAYER_STATE);
                    ASSERT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_wakeSetStatePromise = promise<void>();
                    m_wakeSetStateFuture = m_wakeSetStatePromise.get_future();
                    m_speechSynthesizer->onPlaybackStopped(m_mockSpeechPlayer->getCurrentSourceId(), DEFAULT_MEDIA_PLAYER_STATE);
                    ASSERT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_wakeSetStatePromise = promise<void>();
                    m_wakeSetStateFuture = m_wakeSetStatePromise.get_future();
                    ASSERT_TRUE(future_status::ready == m_wakeReleaseChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_speechSynthesizer->onFocusChanged(FocusState::BACKGROUND, MixingBehavior::MUST_PAUSE);
                }
                TEST_F(SpeechSynthesizerTest, test_givenPlayingStateFocusBecomesNone) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, MESSAGE_ID_TEST,
                                                                          DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, PAYLOAD_TEST,
                                                                              m_attachmentManager, CONTEXT_ID_TEST);
                    /*EXPECT_CALL(*(m_mockFocusManager.get()), acquireChannel(CHANNEL_NAME, _)).Times(1)
                        .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnAcquireChannel));
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()),attachmentSetSource(A<shared_ptr<AttachmentReader>>(), nullptr))
                        .Times(AtLeast(1));*/
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()), play(_)).Times(AtLeast(1));
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()), getOffset(_)).Times(AtLeast(2))
                        .WillRepeatedly(Return(OFFSET_IN_CHRONO_MILLISECONDS_TEST));
                    EXPECT_CALL(*(m_mockDirHandlerResult.get()), setFailed(_)).Times(1)
                        .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnSetFailed));
                    EXPECT_CALL(*(m_mockDirHandlerResult.get()), setCompleted()).Times(0);
                    EXPECT_CALL(*m_mockPowerResourceManager, acquirePowerResource(COMPONENT_NAME, PowerResourceLevel::ACTIVE_HIGH))
                        .Times(AtLeast(1));
                    m_speechSynthesizer->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirHandlerResult));
                    m_speechSynthesizer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                    EXPECT_TRUE(future_status::ready == m_wakeAcquireChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_speechSynthesizer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                    EXPECT_TRUE(m_mockSpeechPlayer->waitUntilPlaybackStarted());
                    EXPECT_CALL(*m_mockPowerResourceManager, releasePowerResource(COMPONENT_NAME)).Times(AtLeast(1));
                    m_speechSynthesizer->onFocusChanged(FocusState::NONE, MixingBehavior::MUST_STOP);
                    EXPECT_TRUE(future_status::ready == m_wakeSetFailedFuture.wait_for(STATE_CHANGE_TIMEOUT));
                }
                TEST_F(SpeechSynthesizerTest, testTimer_onPlayedStopped) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, PAYLOAD_TEST,
                                                                              m_attachmentManager, CONTEXT_ID_TEST);
                    /*EXPECT_CALL(*(m_mockFocusManager.get()), acquireChannel(CHANNEL_NAME, _)).Times(1)
                        .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnAcquireChannel));
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()),attachmentSetSource(A<shared_ptr<AttachmentReader>>(), nullptr))
                        .Times(AtLeast(1));*/
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()), play(_)).Times(AtLeast(1));
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()), getOffset(_)).WillRepeatedly(Return(OFFSET_IN_CHRONO_MILLISECONDS_TEST));
                    EXPECT_CALL(*(m_mockDirHandlerResult.get()), setFailed(_)).Times(1)
                        .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnSetFailed));
                    EXPECT_CALL(*(m_mockDirHandlerResult.get()), setCompleted()).Times(0);
                    EXPECT_CALL(*m_mockPowerResourceManager, acquirePowerResource(COMPONENT_NAME, PowerResourceLevel::ACTIVE_HIGH))
                        .Times(AtLeast(1));
                    m_speechSynthesizer->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirHandlerResult));
                    m_speechSynthesizer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                    EXPECT_TRUE(std::future_status::ready == m_wakeAcquireChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_speechSynthesizer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                    EXPECT_TRUE(m_mockSpeechPlayer->waitUntilPlaybackStarted());
                    EXPECT_CALL(*m_mockPowerResourceManager, releasePowerResource(COMPONENT_NAME)).Times(AtLeast(1));
                    m_speechSynthesizer->onPlaybackStopped(m_mockSpeechPlayer->getCurrentSourceId(), DEFAULT_MEDIA_PLAYER_STATE);
                    EXPECT_TRUE(std::future_status::ready == m_wakeSetFailedFuture.wait_for(STATE_CHANGE_TIMEOUT));
                }
                bool SST::setupActiveSpeech(unique_ptr<DirectiveHandlerResultInterface> resultHandler, const SpeakTestInfo& info) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, info.messageId,
                                                                          DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, info.payload,
                                                                              m_attachmentManager, CONTEXT_ID_TEST);
                    /*EXPECT_CALL(*m_mockFocusManager, acquireChannel(CHANNEL_NAME, _))
                        .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnAcquireChannel));
                    EXPECT_CALL(*m_mockSpeechPlayer,attachmentSetSource(A<shared_ptr<AttachmentReader>>(), nullptr));*/
                    EXPECT_CALL(*m_mockSpeechPlayer, play(_)).Times(AtLeast(1));
                    EXPECT_CALL(*m_mockSpeechPlayer, getOffset(_)).WillRepeatedly(Return(OFFSET_IN_CHRONO_MILLISECONDS_TEST));
                    EXPECT_CALL(*m_mockContextManager,setState(NAMESPACE_AND_NAME_SPEECH_STATE, generatePlayingState(info),
                                StateRefreshPolicy::ALWAYS, 0)).WillOnce(InvokeWithoutArgs(this, &SST::wakeOnSetState));
                    EXPECT_CALL(*m_mockMessageSender, sendMessage(IsStartedEvent()))
                        .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnSendMessage));
                    EXPECT_CALL(*m_mockPowerResourceManager, acquirePowerResource(COMPONENT_NAME, PowerResourceLevel::ACTIVE_HIGH))
                        .Times(AtLeast(1));
                    EXPECT_CALL(*m_mockPowerResourceManager, releasePowerResource(COMPONENT_NAME)).Times(AtLeast(1));
                    m_speechSynthesizer->CapabilityAgent::preHandleDirective(directive, move(resultHandler));
                    m_speechSynthesizer->CapabilityAgent::handleDirective(info.messageId);
                    EXPECT_TRUE(future_status::ready == m_wakeAcquireChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_speechSynthesizer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                    EXPECT_TRUE(m_mockSpeechPlayer->waitUntilPlaybackStarted());
                    EXPECT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                    EXPECT_TRUE(future_status::ready == m_wakeSendMessageFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_wakeSendMessagePromise = promise<void>();
                    m_wakeSendMessageFuture = m_wakeSendMessagePromise.get_future();
                    m_wakeSetStatePromise = promise<void>();
                    m_wakeSetStateFuture = m_wakeSetStatePromise.get_future();
                    m_wakeAcquireChannelPromise = promise<void>();
                    m_wakeAcquireChannelFuture = m_wakeAcquireChannelPromise.get_future();
                    return !Test::HasFailure();
                }
                bool SST::setupPendingSpeech(unique_ptr<DirectiveHandlerResultInterface> resultHandler, const SpeakTestInfo& info) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, info.messageId,
                                                                          DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, info.payload,
                                                                              m_attachmentManager, CONTEXT_ID_TEST);
                    //EXPECT_CALL(*m_mockFocusManager, acquireChannel(CHANNEL_NAME, _))
                    //    .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnAcquireChannel));
                    EXPECT_CALL(*m_mockPowerResourceManager, acquirePowerResource(COMPONENT_NAME, PowerResourceLevel::ACTIVE_HIGH))
                        .Times(AtLeast(1));
                    EXPECT_CALL(*m_mockPowerResourceManager, releasePowerResource(COMPONENT_NAME)).Times(AtLeast(1));
                    m_speechSynthesizer->CapabilityAgent::preHandleDirective(directive, move(resultHandler));
                    m_speechSynthesizer->CapabilityAgent::handleDirective(info.messageId);
                    EXPECT_TRUE(future_status::ready == m_wakeAcquireChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_wakeAcquireChannelPromise = promise<void>();
                    m_wakeAcquireChannelFuture = m_wakeAcquireChannelPromise.get_future();
                    return !Test::HasFailure();
                }
                TEST_F(SpeechSynthesizerTest, test_replaceAllWithEmptyQueue) {
                    auto mockActiveResultHandler = std::unique_ptr<MockDirectiveHandlerResult>(new MockDirectiveHandlerResult());
                    auto info = generateSpeakInfo(PlayBehavior::REPLACE_ALL);
                    EXPECT_CALL(*mockActiveResultHandler, setCompleted())
                        .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnSetCompleted));
                    EXPECT_TRUE(setupActiveSpeech(std::move(mockActiveResultHandler), info));
                    EXPECT_CALL(*m_mockContextManager,setState(NAMESPACE_AND_NAME_SPEECH_STATE, generateFinishedState(info),
                                StateRefreshPolicy::NEVER, 0)).WillOnce(InvokeWithoutArgs(this, &SST::wakeOnSetState));
                    EXPECT_CALL(*m_mockMessageSender, sendMessage(IsFinishedEvent()))
                        .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnSendMessage));
                    m_mockSpeechPlayer->mockFinished(m_mockSpeechPlayer->getCurrentSourceId());
                    EXPECT_TRUE(future_status::ready == m_wakeSendMessageFuture.wait_for(MY_WAIT_TIMEOUT));
                    EXPECT_TRUE(future_status::ready == m_wakeSetCompletedFuture.wait_for(MY_WAIT_TIMEOUT));
                    EXPECT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                }
                TEST_F(SpeechSynthesizerTest, test_replaceAllWithNonEmptyQueue) {
                    {
                        SCOPED_TRACE("Setup Queue");
                        auto mockEnqueuedResultHandler = unique_ptr<MockDirectiveHandlerResult>(new MockDirectiveHandlerResult());
                        auto pending = generateSpeakInfo(PlayBehavior::ENQUEUE);
                        ASSERT_TRUE(setupPendingSpeech(move(mockEnqueuedResultHandler), pending));
                    }
                    auto mockResultHandler = unique_ptr<MockDirectiveHandlerResult>(new MockDirectiveHandlerResult());
                    EXPECT_CALL(*mockResultHandler, setCompleted()).WillOnce(InvokeWithoutArgs(this, &SST::wakeOnSetCompleted));
                    auto speak = generateSpeakInfo(PlayBehavior::REPLACE_ALL);
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, speak.messageId,
                                                                          DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, speak.payload,
                                                                              m_attachmentManager, CONTEXT_ID_TEST);
                    {
                        SCOPED_TRACE("Setup Expectations");
                        /*EXPECT_CALL(*m_mockFocusManager, acquireChannel(CHANNEL_NAME, _))
                            .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnAcquireChannel));
                        EXPECT_CALL(*m_mockSpeechPlayer,attachmentSetSource(A<shared_ptr<AttachmentReader>>(), nullptr));*/
                        EXPECT_CALL(*m_mockSpeechPlayer, play(_)).Times(AtLeast(1));
                        EXPECT_CALL(*m_mockSpeechPlayer, getOffset(_)).WillRepeatedly(Return(OFFSET_IN_CHRONO_MILLISECONDS_TEST));
                        EXPECT_CALL(*m_mockContextManager,setState(NAMESPACE_AND_NAME_SPEECH_STATE, generatePlayingState(speak),
                                    StateRefreshPolicy::ALWAYS, 0)).WillOnce(InvokeWithoutArgs(this, &SST::wakeOnSetState));
                        EXPECT_CALL(*m_mockMessageSender, sendMessage(IsStartedEvent()))
                            .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnSendMessage));
                    }
                    {
                        SCOPED_TRACE("Test Directive Handling");
                        m_speechSynthesizer->CapabilityAgent::preHandleDirective(directive, std::move(mockResultHandler));
                        m_speechSynthesizer->CapabilityAgent::handleDirective(speak.messageId);
                        EXPECT_TRUE(future_status::ready == m_wakeAcquireChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                    }
                    {
                        SCOPED_TRACE("Check Speech Playback");
                        m_speechSynthesizer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                        EXPECT_TRUE(m_mockSpeechPlayer->waitUntilPlaybackStarted());
                        EXPECT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                        EXPECT_TRUE(future_status::ready == m_wakeSendMessageFuture.wait_for(MY_WAIT_TIMEOUT));
                        m_wakeSendMessagePromise = promise<void>();
                        m_wakeSendMessageFuture = m_wakeSendMessagePromise.get_future();
                        m_wakeSetStatePromise = promise<void>();
                        m_wakeSetStateFuture = m_wakeSetStatePromise.get_future();
                    }
                    {
                        SCOPED_TRACE("Check Speech Playback");
                        EXPECT_CALL(*m_mockContextManager,setState(NAMESPACE_AND_NAME_SPEECH_STATE, generateFinishedState(speak),
                                    StateRefreshPolicy::NEVER, 0)).WillOnce(InvokeWithoutArgs(this, &SST::wakeOnSetState));
                        EXPECT_CALL(*m_mockMessageSender, sendMessage(IsFinishedEvent()))
                            .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnSendMessage));
                        m_mockSpeechPlayer->mockFinished(m_mockSpeechPlayer->getCurrentSourceId());
                        EXPECT_TRUE(std::future_status::ready == m_wakeSendMessageFuture.wait_for(MY_WAIT_TIMEOUT));
                        EXPECT_TRUE(std::future_status::ready == m_wakeSetCompletedFuture.wait_for(MY_WAIT_TIMEOUT));
                        EXPECT_TRUE(std::future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                    }
                }
                TEST_F(SpeechSynthesizerTest, test_replaceAllStopActiveSpeech) {
                    auto active = generateSpeakInfo(PlayBehavior::ENQUEUE);
                    {
                        SCOPED_TRACE("Setup Queue");
                        auto mockEnqueuedResultHandler = unique_ptr<MockDirectiveHandlerResult>(new MockDirectiveHandlerResult());
                        EXPECT_CALL(*mockEnqueuedResultHandler, setFailed(_));
                        ASSERT_TRUE(setupActiveSpeech(move(mockEnqueuedResultHandler), active));
                    }

                    auto mockResultHandler = unique_ptr<MockDirectiveHandlerResult>(new MockDirectiveHandlerResult());
                    EXPECT_CALL(*mockResultHandler, setCompleted())
                        .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnSetCompleted));
                    auto speak = generateSpeakInfo(PlayBehavior::REPLACE_ALL);
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, speak.messageId,
                                                                          DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, speak.payload,
                                                                              m_attachmentManager, CONTEXT_ID_TEST);
                    {
                        SCOPED_TRACE("Setup Expectations");
                        EXPECT_CALL(*m_mockSpeechPlayer, stop(_));
                        EXPECT_CALL(*m_mockContextManager,setState(NAMESPACE_AND_NAME_SPEECH_STATE, generateInterruptedState(active),
                                    StateRefreshPolicy::NEVER, 0));
                        EXPECT_CALL(*m_mockMessageSender, sendMessage(IsInterruptedEvent()));
                        /*EXPECT_CALL(*m_mockFocusManager, acquireChannel(CHANNEL_NAME, _))
                            .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnAcquireChannel));
                        EXPECT_CALL(*m_mockSpeechPlayer,attachmentSetSource(A<shared_ptr<ttachmentReader>>(), nullptr));*/
                        EXPECT_CALL(*m_mockSpeechPlayer, play(_)).Times(AtLeast(1));
                        EXPECT_CALL(*m_mockSpeechPlayer, getOffset(_)).Times(AtLeast(2))
                            .WillRepeatedly(Return(OFFSET_IN_CHRONO_MILLISECONDS_TEST));
                        EXPECT_CALL(*m_mockContextManager,setState(NAMESPACE_AND_NAME_SPEECH_STATE, generatePlayingState(speak),
                                    StateRefreshPolicy::ALWAYS, 0)).WillOnce(InvokeWithoutArgs(this, &SST::wakeOnSetState));
                        EXPECT_CALL(*m_mockMessageSender, sendMessage(IsStartedEvent()))
                            .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnSendMessage));
                        EXPECT_CALL(*m_mockPowerResourceManager, acquirePowerResource(COMPONENT_NAME, PowerResourceLevel::ACTIVE_HIGH))
                            .Times(AtLeast(1));
                    }
                    {
                        SCOPED_TRACE("Test Directive Handling");
                        m_speechSynthesizer->CapabilityAgent::preHandleDirective(directive, std::move(mockResultHandler));
                        m_speechSynthesizer->CapabilityAgent::handleDirective(speak.messageId);
                        EXPECT_TRUE(std::future_status::ready == m_wakeAcquireChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                    }
                    {
                        SCOPED_TRACE("Check Speech Playback");
                        m_speechSynthesizer->onFocusChanged(FocusState::NONE, MixingBehavior::MUST_STOP);
                        m_speechSynthesizer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                        EXPECT_TRUE(m_mockSpeechPlayer->waitUntilPlaybackStarted());
                        EXPECT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                        EXPECT_TRUE(future_status::ready == m_wakeSendMessageFuture.wait_for(MY_WAIT_TIMEOUT));
                        m_wakeSendMessagePromise = promise<void>();
                        m_wakeSendMessageFuture = m_wakeSendMessagePromise.get_future();
                        m_wakeSetStatePromise = promise<void>();
                        m_wakeSetStateFuture = m_wakeSetStatePromise.get_future();
                    }
                    {
                        SCOPED_TRACE("Check Speech Playback");
                        EXPECT_CALL(*m_mockContextManager,setState(NAMESPACE_AND_NAME_SPEECH_STATE, generateFinishedState(speak),
                                    StateRefreshPolicy::NEVER, 0)).WillOnce(InvokeWithoutArgs(this, &SST::wakeOnSetState));
                        EXPECT_CALL(*m_mockMessageSender, sendMessage(IsFinishedEvent()))
                            .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnSendMessage));
                        EXPECT_CALL(*m_mockPowerResourceManager, releasePowerResource(COMPONENT_NAME)).Times(AtLeast(1));
                        m_mockSpeechPlayer->mockFinished(m_mockSpeechPlayer->getCurrentSourceId());
                        EXPECT_TRUE(future_status::ready == m_wakeSendMessageFuture.wait_for(MY_WAIT_TIMEOUT));
                        EXPECT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                    }
                }
                TEST_F(SpeechSynthesizerTest, test_enqueueWithActiveSpeech) {
                    auto firstDirective = generateSpeakInfo(PlayBehavior::ENQUEUE);
                    {
                        SCOPED_TRACE("Setup First");
                        auto mockEnqueuedResultHandler = unique_ptr<MockDirectiveHandlerResult>(new MockDirectiveHandlerResult());
                        EXPECT_CALL(*mockEnqueuedResultHandler, setCompleted());
                        ASSERT_TRUE(setupActiveSpeech(move(mockEnqueuedResultHandler), firstDirective));
                    }
                    auto mockResultHandler = unique_ptr<MockDirectiveHandlerResult>(new MockDirectiveHandlerResult());
                    EXPECT_CALL(*mockResultHandler, setCompleted());
                    auto secondDirective = generateSpeakInfo(PlayBehavior::ENQUEUE);
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, secondDirective.messageId,
                                                                          DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, secondDirective.payload,
                                                                              m_attachmentManager, CONTEXT_ID_TEST);
                    {
                        SCOPED_TRACE("Add Second");
                        m_speechSynthesizer->CapabilityAgent::preHandleDirective(directive, move(mockResultHandler));
                    }
                    {
                        SCOPED_TRACE("Finish First");
                        EXPECT_CALL(*m_mockContextManager,setState(NAMESPACE_AND_NAME_SPEECH_STATE, generateFinishedState(firstDirective),
                                    StateRefreshPolicy::NEVER, 0));
                        EXPECT_CALL(*m_mockMessageSender, sendMessage(IsFinishedEvent()))
                            .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnSendMessage));
                        m_mockSpeechPlayer->mockFinished(m_mockSpeechPlayer->getCurrentSourceId());
                        EXPECT_TRUE(future_status::ready == m_wakeSendMessageFuture.wait_for(MY_WAIT_TIMEOUT));
                    }
                    m_wakeSendMessagePromise = std::promise<void>();
                    m_wakeSendMessageFuture = m_wakeSendMessagePromise.get_future();
                    {
                        SCOPED_TRACE("Start Second");
                        //EXPECT_CALL(*m_mockFocusManager, acquireChannel(CHANNEL_NAME, _))
                        //    .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnAcquireChannel));
                        m_speechSynthesizer->CapabilityAgent::handleDirective(secondDirective.messageId);
                        EXPECT_TRUE(std::future_status::ready == m_wakeAcquireChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                        //EXPECT_CALL(*m_mockSpeechPlayer,attachmentSetSource(A<shared_ptr<AttachmentReader>>(), nullptr));
                        EXPECT_CALL(*m_mockSpeechPlayer, play(_));
                        EXPECT_CALL(*m_mockSpeechPlayer, getOffset(_)).WillRepeatedly(Return(OFFSET_IN_CHRONO_MILLISECONDS_TEST));
                        EXPECT_CALL(*m_mockContextManager,setState(NAMESPACE_AND_NAME_SPEECH_STATE, generatePlayingState(secondDirective),
                                    StateRefreshPolicy::ALWAYS, 0)).WillOnce(InvokeWithoutArgs(this, &SST::wakeOnSetState));
                        EXPECT_CALL(*m_mockMessageSender, sendMessage(IsStartedEvent()))
                            .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnSendMessage));
                        EXPECT_CALL(*m_mockPowerResourceManager, acquirePowerResource(COMPONENT_NAME, PowerResourceLevel::ACTIVE_HIGH))
                            .Times(AtLeast(1));

                        m_speechSynthesizer->onFocusChanged(FocusState::NONE, MixingBehavior::MUST_STOP);
                        m_speechSynthesizer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                        EXPECT_TRUE(future_status::ready == m_wakeSendMessageFuture.wait_for(MY_WAIT_TIMEOUT));
                        EXPECT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                    }
                    m_wakeAcquireChannelPromise = promise<void>();
                    m_wakeAcquireChannelFuture = m_wakeAcquireChannelPromise.get_future();
                    m_wakeSendMessagePromise = promise<void>();
                    m_wakeSendMessageFuture = m_wakeSendMessagePromise.get_future();
                    m_wakeSetStatePromise = promise<void>();
                    m_wakeSetStateFuture = m_wakeSetStatePromise.get_future();
                    {
                        SCOPED_TRACE("Finish Second");
                        EXPECT_CALL(*m_mockContextManager,setState(NAMESPACE_AND_NAME_SPEECH_STATE, generateFinishedState(secondDirective),
                                    StateRefreshPolicy::NEVER, 0)).WillOnce(InvokeWithoutArgs(this, &SST::wakeOnSetState));
                        EXPECT_CALL(*m_mockMessageSender, sendMessage(IsFinishedEvent()))
                            .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnSendMessage));
                        EXPECT_CALL(*m_mockPowerResourceManager, releasePowerResource(COMPONENT_NAME)).Times(AtLeast(1));
                        m_mockSpeechPlayer->mockFinished(m_mockSpeechPlayer->getCurrentSourceId());
                        EXPECT_TRUE(future_status::ready == m_wakeSendMessageFuture.wait_for(MY_WAIT_TIMEOUT));
                        EXPECT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                    }
                }
                TEST_F(SpeechSynthesizerTest, test_replaceEnqueuedWithAnotherEnqueuedItem) {
                    auto firstDirective = generateSpeakInfo(PlayBehavior::ENQUEUE);
                    {
                        SCOPED_TRACE("Setup First");
                        auto mockEnqueuedResultHandler = std::unique_ptr<MockDirectiveHandlerResult>(new MockDirectiveHandlerResult());
                        EXPECT_CALL(*mockEnqueuedResultHandler, setCompleted());
                        ASSERT_TRUE(setupActiveSpeech(std::move(mockEnqueuedResultHandler), firstDirective));
                    }
                    {
                        SCOPED_TRACE("Add Second");
                        auto secondDirective = generateSpeakInfo(PlayBehavior::ENQUEUE);
                        auto secondMessageHeader = make_shared<AVSMessageHeader>(
                            NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, secondDirective.messageId, DIALOG_REQUEST_ID_TEST);
                        std::shared_ptr<AVSDirective> directive = AVSDirective::create(
                            "", secondMessageHeader, secondDirective.payload, m_attachmentManager, CONTEXT_ID_TEST);
                        m_speechSynthesizer->CapabilityAgent::preHandleDirective(directive, nullptr);
                    }
                    auto mockResultHandler = unique_ptr<MockDirectiveHandlerResult>(new MockDirectiveHandlerResult());
                    EXPECT_CALL(*mockResultHandler, setCompleted());
                    auto thirdDirective = generateSpeakInfo(PlayBehavior::REPLACE_ENQUEUED);
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, thirdDirective.messageId,
                                                                          DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, thirdDirective.payload,
                                                                              m_attachmentManager, CONTEXT_ID_TEST);
                    {
                        SCOPED_TRACE("Add Third");
                        m_speechSynthesizer->CapabilityAgent::preHandleDirective(directive, std::move(mockResultHandler));
                        m_speechSynthesizer->CapabilityAgent::handleDirective(thirdDirective.messageId);
                    }
                    {
                        SCOPED_TRACE("Finish First");
                        EXPECT_CALL(*m_mockContextManager,setState(NAMESPACE_AND_NAME_SPEECH_STATE, generateFinishedState(firstDirective),
                                    StateRefreshPolicy::NEVER, 0));
                        EXPECT_CALL(*m_mockMessageSender, sendMessage(IsFinishedEvent()));
                        //EXPECT_CALL(*m_mockFocusManager, acquireChannel(CHANNEL_NAME, _))
                        //    .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnAcquireChannel));
                        m_mockSpeechPlayer->mockFinished(m_mockSpeechPlayer->getCurrentSourceId());
                        EXPECT_TRUE(future_status::ready == m_wakeAcquireChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                        m_wakeAcquireChannelPromise = promise<void>();
                        m_wakeAcquireChannelFuture = m_wakeAcquireChannelPromise.get_future();
                    }
                    {
                        SCOPED_TRACE("Start Second");
                        //EXPECT_CALL(*m_mockSpeechPlayer,attachmentSetSource(A<shared_ptr<AttachmentReader>>(), nullptr));
                        EXPECT_CALL(*m_mockSpeechPlayer, play(_));
                        EXPECT_CALL(*m_mockSpeechPlayer, getOffset(_)).WillRepeatedly(Return(OFFSET_IN_CHRONO_MILLISECONDS_TEST));
                        EXPECT_CALL(*m_mockContextManager,setState(NAMESPACE_AND_NAME_SPEECH_STATE, generatePlayingState(thirdDirective),
                                    StateRefreshPolicy::ALWAYS, 0)).WillOnce(InvokeWithoutArgs(this, &SST::wakeOnSetState));
                        EXPECT_CALL(*m_mockMessageSender, sendMessage(IsStartedEvent()))
                            .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnSendMessage));
                        EXPECT_CALL(*m_mockPowerResourceManager, acquirePowerResource(COMPONENT_NAME, PowerResourceLevel::ACTIVE_HIGH))
                            .Times(AtLeast(1));
                        m_speechSynthesizer->onFocusChanged(FocusState::NONE, MixingBehavior::MUST_STOP);
                        m_speechSynthesizer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                        EXPECT_TRUE(future_status::ready == m_wakeSendMessageFuture.wait_for(MY_WAIT_TIMEOUT));
                        EXPECT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                    }
                    m_wakeSendMessagePromise = promise<void>();
                    m_wakeSendMessageFuture = m_wakeSendMessagePromise.get_future();
                    m_wakeSetStatePromise = promise<void>();
                    m_wakeSetStateFuture = m_wakeSetStatePromise.get_future();
                    {
                        SCOPED_TRACE("Finish Second");
                        EXPECT_CALL(*m_mockContextManager,setState(NAMESPACE_AND_NAME_SPEECH_STATE, generateFinishedState(thirdDirective),
                                    StateRefreshPolicy::NEVER, 0)).WillOnce(InvokeWithoutArgs(this, &SST::wakeOnSetState));
                        EXPECT_CALL(*m_mockMessageSender, sendMessage(IsFinishedEvent()))
                            .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnSendMessage));
                        EXPECT_CALL(*m_mockPowerResourceManager, releasePowerResource(COMPONENT_NAME)).Times(AtLeast(1));
                        m_mockSpeechPlayer->mockFinished(m_mockSpeechPlayer->getCurrentSourceId());
                        EXPECT_TRUE(future_status::ready == m_wakeSendMessageFuture.wait_for(MY_WAIT_TIMEOUT));
                        EXPECT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                    }
                }
                TEST_F(SpeechSynthesizerTest, test_parsingSingleAnalyzerConfig) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, MESSAGE_ID_TEST,
                                                                          DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, PAYLOAD_TEST_SINGLE_ANALYZER,
                                                                              m_attachmentManager, CONTEXT_ID_TEST);
                    /*EXPECT_CALL(*(m_mockFocusManager.get()), acquireChannel(CHANNEL_NAME, _)).Times(1)
                        .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnAcquireChannel));
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()),attachmentSetSource(A<shared_ptr<AttachmentReader>>(), nullptr))
                        .Times(AtLeast(1));*/
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()), play(_)).Times(AtLeast(1));
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()), getOffset(_))
                        .Times(1)
                        .WillOnce(Return(OFFSET_IN_CHRONO_MILLISECONDS_TEST));
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()), getMediaPlayerState(_)).Times(AtLeast(2));
                    EXPECT_CALL(*(m_mockContextManager.get()),setState(NAMESPACE_AND_NAME_SPEECH_STATE, PLAYING_STATE_TEST,
                                StateRefreshPolicy::ALWAYS, 0)).Times(AtLeast(1)).WillOnce(InvokeWithoutArgs(this, &SST::wakeOnSetState));
                    EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_)).Times(AtLeast(1))
                        .WillRepeatedly(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnSendMessage));
                    //EXPECT_CALL(*m_mockCaptionManager, onCaption(_, _)).Times(1);
                    EXPECT_CALL(*m_mockPowerResourceManager, acquirePowerResource(COMPONENT_NAME, PowerResourceLevel::ACTIVE_HIGH))
                        .Times(AtLeast(1));
                    vector<audioAnalyzer::AudioAnalyzerState> data;
                    data.push_back(audioAnalyzer::AudioAnalyzerState("analyzername", "YES"));
                    EXPECT_CALL(*m_mockSpeechSynthesizerObserver,onStateChanged(SpeechSynthesizerState::GAINING_FOCUS, _, _, _)).Times(1);
                    EXPECT_CALL(*m_mockSpeechSynthesizerObserver,onStateChanged(SpeechSynthesizerState::PLAYING, _, _, Eq(data))).Times(1);
                    m_speechSynthesizer->addObserver(m_mockSpeechSynthesizerObserver);
                    m_speechSynthesizer->handleDirectiveImmediately(directive);
                    ASSERT_TRUE(future_status::ready == m_wakeAcquireChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_speechSynthesizer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                    ASSERT_TRUE(m_mockSpeechPlayer->waitUntilPlaybackStarted());
                    ASSERT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                    ASSERT_TRUE(future_status::ready == m_wakeSendMessageFuture.wait_for(MY_WAIT_TIMEOUT));
                }
                TEST_F(SpeechSynthesizerTest, test_parsingMultipleAnalyzerConfig) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_SPEECH_SYNTHESIZER, NAME_SPEAK, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, PAYLOAD_TEST_MULTIPLE_ANALYZER,
                                                                              m_attachmentManager, CONTEXT_ID_TEST);
                    /*EXPECT_CALL(*(m_mockFocusManager.get()), acquireChannel(CHANNEL_NAME, _)).Times(1)
                        .WillOnce(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnAcquireChannel));
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()),attachmentSetSource(A<shared_ptr<AttachmentReader>>(), nullptr))
                        .Times(AtLeast(1));*/
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()), play(_)).Times(AtLeast(1));
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()), getOffset(_)).Times(1)
                        .WillOnce(Return(OFFSET_IN_CHRONO_MILLISECONDS_TEST));
                    EXPECT_CALL(*(m_mockSpeechPlayer.get()), getMediaPlayerState(_)).Times(AtLeast(2));
                    EXPECT_CALL(*(m_mockContextManager.get()),setState(NAMESPACE_AND_NAME_SPEECH_STATE, PLAYING_STATE_TEST,
                                StateRefreshPolicy::ALWAYS, 0)).Times(AtLeast(1)).WillOnce(InvokeWithoutArgs(this, &SST::wakeOnSetState));
                    EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_)).Times(AtLeast(1))
                        .WillRepeatedly(InvokeWithoutArgs(this, &SpeechSynthesizerTest::wakeOnSendMessage));
                    //EXPECT_CALL(*m_mockCaptionManager, onCaption(_, _)).Times(1);
                    EXPECT_CALL(*m_mockPowerResourceManager, acquirePowerResource(COMPONENT_NAME, PowerResourceLevel::ACTIVE_HIGH))
                        .Times(AtLeast(1));
                    vector<audioAnalyzer::AudioAnalyzerState> data;
                    data.push_back(AudioAnalyzerState("analyzername1", "YES"));
                    data.push_back(AudioAnalyzerState("analyzername2", "NO"));
                    EXPECT_CALL(*m_mockSpeechSynthesizerObserver,onStateChanged(SpeechSynthesizerState::GAINING_FOCUS, _, _, _)).Times(1);
                    EXPECT_CALL(*m_mockSpeechSynthesizerObserver,onStateChanged(SpeechSynthesizerState::PLAYING, _, _, Eq(data))).Times(1);
                    m_speechSynthesizer->addObserver(m_mockSpeechSynthesizerObserver);
                    m_speechSynthesizer->handleDirectiveImmediately(directive);
                    ASSERT_TRUE(future_status::ready == m_wakeAcquireChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_speechSynthesizer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                    ASSERT_TRUE(m_mockSpeechPlayer->waitUntilPlaybackStarted());
                    ASSERT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                    ASSERT_TRUE(future_status::ready == m_wakeSendMessageFuture.wait_for(MY_WAIT_TIMEOUT));
                }
            }
        }
    }
}