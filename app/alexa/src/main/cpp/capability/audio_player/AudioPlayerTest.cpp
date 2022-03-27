#include <chrono>
#include <future>
#include <map>
#include <memory>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <json/document.h>
#include <json/en.h>
#include <json/stringbuffer.h>
#include <avs/attachment/AttachmentManager.h>
#include <sdkinterfaces/MockChannelVolumeInterface.h>
#include <sdkinterfaces/MockContextManager.h>
#include <sdkinterfaces/MockDirectiveHandlerResult.h>
#include <sdkinterfaces/MockExceptionEncounteredSender.h>
#include <sdkinterfaces/MockFocusManager.h>
#include <sdkinterfaces/MockMessageSender.h>
#include <sdkinterfaces/MockPlaybackRouter.h>
#include <json/JSONGenerator.h>
#include <json/JSONUtils.h>
#include <media_player/MockMediaPlayer.h>
#include <media_player/PooledMediaPlayerFactory.h>
#include <memory/Memory.h>
#include <metrics/MockMetricRecorder.h>
#include <captions/MockCaptionManager.h>
#include "AudioPlayer.h"

namespace alexaClientSDK {
    namespace acsdkAudioPlayer {
        namespace test {
            using namespace acsdkAudioPlayerInterfaces;
            using namespace testing;
            using namespace utils;
            using namespace json;
            using namespace avsCommon;
            using namespace avs;
            using namespace attachment;
            using namespace sdkInterfaces;
            using namespace memory;
            using namespace rapidjson;
            using namespace alexaClientSDK::mediaPlayer;
            using namespace utils::mediaPlayer;
            using namespace sdkInterfaces::test;
            using namespace metrics::test;
            using namespace captions::test;
            using namespace utils::mediaPlayer::test;
            using MediaPlayerState = MediaPlayerState;
            static milliseconds MY_WAIT_TIMEOUT(1000);
            static const MediaPlayerState DEFAULT_MEDIA_PLAYER_STATE = {milliseconds(0)};
            static milliseconds EVENT_PROCESS_DELAY(20);
            static const string CHANNEL_NAME(FocusManagerInterface::CONTENT_CHANNEL_NAME);
            static const string NAMESPACE_AUDIO_PLAYER("AudioPlayer");
            static const string NAMESPACE_AUDIO_PLAYER_2("AudioPlayer_2");
            static const string NAME_PLAY("Play");
            static const string NAME_STOP("Stop");
            static const string NAME_CLEARQUEUE("ClearQueue");
            static const string NAME_UPDATE_PROGRESS_REPORT_INTERVAL("UpdateProgressReportInterval");
            static const NamespaceAndName NAMESPACE_AND_NAME_PLAYBACK_STATE{NAMESPACE_AUDIO_PLAYER, "PlaybackState"};
            static const string MESSAGE_ID_TEST("MessageId_Test");
            static const string MESSAGE_ID_TEST_2("MessageId_Test2");
            static const string MESSAGE_ID_TEST_3("MessageId_Test3");
            static const string PLAY_REQUEST_ID_TEST("PlayRequestId_Test");
            static const string CONTEXT_ID_TEST("ContextId_Test");
            static const string CONTEXT_ID_TEST_2("ContextId_Test2");
            static const string CONTEXT_ID_TEST_3("ContextId_Test3");
            static const string TOKEN_TEST("Token_Test");
            static const string PREV_TOKEN_TEST("Prev_Token_Test");
            static const string FORMAT_TEST("AUDIO_MPEG");
            static const string URL_TEST("cid:Test");
            static const string NAME_ENQUEUE("ENQUEUE");
            static const string NAME_REPLACE_ALL("REPLACE_ALL");
            static const string NAME_CLEAR_ALL("CLEAR_ALL");
            static const string AUDIO_ITEM_ID_1("testID1");
            static const string AUDIO_ITEM_ID_2("testID2");
            static const string FINISHED_STATE("FINISHED");
            static const string PLAYING_STATE{"PLAYING"};
            static const string IDLE_STATE{"IDLE"};
            static const long OFFSET_IN_MILLISECONDS_TEST{100};
            static const string EXPIRY_TEST("481516234248151623421088");
            static const long PROGRESS_REPORT_DELAY{200};
            static const long PROGRESS_REPORT_INTERVAL{100};
            static const long OFFSET_IN_MILLISECONDS_BEFORE_PROGRESS_REPORT_DELAY{PROGRESS_REPORT_DELAY - 1};
            static const long OFFSET_IN_MILLISECONDS_AFTER_PROGRESS_REPORT_DELAY{PROGRESS_REPORT_DELAY + 1};
            static const long OFFSET_IN_MILLISECONDS_BEFORE_PROGRESS_REPORT_INTERVAL{PROGRESS_REPORT_INTERVAL - 1};
            static const long OFFSET_IN_MILLISECONDS_AFTER_PROGRESS_REPORT_INTERVAL{PROGRESS_REPORT_INTERVAL + 1};
            static const milliseconds TIME_FOR_TWO_AND_A_HALF_INTERVAL_PERIODS{milliseconds((2 * PROGRESS_REPORT_INTERVAL) + (PROGRESS_REPORT_INTERVAL / 2))};
            static const long METADATA_EVENT_DELAY{1001};
            static const string CAPTION_CONTENT_SAMPLE = "WEBVTT\\n\\n1\\n00:00.000 --> 00:01.260\\nThe time is 2:17 PM.";
            static const string PLAY_REQUESTOR_TYPE_ALERT{"ALERT"};
            static const string PLAY_REQUESTOR_ID{"12345678"};
            static string createEnqueuePayloadTest(long offsetInMilliseconds, const string& audioId = AUDIO_ITEM_ID_1) {
                const string ENQUEUE_PAYLOAD_TEST = "{\"playBehavior\":\"" + NAME_ENQUEUE + "\",\"audioItem\": {\"audioItemId\":\"" + audioId + "\","
                                                    "\"stream\": {\"url\":\"" + URL_TEST + "\",\"streamFormat\":\"" + FORMAT_TEST + "\",\"offsetInMilliseconds\":" +
                                                    to_string(offsetInMilliseconds) + ",\"expiryTime\":\"" + EXPIRY_TEST + "\",\"progressReport\": {"
                                                    "\"progressReportDelayInMilliseconds\":" + to_string(PROGRESS_REPORT_DELAY) + ","
                                                    "\"progressReportIntervalInMilliseconds\":" + to_string(PROGRESS_REPORT_INTERVAL) + "},\"caption\": {"
                                                    "\"content\":\"" + CAPTION_CONTENT_SAMPLE + "\",\"type\":\"WEBVTT\"},\"token\":\"" + TOKEN_TEST + "\","
                                                    "\"expectedPreviousToken\":\"\"}}}";
                return ENQUEUE_PAYLOAD_TEST;
            }
            static const string REPLACE_ALL_PAYLOAD_TEST = "{\"playBehavior\":\"" + NAME_REPLACE_ALL + "\",\"audioItem\": {\"audioItemId\":\"" +
                                                           AUDIO_ITEM_ID_2 + "\",\"stream\": {\"url\":\"" + URL_TEST + "\",\"streamFormat\":\"" + FORMAT_TEST +
                                                           "\",\"offsetInMilliseconds\":" + to_string(OFFSET_IN_MILLISECONDS_TEST) + ",\"expiryTime\":\"" +
                                                           EXPIRY_TEST + "\",\"progressReport\": {\"progressReportDelayInMilliseconds\":" +
                                                           to_string(PROGRESS_REPORT_DELAY) + ",\"progressReportIntervalInMilliseconds\":" +
                                                           to_string(PROGRESS_REPORT_INTERVAL) + "},\"caption\": {\"content\":\"" + CAPTION_CONTENT_SAMPLE +
                                                           "\",\"type\":\"WEBVTT\"},\"token\":\"" + TOKEN_TEST + "\",\"expectedPreviousToken\":\"\"}}}";
            static string createPayloadWithEndOffset(long offset, long endOffset, const string& audioId = AUDIO_ITEM_ID_1) {
                static const string END_OFFSET_PAYLOAD_TEST = "{\"playBehavior\":\"" + NAME_REPLACE_ALL + "\",\"audioItem\": {\"audioItemId\":\"" + audioId +
                                                              "\",\"stream\": {\"url\":\"" + URL_TEST + "\",\"streamFormat\":\"" + FORMAT_TEST + "\","
                                                              "\"offsetInMilliseconds\":" + to_string(offset) + ",\"endOffsetInMilliseconds\":" +
                                                              to_string(endOffset) + ",\"expiryTime\":\"" + EXPIRY_TEST + "\",\"progressReport\": {"
                                                              "\"progressReportDelayInMilliseconds\":" + to_string(PROGRESS_REPORT_DELAY) + ","
                                                              "\"progressReportIntervalInMilliseconds\":" + to_string(PROGRESS_REPORT_INTERVAL) +
                                                              "},\"caption\": {\"content\":\"" + CAPTION_CONTENT_SAMPLE + "\",\"type\":\"WEBVTT\"},"
                                                              "\"token\":\"" + TOKEN_TEST + "\",\"expectedPreviousToken\":\"\"}}}";
                return END_OFFSET_PAYLOAD_TEST;
            }
            static const string PLAY_REQUESTOR_PAYLOAD_TEST = "{\"playBehavior\":\"" + NAME_REPLACE_ALL + "\",\"playRequestor\": {\"type\":\"" +
                                                              PLAY_REQUESTOR_TYPE_ALERT + "\",\"id\":\"" + PLAY_REQUESTOR_ID + "\"},\"audioItem\": {"
                                                              "\"audioItemId\":\"" + AUDIO_ITEM_ID_2 + "\",\"stream\": {\"url\":\"" + URL_TEST + "\","
                                                              "\"streamFormat\":\"" + FORMAT_TEST + "\",\"offsetInMilliseconds\":" +
                                                              to_string(OFFSET_IN_MILLISECONDS_TEST) + ",\"expiryTime\":\"" + EXPIRY_TEST + "\","
                                                              "\"progressReport\": {\"progressReportDelayInMilliseconds\":" + to_string(PROGRESS_REPORT_DELAY) +
                                                              ",\"progressReportIntervalInMilliseconds\":" + to_string(PROGRESS_REPORT_INTERVAL) + "},"
                                                              "\"token\":\"" + TOKEN_TEST + "\",\"expectedPreviousToken\":\"\"}}}";
            static const string EMPTY_PAYLOAD_TEST = "{}";
            static const string CLEAR_ALL_PAYLOAD_TEST = "{\"clearBehavior\":\"" + NAME_CLEAR_ALL + "\"}";
            static const string TOKEN_KEY = "token";
            static const string OFFSET_KEY = "offsetInMilliseconds";
            static const string SEEK_END_OFFSET_KEY = "seekEndOffsetInMilliseconds";
            static const string ACTIVITY_KEY = "playerActivity";
            static const string IDLE_STATE_TEST = "{\"token\":\"\",\"offsetInMilliseconds\":" + to_string(0) + ",\"playerActivity\":\"" + IDLE_STATE + "\"}";
            static const unsigned int PROVIDE_STATE_TOKEN_TEST{1};
            static const string UPDATE_PROGRESS_REPORT_INTERVAL_PAYLOAD_TEST = "{\"progressReportIntervalInMilliseconds\": 500}";
            static const string MESSAGE_EVENT_KEY = "event";
            static const string MESSAGE_HEADER_KEY = "header";
            static const string MESSAGE_NAME_KEY = "name";
            static const string MESSAGE_TOKEN_KEY = "token";
            static const string MESSAGE_PAYLOAD_KEY = "payload";
            static const string MESSAGE_METADATA_KEY = "metadata";
            static const string MESSAGE_METADATA_STRING_KEY = "StringKey";
            static const string MESSAGE_METADATA_STRING_KEY_WL = "Title";
            static const string MESSAGE_METADATA_STRING_VALUE = "StringValue";
            static const string MESSAGE_METADATA_STRING_VALUE_ALT = "StringValue2";
            static const string MESSAGE_METADATA_UINT_KEY = "UintKey";
            static const string MESSAGE_METADATA_UINT_VALUE = "12345";
            static const string MESSAGE_METADATA_INT_KEY = "IntKey";
            static const string MESSAGE_METADATA_INT_VALUE = "67890";
            static const string MESSAGE_METADATA_DOUBLE_KEY = "DoubleKey";
            static const string MESSAGE_METADATA_DOUBLE_VALUE = "3.14";
            static const string MESSAGE_METADATA_BOOLEAN_KEY = "BooleanKey";
            static const string MESSAGE_METADATA_BOOLEAN_VALUE = "true";
            static const string MESSAGE_PLAYBACK_ATTRIBUTES_KEY = "playbackAttributes";
            static const string MESSAGE_PLAYBACK_ATTRIBUTES_NAME_KEY = "name";
            static const string MESSAGE_PLAYBACK_ATTRIBUTES_NAME_VALUE = "STREAM_NAME_ABSENT";
            static const string MESSAGE_PLAYBACK_ATTRIBUTES_CODEC_KEY = "codec";
            static const string MESSAGE_PLAYBACK_ATTRIBUTES_CODEC_VALUE = "opus";
            static const string MESSAGE_PLAYBACK_ATTRIBUTES_SAMPLING_RATE_KEY = "samplingRateInHertz";
            static const long MESSAGE_PLAYBACK_ATTRIBUTES_SAMPLING_RATE_VALUE = 48000;
            static const string MESSAGE_PLAYBACK_ATTRIBUTES_BITRATE_KEY = "dataRateInBitsPerSecond";
            static const long MESSAGE_PLAYBACK_ATTRIBUTES_BITRATE_VALUE = 49000;
            static const string MESSAGE_PLAYBACK_REPORTS_KEY = "playbackReports";
            static const string MESSAGE_PLAYBACK_REPORTS_START_OFFSET_KEY = "startOffsetInMilliseconds";
            static const long MESSAGE_PLAYBACK_REPORTS_START_OFFSET_VALUE = 0;
            static const string MESSAGE_PLAYBACK_REPORTS_END_OFFSET_KEY = "endOffsetInMilliseconds";
            static const long MESSAGE_PLAYBACK_REPORTS_END_OFFSET_VALUE = 10000;
            static const string PLAYBACK_STARTED_NAME = "PlaybackStarted";
            static const string PLAYBACK_NEARLY_FINISHED_NAME = "PlaybackNearlyFinished";
            static const string PLAYBACK_FINISHED_NAME = "PlaybackFinished";
            static const string PLAYBACK_STOPPED_NAME = "PlaybackStopped";
            static const string PLAYBACK_PAUSED_NAME = "PlaybackPaused";
            static const string PLAYBACK_FAILED_NAME = "PlaybackFailed";
            static const string PLAYBACK_RESUMED_NAME = "PlaybackResumed";
            static const string PLAYBACK_STUTTER_STARTED_NAME = "PlaybackStutterStarted";
            static const string PLAYBACK_STUTTER_FINISHED_NAME = "PlaybackStutterFinished";
            static const string PROGRESS_REPORT_DELAY_ELAPSED_NAME = "ProgressReportDelayElapsed";
            static const string PROGRESS_REPORT_INTERVAL_ELAPSED_NAME = "ProgressReportIntervalElapsed";
            static const string PROGRESS_REPORT_INTERVAL_UPDATED_NAME = "ProgressReportIntervalUpdated";
            static const string STREAM_METADATA_EXTRACTED_NAME = "StreamMetadataExtracted";
            static const string SEEK_COMPLETE_NAME = "PlaybackSeeked";
            static const string TAG("AudioPlayerTest");
            static const Fingerprint FINGERPRINT = {"com.audioplayer.test", "DEBUG", "0001"};
            static const string FINGERPRINT_KEY = "fingerprint";
            static const string FINGERPRINT_PACKAGE_KEY = "package";
            static const string FINGERPRINT_BUILD_TYPE_KEY = "buildType";
            static const string FINGERPRINT_VERSION_NUMBER_KEY = "versionNumber";
            #define LX(event) LogEntry(TAG, event)
            class TestAudioPlayerObserver : public AudioPlayerObserverInterface {
            public:
                TestAudioPlayerObserver() : m_state{PlayerActivity::IDLE} {}
                bool waitFor(PlayerActivity activity, milliseconds timeout) {
                    unique_lock<mutex> lock(m_mutex);
                    return m_conditionVariable.wait_for(lock, timeout, [this, activity] { return m_state == activity; });
                }
                void onPlayerActivityChanged(PlayerActivity state, const Context& context) override {
                    ACSDK_DEBUG(LX("onPlayerActivityChanged").d("state", state).d("audioItemId", context.audioItemId)
                        .d("offsetInMs", duration_cast<milliseconds>(context.offset).count()));
                    {
                        lock_guard<mutex> lock(m_mutex);
                        m_state = state;
                        m_playRequestor = context.playRequestor;
                    }
                    m_conditionVariable.notify_all();
                }
                PlayRequestor getPlayRequestorObject() {
                    lock_guard<mutex> lock(m_mutex);
                    return m_playRequestor;
                }
            private:
                PlayerActivity m_state;
                PlayRequestor m_playRequestor;
                mutex m_mutex;
                condition_variable m_conditionVariable;
            };
            class AudioPlayerTest : public Test {
            public:
                AudioPlayerTest();
                void SetUp() override;
                void TearDown() override;
                void reSetUp(int numberOfPlayers);
                unique_ptr<PooledMediaPlayerFactory> m_mockFactory;
                shared_ptr<AudioPlayer> m_audioPlayer;
                shared_ptr<TestAudioPlayerObserver> m_testAudioPlayerObserver;
                shared_ptr<MockMediaPlayer> m_mockMediaPlayer;
                shared_ptr<MockMediaPlayer> m_mockMediaPlayerTrack2;
                shared_ptr<MockMediaPlayer> m_mockMediaPlayerTrack3;
                shared_ptr<MockChannelVolumeInterface> m_mockSpeaker;
                shared_ptr<MockContextManager> m_mockContextManager;
                shared_ptr<MockFocusManager> m_mockFocusManager;
                unique_ptr<MockDirectiveHandlerResult> m_mockDirectiveHandlerResult;
                shared_ptr<MockMessageSender> m_mockMessageSender;
                shared_ptr<MockExceptionEncounteredSender> m_mockExceptionSender;
                shared_ptr<MockPlaybackRouter> m_mockPlaybackRouter;
                shared_ptr<MockCaptionManager> m_mockCaptionManager;
                shared_ptr<AttachmentManager> m_attachmentManager;
                map<string, int> m_expectedMessages;
                MediaPlayerInterface::SourceId m_sourceId;
                shared_ptr<MockMetricRecorder> m_mockMetricRecorder;
                SetStateResult wakeOnSetState();
                promise<void> m_wakeSetStatePromise;
                future<void> m_wakeSetStateFuture;
                bool wakeOnAcquireChannel();
                promise<void> m_wakeAcquireChannelPromise;
                future<void> m_wakeAcquireChannelFuture;
                future<bool> wakeOnReleaseChannel();
                promise<void> m_wakeReleaseChannelPromise;
                future<void> m_wakeReleaseChannelFuture;
                void wakeOnSendMessage();
                promise<void> m_wakeSendMessagePromise;
                future<void> m_wakeSendMessageFuture;
                void sendPlayDirective(long offsetInMilliseconds = OFFSET_IN_MILLISECONDS_TEST);
                void badEndOffsetDirective(long offset, long endOffset);
                void sendStopDirective();
                void sendClearQueueDirective();
                void sendUpdateProgressReportIntervalDirective();
                bool verifyMessage(shared_ptr<MessageRequest> request, std::string expectedName);
                void verifyMessageMap(shared_ptr<MessageRequest> request, map<string, int>* expectedMessages);
                void verifyMessageOrder2Phase(const vector<string>& orderedMessageList, size_t index, function<void()> trigger1, function<void()> trigger2);
                void verifyState(const string& providedState, const string& expectedState);
                void verifyTags(shared_ptr<MessageRequest> request, map<string, int>* expectedMessages, bool validateBoolean = true);
                void extractPlaybackAttributes(shared_ptr<MessageRequest> request, PlaybackAttributes* actualPlaybackAttributes);
                void extractPlaybackReports(shared_ptr<MessageRequest> request, vector<PlaybackReport>* actualPlaybackReports);
                bool extractMediaPlayerState(shared_ptr<MessageRequest> request, const string& expectedState, MediaPlayerState* playerState);
                void testPlayEnqueueFinishPlay();
                mutex m_mutex;
                condition_variable m_messageSentTrigger;
                condition_variable m_mediaPlayerCallTrigger;
            };
            AudioPlayerTest::AudioPlayerTest() : m_wakeSetStatePromise{}, m_wakeSetStateFuture{m_wakeSetStatePromise.get_future()},
                                                 m_wakeAcquireChannelPromise{}, m_wakeAcquireChannelFuture{m_wakeAcquireChannelPromise.get_future()},
                                                 m_wakeReleaseChannelPromise{}, m_wakeReleaseChannelFuture{m_wakeReleaseChannelPromise.get_future()},
                                                 m_wakeSendMessagePromise{}, m_wakeSendMessageFuture{m_wakeSendMessagePromise.get_future()} {}
            void AudioPlayerTest::SetUp() {
                MockMediaPlayer::enableConcurrentMediaPlayers();
                m_mockContextManager = make_shared<NiceMock<MockContextManager>>();
                m_mockFocusManager = make_shared<NiceMock<MockFocusManager>>();
                m_mockMessageSender = make_shared<NiceMock<MockMessageSender>>();
                m_mockExceptionSender = make_shared<NiceMock<MockExceptionEncounteredSender>>();
                m_attachmentManager = make_shared<AttachmentManager>(AttachmentManager::AttachmentType::IN_PROCESS);
                m_mockSpeaker = make_shared<NiceMock<MockChannelVolumeInterface>>();
                m_mockMediaPlayer = MockMediaPlayer::create();
                m_mockPlaybackRouter = make_shared<NiceMock<MockPlaybackRouter>>();
                ASSERT_TRUE(m_mockMediaPlayer);
                m_mockMediaPlayerTrack2 = MockMediaPlayer::create();
                ASSERT_TRUE(m_mockMediaPlayerTrack2);
                m_mockMediaPlayerTrack3 = MockMediaPlayer::create();
                ASSERT_TRUE(m_mockMediaPlayerTrack3);
                vector<shared_ptr<MediaPlayerInterface>> pool = {m_mockMediaPlayer, m_mockMediaPlayerTrack2, m_mockMediaPlayerTrack3};
                m_mockFactory = PooledMediaPlayerFactory::create(pool, FINGERPRINT);
                //m_mockCaptionManager = make_shared<NiceMock<MockCaptionManager>>();
                m_mockMetricRecorder = make_shared<NiceMock<MockMetricRecorder>>();
                m_audioPlayer = AudioPlayer::create(move(m_mockFactory),m_mockMessageSender,m_mockFocusManager,
                                      m_mockContextManager,m_mockExceptionSender,m_mockPlaybackRouter,
                           {m_mockSpeaker},m_mockCaptionManager,m_mockMetricRecorder);
                ASSERT_TRUE(m_audioPlayer);
                m_testAudioPlayerObserver = make_shared<TestAudioPlayerObserver>();
                m_audioPlayer->addObserver(m_testAudioPlayerObserver);
                m_mockDirectiveHandlerResult = unique_ptr<MockDirectiveHandlerResult>(new MockDirectiveHandlerResult);
            }
            void AudioPlayerTest::TearDown() {
                if (m_audioPlayer) m_audioPlayer->shutdown();
                m_mockMediaPlayer->shutdown();
                m_mockMediaPlayerTrack2->shutdown();
                m_mockMediaPlayerTrack3->shutdown();
            }
            void AudioPlayerTest::reSetUp(int numberOfPlayers) {
                ASSERT_LE(numberOfPlayers, 3);
                ASSERT_GE(numberOfPlayers, 1);
                if (m_audioPlayer) m_audioPlayer->shutdown();
                vector<shared_ptr<MediaPlayerInterface>> pool;
                switch(numberOfPlayers) {
                    case 3: pool.push_back(m_mockMediaPlayerTrack3);
                    case 2: pool.push_back(m_mockMediaPlayerTrack2);
                    case 1: pool.push_back(m_mockMediaPlayer);
                }
                m_mockFactory = PooledMediaPlayerFactory::create(pool);
                m_audioPlayer = AudioPlayer::create(move(m_mockFactory),m_mockMessageSender,m_mockFocusManager,
                                      m_mockContextManager,m_mockExceptionSender,m_mockPlaybackRouter,
                            {m_mockSpeaker},m_mockCaptionManager,m_mockMetricRecorder);
                ASSERT_TRUE(m_audioPlayer);
                m_testAudioPlayerObserver = make_shared<TestAudioPlayerObserver>();
                m_audioPlayer->addObserver(m_testAudioPlayerObserver);
            }
            SetStateResult AudioPlayerTest::wakeOnSetState() {
                m_wakeSetStatePromise.set_value();
                return SetStateResult::SUCCESS;
            }
            bool AudioPlayerTest::wakeOnAcquireChannel() {
                m_wakeAcquireChannelPromise.set_value();
                return true;
            }
            future<bool> AudioPlayerTest::wakeOnReleaseChannel() {
                promise<bool> releaseChannelSuccess;
                future<bool> returnValue = releaseChannelSuccess.get_future();
                releaseChannelSuccess.set_value(true);
                m_wakeReleaseChannelPromise.set_value();
                return returnValue;
            }
            void AudioPlayerTest::wakeOnSendMessage() {
                m_wakeSendMessagePromise.set_value();
            }
            void AudioPlayerTest::sendPlayDirective(long offsetInMilliseconds) {
                auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_AUDIO_PLAYER, NAME_PLAY, MESSAGE_ID_TEST, PLAY_REQUEST_ID_TEST);
                shared_ptr<AVSDirective> playDirective = AVSDirective::create("", avsMessageHeader, createEnqueuePayloadTest(offsetInMilliseconds),
                                                              m_attachmentManager, CONTEXT_ID_TEST);
                //EXPECT_CALL(*(m_mockFocusManager.get()), acquireChannel(CHANNEL_NAME, _)).Times(1).WillOnce(InvokeWithoutArgs(this, &AudioPlayerTest::wakeOnAcquireChannel));
                EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted());
                m_audioPlayer->CapabilityAgent::preHandleDirective(playDirective, std::move(m_mockDirectiveHandlerResult));
                m_mockMediaPlayer->waitUntilNextSetSource(MY_WAIT_TIMEOUT);
                m_audioPlayer->onBufferingComplete(m_mockMediaPlayer->getLatestSourceId(), DEFAULT_MEDIA_PLAYER_STATE);
                m_audioPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                ASSERT_EQ(future_status::ready, m_wakeAcquireChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                m_audioPlayer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PLAYING, MY_WAIT_TIMEOUT));
            }
            void AudioPlayerTest::badEndOffsetDirective(long offset, long endOffset) {
                ASSERT_LE(endOffset, offset);
                auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_AUDIO_PLAYER, NAME_PLAY, MESSAGE_ID_TEST_2);
                string endOffsetPayload = createPayloadWithEndOffset(offset, endOffset);
                shared_ptr<AVSDirective> playDirective = AVSDirective::create("", avsMessageHeader, endOffsetPayload, m_attachmentManager, CONTEXT_ID_TEST_2);
                EXPECT_CALL(*(m_mockExceptionSender.get()), sendExceptionEncountered(_, _, _));
                EXPECT_CALL(*m_mockDirectiveHandlerResult, setFailed(_));
                m_audioPlayer->CapabilityAgent::preHandleDirective(playDirective, std::move(m_mockDirectiveHandlerResult));
            }
            void AudioPlayerTest::sendStopDirective() {
                auto avsStopMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_AUDIO_PLAYER, NAME_STOP, MESSAGE_ID_TEST, PLAY_REQUEST_ID_TEST);
                shared_ptr<AVSDirective> stopDirective = AVSDirective::create("", avsStopMessageHeader, EMPTY_PAYLOAD_TEST, m_attachmentManager, CONTEXT_ID_TEST);
                m_audioPlayer->CapabilityAgent::preHandleDirective(stopDirective, std::move(m_mockDirectiveHandlerResult));
                m_audioPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
            }
            void AudioPlayerTest::sendClearQueueDirective() {
                auto avsClearMessageHeader = std::make_shared<AVSMessageHeader>(NAMESPACE_AUDIO_PLAYER, NAME_CLEARQUEUE, MESSAGE_ID_TEST, PLAY_REQUEST_ID_TEST);
                std::shared_ptr<AVSDirective> clearQueueDirective =
                    AVSDirective::create("", avsClearMessageHeader, CLEAR_ALL_PAYLOAD_TEST, m_attachmentManager, CONTEXT_ID_TEST);
                m_audioPlayer->CapabilityAgent::preHandleDirective(clearQueueDirective, std::move(m_mockDirectiveHandlerResult));
                m_audioPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
            }
            void AudioPlayerTest::sendUpdateProgressReportIntervalDirective() {
                auto header = make_shared<AVSMessageHeader>(NAMESPACE_AUDIO_PLAYER, NAME_UPDATE_PROGRESS_REPORT_INTERVAL, MESSAGE_ID_TEST, PLAY_REQUEST_ID_TEST);
                shared_ptr<AVSDirective> directive = AVSDirective::create("", header, UPDATE_PROGRESS_REPORT_INTERVAL_PAYLOAD_TEST, m_attachmentManager, CONTEXT_ID_TEST);
                m_audioPlayer->CapabilityAgent::preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                m_audioPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
            }
            bool AudioPlayerTest::verifyMessage(std::shared_ptr<avsCommon::avs::MessageRequest> request, std::string expectedName) {
                Document document;
                document.Parse(request->getJsonContent().c_str());
                EXPECT_FALSE(document.HasParseError()) << "rapidjson detected a parsing error at offset:" + to_string(document.GetErrorOffset()) +
                             ", error message: " + GetParseError_En(document.GetParseError());
                auto event = document.FindMember(MESSAGE_EVENT_KEY.data());
                EXPECT_NE(event, document.MemberEnd());
                auto header = event->value.FindMember(MESSAGE_HEADER_KEY.data());
                EXPECT_NE(header, event->value.MemberEnd());
                string requestName;
                rapidjson::Value value{header->value.GetString(), strlen(header->value.GetString())};
                jsonUtils::retrieveValue1(value, MESSAGE_NAME_KEY.data(), &requestName);
                return (requestName == expectedName);
            }
            void AudioPlayerTest::verifyMessageMap(shared_ptr<MessageRequest> request, map<string, int>* expectedMessages) {
                Document document;
                document.Parse(request->getJsonContent().c_str());
                EXPECT_FALSE(document.HasParseError()) << "rapidjson detected a parsing error at offset:" + to_string(document.GetErrorOffset()) +
                             ", error message: " + GetParseError_En(document.GetParseError());
                auto event = document.FindMember(MESSAGE_EVENT_KEY.data());
                EXPECT_NE(event, document.MemberEnd());
                auto header = event->value.FindMember(MESSAGE_HEADER_KEY.data());
                EXPECT_NE(header, event->value.MemberEnd());
                string requestName;
                rapidjson::Value value{header->value.GetString(), strlen(header->value.GetString())};
                jsonUtils::retrieveValue1(value, MESSAGE_NAME_KEY.data(), &requestName);
                if (expectedMessages->find(requestName) != expectedMessages->end()) expectedMessages->at(requestName) = expectedMessages->at(requestName) + 1;
            }
            void AudioPlayerTest::verifyState(const string& providedState, const string& expectedState) {
                Document providedStateParsed;
                providedStateParsed.Parse(providedState.data());
                Document expectedStateParsed;
                expectedStateParsed.Parse(expectedState.data());
                EXPECT_EQ(providedStateParsed, expectedStateParsed);
            }
            void AudioPlayerTest::verifyTags(shared_ptr<MessageRequest> request, map<string, int>* expectedMessages, bool validateBoolean) {
                Document document;
                document.Parse(request->getJsonContent().c_str());
                EXPECT_FALSE(document.HasParseError()) << "rapidjson detected a parsing error at offset:" + to_string(document.GetErrorOffset()) +
                             ", error message: " + GetParseError_En(document.GetParseError());
                auto event = document.FindMember(MESSAGE_EVENT_KEY.data());
                EXPECT_NE(event, document.MemberEnd());
                auto header = event->value.FindMember(MESSAGE_HEADER_KEY.data());
                EXPECT_NE(header, event->value.MemberEnd());
                string requestName;
                rapidjson::Value value{header->value.GetString(), strlen(header->value.GetString())};
                jsonUtils::retrieveValue1(value, MESSAGE_NAME_KEY.data(), &requestName);
                if (expectedMessages->find(requestName) != expectedMessages->end()) {
                    expectedMessages->at(requestName) = expectedMessages->at(requestName) + 1;
                }
                auto payload = event->value.FindMember(MESSAGE_PAYLOAD_KEY.data());
                EXPECT_NE(payload, event->value.MemberEnd());
                auto metadata = payload->value.FindMember(MESSAGE_METADATA_KEY.data());
                if (metadata == payload->value.MemberEnd()) return;
                string metadata_string_value;
                rapidjson::Value value1{metadata->value.GetString(), strlen(metadata->value.GetString())};
                jsonUtils::retrieveValue1(value1, MESSAGE_METADATA_STRING_KEY.data(), &metadata_string_value);
                if (expectedMessages->find(metadata_string_value) != expectedMessages->end()) {
                    expectedMessages->at(metadata_string_value) = expectedMessages->at(metadata_string_value) + 1;
                }
                metadata_string_value = "";
                jsonUtils::retrieveValue1(value1, MESSAGE_METADATA_STRING_KEY_WL.data(), &metadata_string_value);
                if (expectedMessages->find(metadata_string_value) != expectedMessages->end()) {
                    expectedMessages->at(metadata_string_value) = expectedMessages->at(metadata_string_value) + 1;
                }
                string metadata_uint_value;
                jsonUtils::retrieveValue1(value1, MESSAGE_METADATA_UINT_KEY.data(), &metadata_uint_value);
                if (expectedMessages->find(metadata_uint_value) != expectedMessages->end()) {
                    expectedMessages->at(metadata_uint_value) = expectedMessages->at(metadata_uint_value) + 1;
                }
                string metadata_int_value;
                jsonUtils::retrieveValue1(value1, MESSAGE_METADATA_INT_KEY.data(), &metadata_int_value);
                if (expectedMessages->find(metadata_int_value) != expectedMessages->end()) {
                    expectedMessages->at(metadata_int_value) = expectedMessages->at(metadata_int_value) + 1;
                }
                string metadata_double_value;
                jsonUtils::retrieveValue1(value1, MESSAGE_METADATA_DOUBLE_KEY.data(), &metadata_double_value);
                if (expectedMessages->find(metadata_double_value) != expectedMessages->end()) {
                    expectedMessages->at(metadata_double_value) = expectedMessages->at(metadata_double_value) + 1;
                }
                if (validateBoolean) {
                    bool metadata_boolean_value = false;
                    jsonUtils::retrieveValue2(value1, MESSAGE_METADATA_BOOLEAN_KEY.data(), &metadata_boolean_value);
                    ASSERT_TRUE(metadata_boolean_value);
                }
            }
            void AudioPlayerTest::extractPlaybackAttributes(shared_ptr<MessageRequest> request, PlaybackAttributes* actualPlaybackAttributes) {
                Document document;
                document.Parse(request->getJsonContent().c_str());
                EXPECT_FALSE(document.HasParseError())<< "rapidjson detected a parsing error at offset:" + to_string(document.GetErrorOffset()) +
                             ", error message: " + GetParseError_En(document.GetParseError());
                auto event = document.FindMember(MESSAGE_EVENT_KEY.data());
                EXPECT_NE(event, document.MemberEnd());
                auto payload = event->value.FindMember(MESSAGE_PAYLOAD_KEY.data());
                EXPECT_NE(payload, event->value.MemberEnd());
                auto playbackAttributes = payload->value.FindMember(MESSAGE_PLAYBACK_ATTRIBUTES_KEY.data());
                EXPECT_NE(playbackAttributes, payload->value.MemberEnd());
                string name;
                rapidjson::Value value{playbackAttributes->value.GetString(), strlen(playbackAttributes->value.GetString())};
                jsonUtils::retrieveValue1(value, MESSAGE_PLAYBACK_ATTRIBUTES_NAME_KEY.data(), &name);
                actualPlaybackAttributes->name = name;
                string codec;
                jsonUtils::retrieveValue1(value, MESSAGE_PLAYBACK_ATTRIBUTES_CODEC_KEY.data(), &codec);
                actualPlaybackAttributes->codec = codec;
                int64_t samplingRate;
                jsonUtils::retrieveValue3(value, MESSAGE_PLAYBACK_ATTRIBUTES_SAMPLING_RATE_KEY.data(), &samplingRate);
                actualPlaybackAttributes->samplingRateInHertz = (long)samplingRate;
                int64_t bitrate;
                jsonUtils::retrieveValue3(value, MESSAGE_PLAYBACK_ATTRIBUTES_BITRATE_KEY.data(), &bitrate);
                actualPlaybackAttributes->dataRateInBitsPerSecond = (long)bitrate;
            }
            void AudioPlayerTest::extractPlaybackReports(shared_ptr<MessageRequest> request, vector<PlaybackReport>* actualPlaybackReports) {
                rapidjson::Document document;
                document.Parse(request->getJsonContent().c_str());
                EXPECT_FALSE(document.HasParseError()) << "rapidjson detected a parsing error at offset:" + to_string(document.GetErrorOffset()) +
                             ", error message: " + GetParseError_En(document.GetParseError());
                auto event = document.FindMember(MESSAGE_EVENT_KEY.data());
                EXPECT_NE(event, document.MemberEnd());
                auto payload = event->value.FindMember(MESSAGE_PAYLOAD_KEY.data());
                EXPECT_NE(payload, event->value.MemberEnd());
                auto playbackReports = payload->value.FindMember(MESSAGE_PLAYBACK_REPORTS_KEY.data());
                if (playbackReports == payload->value.MemberEnd()) return;
                for (const auto& playbackReport : playbackReports->value.GetArray()) {
                    PlaybackReport actualPlaybackReport;
                    int64_t startOffset;
                    rapidjson::Value _playbackReport{playbackReport.GetString(), strlen(playbackReport.GetString())};
                    jsonUtils::retrieveValue3(_playbackReport, MESSAGE_PLAYBACK_REPORTS_START_OFFSET_KEY.data(), &startOffset);
                    actualPlaybackReport.startOffset = std::chrono::milliseconds(startOffset);
                    int64_t endOffset;
                    jsonUtils::retrieveValue3(_playbackReport, MESSAGE_PLAYBACK_REPORTS_END_OFFSET_KEY.data(), &endOffset);
                    actualPlaybackReport.endOffset = std::chrono::milliseconds(endOffset);
                    auto playbackAttributes = playbackReport.FindMember(MESSAGE_PLAYBACK_ATTRIBUTES_KEY.data());
                    EXPECT_NE(playbackAttributes, playbackReport.MemberEnd());
                    string name;
                    rapidjson::Value value{playbackAttributes->value.GetString(), strlen(playbackAttributes->value.GetString())};
                    jsonUtils::retrieveValue1(value, MESSAGE_PLAYBACK_ATTRIBUTES_NAME_KEY.data(), &name);
                    actualPlaybackReport.playbackAttributes.name = name;
                    string codec;
                    jsonUtils::retrieveValue1(value, MESSAGE_PLAYBACK_ATTRIBUTES_CODEC_KEY, &codec);
                    actualPlaybackReport.playbackAttributes.codec = codec;
                    int64_t samplingRate;
                    jsonUtils::retrieveValue3(value, MESSAGE_PLAYBACK_ATTRIBUTES_SAMPLING_RATE_KEY.data(), &samplingRate);
                    actualPlaybackReport.playbackAttributes.samplingRateInHertz = (long)samplingRate;
                    int64_t bitrate;
                    jsonUtils::retrieveValue3(value, MESSAGE_PLAYBACK_ATTRIBUTES_BITRATE_KEY.data(), &bitrate);
                    actualPlaybackReport.playbackAttributes.dataRateInBitsPerSecond = (long)bitrate;
                    actualPlaybackReports->push_back(actualPlaybackReport);
                }
            }
            bool AudioPlayerTest::extractMediaPlayerState(
                std::shared_ptr<avsCommon::avs::MessageRequest> request,
                const std::string& expectedState,
                MediaPlayerState* playerState) {
                rapidjson::Document document;
                document.Parse(request->getJsonContent().c_str());
                EXPECT_FALSE(document.HasParseError()) << "rapidjson detected a parsing error at offset:" + to_string(document.GetErrorOffset()) +
                             ", error message: " + GetParseError_En(document.GetParseError());
                auto event = document.FindMember(MESSAGE_EVENT_KEY.data());
                EXPECT_NE(event, document.MemberEnd());
                auto header = event->value.FindMember(MESSAGE_HEADER_KEY.data());
                EXPECT_NE(header, event->value.MemberEnd());
                string requestName;
                rapidjson::Value value{header->value.GetString(), strlen(header->value.GetString())};
                jsonUtils::retrieveValue1(value, MESSAGE_NAME_KEY.data(), &requestName);
                if (expectedState == requestName) {
                    auto payload = event->value.FindMember(MESSAGE_PAYLOAD_KEY.data());
                    EXPECT_NE(payload, event->value.MemberEnd());
                    uint64_t offset = 0;
                    rapidjson::Value _payload{payload->value.GetString(), strlen(payload->value.GetString())};
                    jsonUtils::retrieveValue4(_payload, OFFSET_KEY.data(), &offset);
                    playerState->offset = milliseconds(offset);
                    return true;
                }
                return false;
            }
            TEST_F(AudioPlayerTest, test_createWithNullPointers) {
                shared_ptr<AudioPlayer> testAudioPlayer;
                vector<shared_ptr<MediaPlayerInterface>> pool = { m_mockMediaPlayer, m_mockMediaPlayerTrack2, m_mockMediaPlayerTrack3 };
                testAudioPlayer = AudioPlayer::create(nullptr, m_mockMessageSender, m_mockFocusManager, m_mockContextManager, m_mockExceptionSender,
                                                      m_mockPlaybackRouter, {m_mockSpeaker}, m_mockCaptionManager, m_mockMetricRecorder);
                EXPECT_EQ(testAudioPlayer, nullptr);
                m_mockFactory = PooledMediaPlayerFactory::create(pool);
                testAudioPlayer = AudioPlayer::create(move(m_mockFactory), nullptr, m_mockFocusManager, m_mockContextManager, m_mockExceptionSender,
                                                      m_mockPlaybackRouter, {m_mockSpeaker}, m_mockCaptionManager, m_mockMetricRecorder);
                EXPECT_EQ(testAudioPlayer, nullptr);
                m_mockFactory = PooledMediaPlayerFactory::create(pool);
                testAudioPlayer = AudioPlayer::create(move(m_mockFactory),m_mockMessageSender,nullptr,
                                        m_mockContextManager,m_mockExceptionSender,m_mockPlaybackRouter,
                             {m_mockSpeaker},m_mockCaptionManager,m_mockMetricRecorder);
                EXPECT_EQ(testAudioPlayer, nullptr);
                m_mockFactory = PooledMediaPlayerFactory::create(pool);
                testAudioPlayer = AudioPlayer::create(move(m_mockFactory),m_mockMessageSender,m_mockFocusManager,
                                        nullptr,m_mockExceptionSender,m_mockPlaybackRouter,
                              {m_mockSpeaker},m_mockCaptionManager,m_mockMetricRecorder);
                EXPECT_EQ(testAudioPlayer, nullptr);
                m_mockFactory = PooledMediaPlayerFactory::create(pool);
                testAudioPlayer = AudioPlayer::create(move(m_mockFactory),m_mockMessageSender,m_mockFocusManager,
                                        m_mockContextManager,nullptr,m_mockPlaybackRouter,
                              {m_mockSpeaker},m_mockCaptionManager,m_mockMetricRecorder);
                EXPECT_EQ(testAudioPlayer, nullptr);
                m_mockFactory = PooledMediaPlayerFactory::create(pool);
                testAudioPlayer = AudioPlayer::create(move(m_mockFactory),m_mockMessageSender,m_mockFocusManager,
                                        m_mockContextManager,m_mockExceptionSender,nullptr,
                              {m_mockSpeaker},m_mockCaptionManager,m_mockMetricRecorder);
                EXPECT_EQ(testAudioPlayer, nullptr);
                m_mockFactory = PooledMediaPlayerFactory::create(pool);
                testAudioPlayer = AudioPlayer::create(move(m_mockFactory),m_mockMessageSender,m_mockFocusManager,
                                        m_mockContextManager,m_mockExceptionSender,m_mockPlaybackRouter,
                                                      {},m_mockCaptionManager,m_mockMetricRecorder);
                EXPECT_EQ(testAudioPlayer, nullptr);
            }
            TEST_F(AudioPlayerTest, test_transitionFromIdleToPlaying) {
                EXPECT_CALL(*(m_mockMediaPlayer.get()), play(_)).Times(AtLeast(1));
                sendPlayDirective();
            }
            TEST_F(AudioPlayerTest, test_transitionFromPlayingToStopped) {
                sendPlayDirective();
                EXPECT_CALL(*(m_mockMediaPlayer.get()), stop(_)).Times(AtLeast(1));
                sendStopDirective();
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::STOPPED, MY_WAIT_TIMEOUT));
            }
            TEST_F(AudioPlayerTest, test_transitionFromPlayingToStoppedWithClear) {
                sendPlayDirective();
                EXPECT_CALL(*(m_mockMediaPlayer.get()), stop(_)).Times(AtLeast(1));
                sendClearQueueDirective();
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::STOPPED, MY_WAIT_TIMEOUT));
            }
            TEST_F(AudioPlayerTest, test_transitionFromStoppedToPlaying) {
                sendPlayDirective();
                EXPECT_CALL(*(m_mockMediaPlayer.get()), stop(_)).Times(AtLeast(1));
                sendClearQueueDirective();
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::STOPPED, MY_WAIT_TIMEOUT));
                m_audioPlayer->onFocusChanged(FocusState::NONE, MixingBehavior::MUST_STOP);
                EXPECT_CALL(*(m_mockMediaPlayer.get()), play(_)).Times(0);
                EXPECT_CALL(*(m_mockMediaPlayerTrack2.get()), play(_)).Times(AtLeast(1));
                //EXPECT_CALL(*(m_mockFocusManager.get()), acquireChannel(CHANNEL_NAME, _)).Times(1).WillOnce(Return(true));
                auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_AUDIO_PLAYER, NAME_PLAY, MESSAGE_ID_TEST_2);
                shared_ptr<AVSDirective> playDirective = AVSDirective::create("", avsMessageHeader, createEnqueuePayloadTest(OFFSET_IN_MILLISECONDS_TEST,
                                                                              AUDIO_ITEM_ID_2),m_attachmentManager,CONTEXT_ID_TEST_2);
                m_audioPlayer->CapabilityAgent::preHandleDirective(playDirective, move(m_mockDirectiveHandlerResult));
                m_audioPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST_2);
                m_audioPlayer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PLAYING, MY_WAIT_TIMEOUT));
            }
            TEST_F(AudioPlayerTest, testTransitionFromStoppedToResumePlaying) {
                sendPlayDirective();
                EXPECT_CALL(*(m_mockMediaPlayer.get()), stop(_)).Times(AtLeast(1));
                sendClearQueueDirective();
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::STOPPED, MY_WAIT_TIMEOUT));
                m_audioPlayer->onFocusChanged(FocusState::NONE, MixingBehavior::MUST_STOP);
                m_attachmentManager = make_shared<AttachmentManager>(AttachmentManager::AttachmentType::IN_PROCESS);
                m_mockMediaPlayer->resetWaitTimer();
                EXPECT_CALL(*(m_mockMediaPlayerTrack2.get()), play(_)).Times(1);
                //EXPECT_CALL(*(m_mockFocusManager.get()), acquireChannel(CHANNEL_NAME, _)).Times(1).WillOnce(Return(true));
                auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_AUDIO_PLAYER, NAME_PLAY, MESSAGE_ID_TEST_2);
                shared_ptr<AVSDirective> playDirective = AVSDirective::create("", avsMessageHeader, createEnqueuePayloadTest(OFFSET_IN_MILLISECONDS_TEST),
                                                              m_attachmentManager, CONTEXT_ID_TEST);
                m_audioPlayer->CapabilityAgent::preHandleDirective(playDirective, move(m_mockDirectiveHandlerResult));
                m_audioPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST_2);
                m_audioPlayer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PLAYING, MY_WAIT_TIMEOUT));
            }
            TEST_F(AudioPlayerTest, testTransitionFromPlayingToPlayingNextEnqueuedTrack) {
                sendPlayDirective();
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PLAYING, MY_WAIT_TIMEOUT));
                this_thread::sleep_for(milliseconds(EVENT_PROCESS_DELAY));
                EXPECT_CALL(*(m_mockMediaPlayer.get()), play(_)).Times(0);
                EXPECT_CALL(*(m_mockMediaPlayerTrack2.get()), play(_)).Times(AtLeast(1));
                EXPECT_CALL(*(m_mockMediaPlayerTrack3.get()), play(_)).Times(0);
                auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_AUDIO_PLAYER, NAME_PLAY, MESSAGE_ID_TEST_2);
                shared_ptr<AVSDirective> playDirective = AVSDirective::create("", avsMessageHeader, createEnqueuePayloadTest(OFFSET_IN_MILLISECONDS_TEST, AUDIO_ITEM_ID_2),
                                                              m_attachmentManager, CONTEXT_ID_TEST_2);
                m_audioPlayer->CapabilityAgent::preHandleDirective(playDirective, move(m_mockDirectiveHandlerResult));
                m_audioPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST_2);
                this_thread::sleep_for(milliseconds(EVENT_PROCESS_DELAY));
                avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_AUDIO_PLAYER, NAME_PLAY, MESSAGE_ID_TEST_3);
                playDirective = AVSDirective::create("", avsMessageHeader, REPLACE_ALL_PAYLOAD_TEST, m_attachmentManager, CONTEXT_ID_TEST_3);
                m_audioPlayer->CapabilityAgent::preHandleDirective(playDirective, move(m_mockDirectiveHandlerResult));
                m_audioPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST_3);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::STOPPED, MY_WAIT_TIMEOUT));
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PLAYING, MY_WAIT_TIMEOUT));
            }
            TEST_F(AudioPlayerTest, test_transitionFromPlayingToPaused) {
                sendPlayDirective();
                EXPECT_CALL(*(m_mockMediaPlayer.get()), pause(_)).Times(AtLeast(1));
                m_audioPlayer->onFocusChanged(FocusState::BACKGROUND, MixingBehavior::MUST_PAUSE);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PAUSED, MY_WAIT_TIMEOUT));
            }
            TEST_F(AudioPlayerTest, test_transitionFromPausedToStopped) {
                sendPlayDirective();
                EXPECT_CALL(*(m_mockMediaPlayer.get()), stop(_)).Times(AtLeast(1));
                m_audioPlayer->onFocusChanged(FocusState::BACKGROUND, MixingBehavior::MUST_PAUSE);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PAUSED, MY_WAIT_TIMEOUT));
                sendClearQueueDirective();
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::STOPPED, MY_WAIT_TIMEOUT));
            }
            TEST_F(AudioPlayerTest, test_resumeAfterPaused) {
                sendPlayDirective();
                EXPECT_CALL(*(m_mockMediaPlayer.get()), stop(_)).Times(AtLeast(1));
                m_audioPlayer->onFocusChanged(FocusState::BACKGROUND, MixingBehavior::MUST_PAUSE);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PAUSED, MY_WAIT_TIMEOUT));
                EXPECT_CALL(*(m_mockMediaPlayer.get()), resume(_)).Times(AtLeast(1));
                m_audioPlayer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PLAYING, MY_WAIT_TIMEOUT));
            }
            TEST_F(AudioPlayerTest, test_callingProvideStateWhenIdle) {
                EXPECT_CALL(*(m_mockContextManager.get()),setState(NAMESPACE_AND_NAME_PLAYBACK_STATE, _, StateRefreshPolicy::NEVER, PROVIDE_STATE_TOKEN_TEST))
                    .Times(1).WillOnce(DoAll(Invoke([this](const NamespaceAndName& namespaceAndName, const string& jsonState, const StateRefreshPolicy& refreshPolicy,
                    const unsigned int stateRequestToken) { verifyState(jsonState, IDLE_STATE_TEST); }),InvokeWithoutArgs(this, &AudioPlayerTest::wakeOnSetState)));
                m_audioPlayer->provideState(NAMESPACE_AND_NAME_PLAYBACK_STATE, PROVIDE_STATE_TOKEN_TEST);
                ASSERT_TRUE(std::future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
            }
            TEST_F(AudioPlayerTest, test_onPlaybackError) {
                m_expectedMessages.insert({PLAYBACK_STARTED_NAME, 0});
                m_expectedMessages.insert({PLAYBACK_FAILED_NAME, 0});
                m_expectedMessages.insert({PLAYBACK_STOPPED_NAME, 0});
                EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_)).Times(AtLeast(1))
                    .WillRepeatedly(Invoke([this](std::shared_ptr<avsCommon::avs::MessageRequest> request) {
                        lock_guard<mutex> lock(m_mutex);
                        verifyMessageMap(request, &m_expectedMessages);
                        m_messageSentTrigger.notify_one();
                    }));
                sendPlayDirective();
                m_audioPlayer->onPlaybackError(m_mockMediaPlayer->getCurrentSourceId(),ErrorType::MEDIA_ERROR_UNKNOWN,"TEST_ERROR", DEFAULT_MEDIA_PLAYER_STATE);
                unique_lock<std::mutex> lock(m_mutex);
                bool result;
                result = m_messageSentTrigger.wait_for(lock, MY_WAIT_TIMEOUT, [this] {
                    for (auto messageStatus : m_expectedMessages) {
                        if (messageStatus.second == 0) return false;
                    }
                    return true;
                });
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::STOPPED, MY_WAIT_TIMEOUT));
                ASSERT_TRUE(result);
            }
            TEST_F(AudioPlayerTest, test_onPlaybackError_Stopped) {
                m_expectedMessages.insert({PLAYBACK_STARTED_NAME, 0});
                m_expectedMessages.insert({PLAYBACK_STOPPED_NAME, 0});
                m_expectedMessages.insert({PLAYBACK_FAILED_NAME, -1});
                EXPECT_CALL(*(m_mockMediaPlayer.get()), getOffset(_)).WillRepeatedly(Return(milliseconds(600)));
                EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_)).Times(AtLeast(1))
                    .WillRepeatedly(Invoke([this](shared_ptr<avsCommon::avs::MessageRequest> request) {
                        lock_guard<std::mutex> lock(m_mutex);
                        verifyMessageMap(request, &m_expectedMessages);
                        m_messageSentTrigger.notify_one();
                    }));
                sendPlayDirective();
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PLAYING, MY_WAIT_TIMEOUT));
                sendStopDirective();
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::STOPPED, MY_WAIT_TIMEOUT));
                m_audioPlayer->onPlaybackError(m_mockMediaPlayer->getCurrentSourceId(), ErrorType::MEDIA_ERROR_UNKNOWN,"TEST_ERROR", DEFAULT_MEDIA_PLAYER_STATE);
                unique_lock<std::mutex> lock(m_mutex);
                bool result;
                result = m_messageSentTrigger.wait_for(lock, MY_WAIT_TIMEOUT, [this] {
                    for (auto messageStatus : m_expectedMessages) {
                        if (messageStatus.second == 0) return false;
                    }
                    return true;
                });
                ASSERT_TRUE(result);
            }
            TEST_F(AudioPlayerTest, testPrebufferOnPlaybackError) {
                m_expectedMessages.insert({PLAYBACK_FAILED_NAME, 0});
                EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_))
                    .Times(AtLeast(1))
                    .WillRepeatedly(Invoke([this](shared_ptr<MessageRequest> request) {
                        lock_guard<mutex> lock(m_mutex);
                        verifyMessageMap(request, &m_expectedMessages);
                        m_messageSentTrigger.notify_one();
                    }));
                sendPlayDirective();
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PLAYING, MY_WAIT_TIMEOUT));
                bool result;
                this_thread::sleep_for(milliseconds(EVENT_PROCESS_DELAY));
                EXPECT_CALL(*(m_mockMediaPlayer.get()), play(_)).Times(0);
                EXPECT_CALL(*(m_mockMediaPlayerTrack2.get()), play(_)).Times(0);
                auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_AUDIO_PLAYER, NAME_PLAY, MESSAGE_ID_TEST_2);
                shared_ptr<AVSDirective> playDirective = AVSDirective::create("", avsMessageHeader, createEnqueuePayloadTest(OFFSET_IN_MILLISECONDS_TEST, AUDIO_ITEM_ID_2),
                                                              m_attachmentManager, CONTEXT_ID_TEST_2);
                m_audioPlayer->CapabilityAgent::preHandleDirective(playDirective, std::move(m_mockDirectiveHandlerResult));
                this_thread::sleep_for(milliseconds(EVENT_PROCESS_DELAY));
                m_audioPlayer->onPlaybackError(m_mockMediaPlayerTrack2->getSourceId(),ErrorType::MEDIA_ERROR_UNKNOWN,"TEST_ERROR", DEFAULT_MEDIA_PLAYER_STATE);
                m_audioPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST_2);
                this_thread::sleep_for(milliseconds(EVENT_PROCESS_DELAY));
                {
                    unique_lock<mutex> lock(m_mutex);
                    result = m_messageSentTrigger.wait_for(lock, MY_WAIT_TIMEOUT, [this] {
                        for (auto messageStatus : m_expectedMessages) {
                            if (messageStatus.second == 0) return false;
                        }
                        return true;
                    });
                    ASSERT_FALSE(result);
                }
                unique_lock<mutex> lock(m_mutex);
                m_audioPlayer->onPlaybackFinished(m_mockMediaPlayer->getSourceId(), DEFAULT_MEDIA_PLAYER_STATE);
                result = m_messageSentTrigger.wait_for(lock, MY_WAIT_TIMEOUT, [this] {
                    for (auto messageStatus : m_expectedMessages) {
                        if (messageStatus.second == 0) return false;
                    }
                    return true;
                });
                ASSERT_TRUE(result);
            }
            TEST_F(AudioPlayerTest, test_onPlaybackPaused) {
                m_expectedMessages.insert({PLAYBACK_STARTED_NAME, 0});
                m_expectedMessages.insert({PLAYBACK_PAUSED_NAME, 0});
                EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_))
                    .Times(AtLeast(1))
                    .WillRepeatedly(Invoke([this](shared_ptr<MessageRequest> request) {
                        lock_guard<mutex> lock(m_mutex);
                        verifyMessageMap(request, &m_expectedMessages);
                        m_messageSentTrigger.notify_one();
                    }));
                sendPlayDirective();
                m_audioPlayer->onFocusChanged(FocusState::BACKGROUND, MixingBehavior::MUST_PAUSE);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PAUSED, MY_WAIT_TIMEOUT));
                unique_lock<mutex> lock(m_mutex);
                bool result;
                result = m_messageSentTrigger.wait_for(lock, MY_WAIT_TIMEOUT, [this] {
                    for (auto messageStatus : m_expectedMessages) {
                        if (messageStatus.second == 0) return false;
                    }
                    return true;
                });
                ASSERT_TRUE(result);
            }
            TEST_F(AudioPlayerTest, test_onPlaybackResumed) {
                m_expectedMessages.insert({PLAYBACK_STARTED_NAME, 0});
                m_expectedMessages.insert({PLAYBACK_RESUMED_NAME, 0});
                EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_))
                    .Times(AtLeast(1))
                    .WillRepeatedly(Invoke([this](shared_ptr<MessageRequest> request) {
                        lock_guard<mutex> lock(m_mutex);
                        verifyMessageMap(request, &m_expectedMessages);
                        m_messageSentTrigger.notify_one();
                    }));
                sendPlayDirective();
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PLAYING, MY_WAIT_TIMEOUT));
                bool result;
                m_audioPlayer->onPlaybackResumed(m_mockMediaPlayer->getCurrentSourceId(), DEFAULT_MEDIA_PLAYER_STATE);
                unique_lock<std::mutex> lock(m_mutex);
                result = m_messageSentTrigger.wait_for(lock, MY_WAIT_TIMEOUT, [this] {
                    for (auto messageStatus : m_expectedMessages) {
                        if (messageStatus.second == 0) return false;
                    }
                    return true;
                });
                ASSERT_TRUE(result);
            }
            TEST_F(AudioPlayerTest, test_onPlaybackFinished_bufferCompleteAfterStarted) {
                m_expectedMessages.insert({PLAYBACK_STARTED_NAME, 0});
                m_expectedMessages.insert({PLAYBACK_NEARLY_FINISHED_NAME, 0});
                m_expectedMessages.insert({PLAYBACK_FINISHED_NAME, 0});
                EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_)).Times(AtLeast(1))
                    .WillRepeatedly(Invoke([this](shared_ptr<MessageRequest> request) {
                        lock_guard<std::mutex> lock(m_mutex);
                        verifyMessageMap(request, &m_expectedMessages);
                        m_messageSentTrigger.notify_one();
                    }));
                sendPlayDirective();
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PLAYING, MY_WAIT_TIMEOUT));
                m_audioPlayer->onPlaybackFinished(m_mockMediaPlayer->getCurrentSourceId(), DEFAULT_MEDIA_PLAYER_STATE);
                unique_lock<mutex> lock(m_mutex);
                bool result;
                result = m_messageSentTrigger.wait_for(lock, MY_WAIT_TIMEOUT, [this] {
                    for (auto messageStatus : m_expectedMessages) {
                        if (messageStatus.second == 0) return false;
                    }
                    return true;
                });
                ASSERT_TRUE(result);
            }
            TEST_F(AudioPlayerTest, test_onPlaybackFinished_bufferCompleteBeforeStarted) {
                m_expectedMessages.insert({PLAYBACK_STARTED_NAME, 0});
                m_expectedMessages.insert({PLAYBACK_NEARLY_FINISHED_NAME, 0});
                m_expectedMessages.insert({PLAYBACK_FINISHED_NAME, 0});
                EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_)).Times(AtLeast(1))
                    .WillRepeatedly(Invoke([this](shared_ptr<MessageRequest> request) {
                        lock_guard<mutex> lock(m_mutex);
                        verifyMessageMap(request, &m_expectedMessages);
                        m_messageSentTrigger.notify_one();
                    }));
                auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_AUDIO_PLAYER, NAME_PLAY, MESSAGE_ID_TEST, PLAY_REQUEST_ID_TEST);
                shared_ptr<AVSDirective> playDirective = AVSDirective::create("", avsMessageHeader, createEnqueuePayloadTest(OFFSET_IN_MILLISECONDS_TEST),
                                                              m_attachmentManager, CONTEXT_ID_TEST);
                //EXPECT_CALL(*(m_mockFocusManager.get()), acquireChannel(CHANNEL_NAME, _)).Times(1).WillOnce(InvokeWithoutArgs(this, &AudioPlayerTest::wakeOnAcquireChannel));
                EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted());
                m_audioPlayer->CapabilityAgent::preHandleDirective(playDirective, std::move(m_mockDirectiveHandlerResult));
                m_audioPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                ASSERT_EQ(future_status::ready, m_wakeAcquireChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                m_audioPlayer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PLAYING, MY_WAIT_TIMEOUT));
                m_audioPlayer->onBufferingComplete(m_mockMediaPlayer->getCurrentSourceId(), DEFAULT_MEDIA_PLAYER_STATE);
                m_audioPlayer->onPlaybackFinished(m_mockMediaPlayer->getCurrentSourceId(), DEFAULT_MEDIA_PLAYER_STATE);
                bool result;
                unique_lock<mutex> lock(m_mutex);
                result = m_messageSentTrigger.wait_for(lock, MY_WAIT_TIMEOUT, [this] {
                    for (auto messageStatus : m_expectedMessages) {
                        if (messageStatus.second == 0) return false;
                    }
                    return true;
                });
                ASSERT_TRUE(result);
            }
            TEST_F(AudioPlayerTest, testOnPlaybackFinishedWithPlaybackAttributes) {
                PlaybackAttributes expectedPlaybackAttributes = {MESSAGE_PLAYBACK_ATTRIBUTES_NAME_VALUE, MESSAGE_PLAYBACK_ATTRIBUTES_CODEC_VALUE,
                                                                 MESSAGE_PLAYBACK_ATTRIBUTES_SAMPLING_RATE_VALUE, MESSAGE_PLAYBACK_ATTRIBUTES_BITRATE_VALUE};
                EXPECT_CALL(*(m_mockMediaPlayer.get()), getPlaybackAttributes()).WillRepeatedly(Return(Optional<PlaybackAttributes>(expectedPlaybackAttributes)));
                PlaybackAttributes* actualPlaybackAttributes = new PlaybackAttributes();
                EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_)).Times(AtLeast(1))
                    .WillRepeatedly(
                        Invoke([this, actualPlaybackAttributes](std::shared_ptr<avsCommon::avs::MessageRequest> request) {
                            std::lock_guard<std::mutex> lock(m_mutex);
                            extractPlaybackAttributes(request, actualPlaybackAttributes);
                            m_messageSentTrigger.notify_one();
                        }));
                sendPlayDirective();
                m_audioPlayer->onPlaybackFinished(m_mockMediaPlayer->getCurrentSourceId(), DEFAULT_MEDIA_PLAYER_STATE);
                std::unique_lock<std::mutex> lock(m_mutex);
                bool result;
                result = m_messageSentTrigger.wait_for(lock, MY_WAIT_TIMEOUT, [expectedPlaybackAttributes, actualPlaybackAttributes] {
                        if (actualPlaybackAttributes->name != expectedPlaybackAttributes.name ||
                            actualPlaybackAttributes->codec != expectedPlaybackAttributes.codec ||
                            actualPlaybackAttributes->samplingRateInHertz != expectedPlaybackAttributes.samplingRateInHertz ||
                            actualPlaybackAttributes->dataRateInBitsPerSecond != expectedPlaybackAttributes.dataRateInBitsPerSecond) {
                            return false;
                        }
                        return true;
                    });
                ASSERT_TRUE(result);
            }
            TEST_F(AudioPlayerTest, testOnPlaybackStoppedWithPlaybackReports) {
                PlaybackAttributes expectedPlaybackAttributes = {MESSAGE_PLAYBACK_ATTRIBUTES_NAME_VALUE, MESSAGE_PLAYBACK_ATTRIBUTES_CODEC_VALUE,
                                                                 MESSAGE_PLAYBACK_ATTRIBUTES_SAMPLING_RATE_VALUE, MESSAGE_PLAYBACK_ATTRIBUTES_BITRATE_VALUE};
                PlaybackReport expectedPlaybackReport = {milliseconds(MESSAGE_PLAYBACK_REPORTS_START_OFFSET_VALUE),
                                                          milliseconds(MESSAGE_PLAYBACK_REPORTS_END_OFFSET_VALUE), expectedPlaybackAttributes};
                EXPECT_CALL(*(m_mockMediaPlayer.get()), getPlaybackReports()).WillRepeatedly(Return(vector<PlaybackReport>{expectedPlaybackReport}));
                vector<PlaybackReport>* actualPlaybackReports = new vector<PlaybackReport>();
                EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_)).Times(AtLeast(1))
                    .WillRepeatedly(Invoke([this, actualPlaybackReports](shared_ptr<MessageRequest> request) {
                        lock_guard<mutex> lock(m_mutex);
                        extractPlaybackReports(request, actualPlaybackReports);
                        m_messageSentTrigger.notify_one();
                    }));
                sendPlayDirective();
                m_audioPlayer->onPlaybackStopped(m_mockMediaPlayer->getCurrentSourceId(), DEFAULT_MEDIA_PLAYER_STATE);
                unique_lock<mutex> lock(m_mutex);
                bool result;
                result = m_messageSentTrigger.wait_for(lock, MY_WAIT_TIMEOUT, [expectedPlaybackReport, expectedPlaybackAttributes, actualPlaybackReports] {
                        if (actualPlaybackReports->size() != 1) return false;
                        PlaybackAttributes actualPlaybackAttributes = actualPlaybackReports->at(0).playbackAttributes;
                        if (actualPlaybackReports->at(0).startOffset != expectedPlaybackReport.startOffset ||
                            actualPlaybackReports->at(0).endOffset != expectedPlaybackReport.endOffset ||
                            actualPlaybackAttributes.name != expectedPlaybackAttributes.name ||
                            actualPlaybackAttributes.codec != expectedPlaybackAttributes.codec ||
                            actualPlaybackAttributes.samplingRateInHertz != expectedPlaybackAttributes.samplingRateInHertz ||
                            actualPlaybackAttributes.dataRateInBitsPerSecond != expectedPlaybackAttributes.dataRateInBitsPerSecond) {
                            return false;
                        }
                        return true;
                    });
                ASSERT_TRUE(result);
            }
            TEST_F(AudioPlayerTest, test_onBufferUnderrun) {
                m_expectedMessages.insert({PLAYBACK_STARTED_NAME, 0});
                m_expectedMessages.insert({PLAYBACK_STUTTER_STARTED_NAME, 0});
                EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_)).Times(AtLeast(1))
                    .WillRepeatedly(Invoke([this](shared_ptr<MessageRequest> request) {
                        lock_guard<mutex> lock(m_mutex);
                        verifyMessageMap(request, &m_expectedMessages);
                        m_messageSentTrigger.notify_one();
                    }));
                sendPlayDirective();
                m_audioPlayer->onBufferUnderrun(m_mockMediaPlayer->getCurrentSourceId(), DEFAULT_MEDIA_PLAYER_STATE);
                unique_lock<mutex> lock(m_mutex);
                bool result;
                result = m_messageSentTrigger.wait_for(lock, MY_WAIT_TIMEOUT, [this] {
                    for (auto messageStatus : m_expectedMessages) {
                        if (messageStatus.second == 0) return false;
                    }
                    return true;
                });
                ASSERT_TRUE(result);
            }
            TEST_F(AudioPlayerTest, testTimer_onBufferRefilled) {
                m_expectedMessages.insert({PLAYBACK_STARTED_NAME, 0});
                m_expectedMessages.insert({PLAYBACK_STUTTER_FINISHED_NAME, 0});
                EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_)).Times(AtLeast(1))
                    .WillRepeatedly(Invoke([this](std::shared_ptr<avsCommon::avs::MessageRequest> request) {
                        lock_guard<mutex> lock(m_mutex);
                        verifyMessageMap(request, &m_expectedMessages);
                        m_messageSentTrigger.notify_one();
                    }));
                sendPlayDirective();
                m_audioPlayer->onBufferRefilled(m_mockMediaPlayer->getCurrentSourceId(), DEFAULT_MEDIA_PLAYER_STATE);
                unique_lock<mutex> lock(m_mutex);
                bool result;
                result = m_messageSentTrigger.wait_for(lock, MY_WAIT_TIMEOUT, [this] {
                    for (auto messageStatus : m_expectedMessages) {
                        if (messageStatus.second == 0) return false;
                    }
                    return true;
                });
                ASSERT_TRUE(result);
            }
            TEST_F(AudioPlayerTest, test_onTags_filteredOut) {
                sendPlayDirective();
                m_expectedMessages.insert({STREAM_METADATA_EXTRACTED_NAME, 0});
                m_expectedMessages.insert({MESSAGE_METADATA_STRING_VALUE, 0});
                m_expectedMessages.insert({MESSAGE_METADATA_UINT_VALUE, 0});
                m_expectedMessages.insert({MESSAGE_METADATA_DOUBLE_VALUE, 0});
                EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_))
                    .WillRepeatedly(Invoke([this](shared_ptr<MessageRequest> request) {
                        if (!m_mockMediaPlayer->waitUntilPlaybackStopped(milliseconds(0))) {
                            lock_guard<mutex> lock(m_mutex);
                            verifyTags(request, &m_expectedMessages);
                            m_messageSentTrigger.notify_one();
                        }
                    }));
                unique_ptr<AudioPlayer::VectorOfTags> ptrToVectorOfTags = make_unique<AudioPlayer::VectorOfTags>();
                auto vectorOfTags = ptrToVectorOfTags.get();
                AudioPlayer::TagKeyValueType stringTag, uintTag, intTag, doubleTag, booleanTag;
                stringTag.key = string(MESSAGE_METADATA_STRING_KEY);
                stringTag.value = string(MESSAGE_METADATA_STRING_VALUE);
                stringTag.type = AudioPlayer::TagType::STRING;
                vectorOfTags->push_back(stringTag);
                uintTag.key = string(MESSAGE_METADATA_UINT_KEY);
                uintTag.value = string(MESSAGE_METADATA_UINT_VALUE);
                uintTag.type = AudioPlayer::TagType::UINT;
                vectorOfTags->push_back(uintTag);
                intTag.key = string(MESSAGE_METADATA_INT_KEY);
                intTag.value = string(MESSAGE_METADATA_INT_VALUE);
                intTag.type = AudioPlayer::TagType::INT;
                vectorOfTags->push_back(intTag);
                doubleTag.key = string(MESSAGE_METADATA_DOUBLE_KEY);
                doubleTag.value = string(MESSAGE_METADATA_DOUBLE_VALUE);
                doubleTag.type = AudioPlayer::TagType::DOUBLE;
                vectorOfTags->push_back(doubleTag);
                booleanTag.key = string(MESSAGE_METADATA_BOOLEAN_KEY);
                booleanTag.value = string(MESSAGE_METADATA_BOOLEAN_VALUE);
                booleanTag.type = AudioPlayer::TagType::BOOLEAN;
                vectorOfTags->push_back(booleanTag);
                m_audioPlayer->onTags(m_mockMediaPlayer->getCurrentSourceId(), std::move(ptrToVectorOfTags), DEFAULT_MEDIA_PLAYER_STATE);
                unique_lock<mutex> lock(m_mutex);
                auto result = m_messageSentTrigger.wait_for(lock, MY_WAIT_TIMEOUT, [this] {
                    for (auto messageStatus : m_expectedMessages) {
                        if (messageStatus.second == 0) return false;
                    }
                    return true;
                });
                ASSERT_FALSE(result);
            }
            TEST_F(AudioPlayerTest, test_onTags_filteredIn) {
                sendPlayDirective();
                m_expectedMessages.insert({STREAM_METADATA_EXTRACTED_NAME, 0});
                m_expectedMessages.insert({MESSAGE_METADATA_STRING_VALUE, 0});
                m_expectedMessages.insert({MESSAGE_METADATA_UINT_VALUE, -1});
                m_expectedMessages.insert({MESSAGE_METADATA_DOUBLE_VALUE, -1});
                EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_)).Times(AtLeast(1))
                    .WillRepeatedly(Invoke([this](shared_ptr<MessageRequest> request) {
                        if (!m_mockMediaPlayer->waitUntilPlaybackStopped(milliseconds(0))) {
                            lock_guard<mutex> lock(m_mutex);
                            verifyTags(request, &m_expectedMessages, false);
                            m_messageSentTrigger.notify_one();
                        }
                    }));
                unique_ptr<AudioPlayer::VectorOfTags> ptrToVectorOfTags = make_unique<AudioPlayer::VectorOfTags>();
                auto vectorOfTags = ptrToVectorOfTags.get();
                AudioPlayer::TagKeyValueType stringTag, uintTag, intTag, doubleTag, booleanTag;
                stringTag.key = string(MESSAGE_METADATA_STRING_KEY_WL);
                stringTag.value = string(MESSAGE_METADATA_STRING_VALUE);
                stringTag.type = AudioPlayer::TagType::STRING;
                vectorOfTags->push_back(stringTag);
                uintTag.key = string(MESSAGE_METADATA_UINT_KEY);
                uintTag.value = string(MESSAGE_METADATA_UINT_VALUE);
                uintTag.type = AudioPlayer::TagType::UINT;
                vectorOfTags->push_back(uintTag);
                intTag.key = string(MESSAGE_METADATA_INT_KEY);
                intTag.value = string(MESSAGE_METADATA_INT_VALUE);
                intTag.type = AudioPlayer::TagType::INT;
                vectorOfTags->push_back(intTag);
                doubleTag.key = string(MESSAGE_METADATA_DOUBLE_KEY);
                doubleTag.value = string(MESSAGE_METADATA_DOUBLE_VALUE);
                doubleTag.type = AudioPlayer::TagType::DOUBLE;
                vectorOfTags->push_back(doubleTag);
                booleanTag.key = string(MESSAGE_METADATA_BOOLEAN_KEY);
                booleanTag.value = string(MESSAGE_METADATA_BOOLEAN_VALUE);
                booleanTag.type = AudioPlayer::TagType::BOOLEAN;
                vectorOfTags->push_back(booleanTag);
                m_audioPlayer->onTags(m_mockMediaPlayer->getCurrentSourceId(), move(ptrToVectorOfTags), DEFAULT_MEDIA_PLAYER_STATE);
                unique_lock<mutex> lock(m_mutex);
                auto result = m_messageSentTrigger.wait_for(lock, MY_WAIT_TIMEOUT, [this] {
                    for (auto messageStatus : m_expectedMessages) {
                        if (messageStatus.second == 0) return false;
                    }
                    return true;
                });
                ASSERT_TRUE(result);
            }
            TEST_F(AudioPlayerTest, test_onTags_filteredIn_rateCheck) {
                sendPlayDirective();
                m_expectedMessages.insert({STREAM_METADATA_EXTRACTED_NAME, 0});
                m_expectedMessages.insert({MESSAGE_METADATA_STRING_VALUE, 0});
                m_expectedMessages.insert({MESSAGE_METADATA_UINT_VALUE, -1});
                m_expectedMessages.insert({MESSAGE_METADATA_DOUBLE_VALUE, -1});
                EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_)).Times(AtLeast(1))
                    .WillRepeatedly(Invoke([this](shared_ptr<MessageRequest> request) {
                        if (!m_mockMediaPlayer->waitUntilPlaybackStopped(milliseconds(0))) {
                            lock_guard<mutex> lock(m_mutex);
                            verifyTags(request, &m_expectedMessages, false);
                            m_messageSentTrigger.notify_one();
                        }
                    }));
                unique_ptr<AudioPlayer::VectorOfTags> ptrToVectorOfTags = make_unique<AudioPlayer::VectorOfTags>();
                auto vectorOfTags = ptrToVectorOfTags.get();
                AudioPlayer::TagKeyValueType stringTag, uintTag, intTag, doubleTag, booleanTag;
                stringTag.key = string(MESSAGE_METADATA_STRING_KEY_WL);
                stringTag.value = string(MESSAGE_METADATA_STRING_VALUE);
                stringTag.type = AudioPlayer::TagType::STRING;
                vectorOfTags->push_back(stringTag);
                uintTag.key = string(MESSAGE_METADATA_UINT_KEY);
                uintTag.value = string(MESSAGE_METADATA_UINT_VALUE);
                uintTag.type = AudioPlayer::TagType::UINT;
                vectorOfTags->push_back(uintTag);
                intTag.key = string(MESSAGE_METADATA_INT_KEY);
                intTag.value = string(MESSAGE_METADATA_INT_VALUE);
                intTag.type = AudioPlayer::TagType::INT;
                vectorOfTags->push_back(intTag);
                doubleTag.key = string(MESSAGE_METADATA_DOUBLE_KEY);
                doubleTag.value = string(MESSAGE_METADATA_DOUBLE_VALUE);
                doubleTag.type = AudioPlayer::TagType::DOUBLE;
                vectorOfTags->push_back(doubleTag);
                booleanTag.key = string(MESSAGE_METADATA_BOOLEAN_KEY);
                booleanTag.value = string(MESSAGE_METADATA_BOOLEAN_VALUE);
                booleanTag.type = AudioPlayer::TagType::BOOLEAN;
                vectorOfTags->push_back(booleanTag);
                unique_ptr<AudioPlayer::VectorOfTags> ptrToVectorOfTags1;// = make_unique<AudioPlayer::VectorOfTags>(vectorOfTags->begin(), vectorOfTags->end());
                m_audioPlayer->onTags(m_mockMediaPlayer->getCurrentSourceId(), std::move(ptrToVectorOfTags1), DEFAULT_MEDIA_PLAYER_STATE);
                {
                    std::unique_lock<std::mutex> lock(m_mutex);
                    auto result = m_messageSentTrigger.wait_for(lock, MY_WAIT_TIMEOUT, [this] {
                        for (auto messageStatus : m_expectedMessages) {
                            if (messageStatus.second == 0) return false;
                        }
                        return true;
                    });
                    ASSERT_TRUE(result);
                }
                m_expectedMessages.clear();
                m_expectedMessages.insert({STREAM_METADATA_EXTRACTED_NAME, 0});
                m_expectedMessages.insert({MESSAGE_METADATA_STRING_VALUE_ALT, 0});
                m_expectedMessages.insert({MESSAGE_METADATA_UINT_VALUE, -1});
                m_expectedMessages.insert({MESSAGE_METADATA_DOUBLE_VALUE, -1});
                for (auto iter = vectorOfTags->begin(); iter != vectorOfTags->end(); ++iter) {
                    if (iter->key == MESSAGE_METADATA_STRING_KEY_WL) {
                        iter->value = MESSAGE_METADATA_STRING_VALUE_ALT;
                        break;
                    }
                }
                unique_ptr<AudioPlayer::VectorOfTags> ptrToVectorOfTags2;// = make_unique<AudioPlayer::VectorOfTags>(vectorOfTags->begin(), vectorOfTags->end());
                m_audioPlayer->onTags(m_mockMediaPlayer->getCurrentSourceId(), move(ptrToVectorOfTags2), DEFAULT_MEDIA_PLAYER_STATE);
                {
                    unique_lock<mutex> lock(m_mutex);
                    auto result = m_messageSentTrigger.wait_for(lock, MY_WAIT_TIMEOUT, [this] {
                        for (auto messageStatus : m_expectedMessages) {
                            if (messageStatus.second == 0) return false;
                        }
                        return true;
                    });
                    ASSERT_FALSE(result);
                }
                this_thread::sleep_for(milliseconds(METADATA_EVENT_DELAY));
                m_audioPlayer->onTags(m_mockMediaPlayer->getCurrentSourceId(), std::move(ptrToVectorOfTags), DEFAULT_MEDIA_PLAYER_STATE);
                {
                    unique_lock<mutex> lock(m_mutex);
                    auto result = m_messageSentTrigger.wait_for(lock, MY_WAIT_TIMEOUT, [this] {
                        for (auto messageStatus : m_expectedMessages) {
                            if (messageStatus.second == 0) return false;
                        }
                        return true;
                    });
                    ASSERT_TRUE(result);
                }
            }
            TEST_F(AudioPlayerTest, test_onTags_filteredIn_duplicateCheck) {
                sendPlayDirective();
                m_expectedMessages.insert({STREAM_METADATA_EXTRACTED_NAME, 0});
                m_expectedMessages.insert({MESSAGE_METADATA_STRING_VALUE, 0});
                m_expectedMessages.insert({MESSAGE_METADATA_UINT_VALUE, -1});
                m_expectedMessages.insert({MESSAGE_METADATA_DOUBLE_VALUE, -1});
                EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_))
                    .Times(AtLeast(1))
                    .WillRepeatedly(Invoke([this](shared_ptr<MessageRequest> request) {
                        if (!m_mockMediaPlayer->waitUntilPlaybackStopped(milliseconds(0))) {
                            lock_guard<mutex> lock(m_mutex);
                            verifyTags(request, &m_expectedMessages, false);
                            m_messageSentTrigger.notify_one();
                        }
                    }));
                unique_ptr<AudioPlayer::VectorOfTags> ptrToVectorOfTags = make_unique<AudioPlayer::VectorOfTags>();
                auto vectorOfTags = ptrToVectorOfTags.get();
                AudioPlayer::TagKeyValueType stringTag, uintTag, intTag, doubleTag, booleanTag;
                stringTag.key = string(MESSAGE_METADATA_STRING_KEY_WL);
                stringTag.value = string(MESSAGE_METADATA_STRING_VALUE);
                stringTag.type = AudioPlayer::TagType::STRING;
                vectorOfTags->push_back(stringTag);
                uintTag.key = string(MESSAGE_METADATA_UINT_KEY);
                uintTag.value = string(MESSAGE_METADATA_UINT_VALUE);
                uintTag.type = AudioPlayer::TagType::UINT;
                vectorOfTags->push_back(uintTag);
                intTag.key = string(MESSAGE_METADATA_INT_KEY);
                intTag.value = string(MESSAGE_METADATA_INT_VALUE);
                intTag.type = AudioPlayer::TagType::INT;
                vectorOfTags->push_back(intTag);
                doubleTag.key = string(MESSAGE_METADATA_DOUBLE_KEY);
                doubleTag.value = string(MESSAGE_METADATA_DOUBLE_VALUE);
                doubleTag.type = AudioPlayer::TagType::DOUBLE;
                vectorOfTags->push_back(doubleTag);
                booleanTag.key = string(MESSAGE_METADATA_BOOLEAN_KEY);
                booleanTag.value = string(MESSAGE_METADATA_BOOLEAN_VALUE);
                booleanTag.type = AudioPlayer::TagType::BOOLEAN;
                vectorOfTags->push_back(booleanTag);
                unique_ptr<AudioPlayer::VectorOfTags> ptrToVectorOfTags1;// = make_unique<AudioPlayer::VectorOfTags>(vectorOfTags->begin(), vectorOfTags->end());
                m_audioPlayer->onTags(m_mockMediaPlayer->getCurrentSourceId(), move(ptrToVectorOfTags1), DEFAULT_MEDIA_PLAYER_STATE);
                {
                    unique_lock<mutex> lock(m_mutex);
                    auto result = m_messageSentTrigger.wait_for(lock, MY_WAIT_TIMEOUT, [this] {
                        for (auto messageStatus : m_expectedMessages) {
                            if (messageStatus.second == 0) return false;
                        }
                        return true;
                    });
                    ASSERT_TRUE(result);
                }
                this_thread::sleep_for(milliseconds(METADATA_EVENT_DELAY));
                m_expectedMessages.clear();
                m_expectedMessages.insert({STREAM_METADATA_EXTRACTED_NAME, 0});
                m_expectedMessages.insert({MESSAGE_METADATA_STRING_VALUE, 0});
                m_expectedMessages.insert({MESSAGE_METADATA_UINT_VALUE, -1});
                m_expectedMessages.insert({MESSAGE_METADATA_DOUBLE_VALUE, -1});
                unique_ptr<AudioPlayer::VectorOfTags> ptrToVectorOfTags2;// = make_unique<AudioPlayer::VectorOfTags>(vectorOfTags->begin(), vectorOfTags->end());
                m_audioPlayer->onTags(m_mockMediaPlayer->getCurrentSourceId(), move(ptrToVectorOfTags2), DEFAULT_MEDIA_PLAYER_STATE);
                {
                    unique_lock<std::mutex> lock(m_mutex);
                    auto result = m_messageSentTrigger.wait_for(lock, MY_WAIT_TIMEOUT, [this] {
                        for (auto messageStatus : m_expectedMessages) {
                            if (messageStatus.second == 0) return false;
                        }
                        return true;
                    });
                    ASSERT_FALSE(result);
                }
                this_thread::sleep_for(milliseconds(METADATA_EVENT_DELAY));
                m_expectedMessages.clear();
                m_expectedMessages.insert({STREAM_METADATA_EXTRACTED_NAME, 0});
                m_expectedMessages.insert({MESSAGE_METADATA_STRING_VALUE_ALT, 0});
                m_expectedMessages.insert({MESSAGE_METADATA_UINT_VALUE, -1});
                m_expectedMessages.insert({MESSAGE_METADATA_DOUBLE_VALUE, -1});
                for (auto iter = vectorOfTags->begin(); iter != vectorOfTags->end(); ++iter) {
                    if (iter->key == MESSAGE_METADATA_STRING_KEY_WL) {
                        iter->value = MESSAGE_METADATA_STRING_VALUE_ALT;
                        break;
                    }
                }
                m_audioPlayer->onTags(m_mockMediaPlayer->getCurrentSourceId(), move(ptrToVectorOfTags), DEFAULT_MEDIA_PLAYER_STATE);
                {
                    unique_lock<mutex> lock(m_mutex);
                    auto result = m_messageSentTrigger.wait_for(lock, MY_WAIT_TIMEOUT, [this] {
                        for (auto messageStatus : m_expectedMessages) {
                            if (messageStatus.second == 0) return false;
                        }
                        return true;
                    });
                    ASSERT_TRUE(result);
                }
            }
            TEST_F(AudioPlayerTest, test_cancelDirective) {
                sendPlayDirective();
                m_audioPlayer->CapabilityAgent::cancelDirective(MESSAGE_ID_TEST);
                ASSERT_FALSE(m_audioPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST));
            }
            TEST_F(AudioPlayerTest, test_focusChangeToNoneInIdleState) {
                m_audioPlayer->onFocusChanged(FocusState::NONE, MixingBehavior::MUST_STOP);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::IDLE, MY_WAIT_TIMEOUT));
            }
            TEST_F(AudioPlayerTest, test_focusChangeFromForegroundToBackgroundInIdleState) {
                m_audioPlayer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::IDLE, MY_WAIT_TIMEOUT));
                m_audioPlayer->onFocusChanged(FocusState::BACKGROUND, MixingBehavior::MUST_PAUSE);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::IDLE, MY_WAIT_TIMEOUT));
            }
            TEST_F(AudioPlayerTest, test_focusChangeFromNoneToBackgroundInIdleState) {
                m_audioPlayer->onFocusChanged(FocusState::BACKGROUND, MixingBehavior::MUST_PAUSE);
            }
            TEST_F(AudioPlayerTest, test_focusChangesInPlayingState) {
                sendPlayDirective();
                m_audioPlayer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PLAYING, MY_WAIT_TIMEOUT));
                EXPECT_CALL(*(m_mockMediaPlayer.get()), pause(_)).Times(1);
                m_audioPlayer->onFocusChanged(FocusState::BACKGROUND, MixingBehavior::MUST_PAUSE);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PAUSED, MY_WAIT_TIMEOUT));
                EXPECT_CALL(*(m_mockMediaPlayer.get()), resume(_)).Times(1);
                m_audioPlayer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PLAYING, MY_WAIT_TIMEOUT));
                EXPECT_CALL(*(m_mockMediaPlayer.get()), stop(_)).Times(1);
                m_audioPlayer->onFocusChanged(FocusState::NONE, MixingBehavior::MUST_STOP);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::STOPPED, MY_WAIT_TIMEOUT));
            }
            TEST_F(AudioPlayerTest, test_focusChangesInStoppedState) {
                sendPlayDirective();
                EXPECT_CALL(*(m_mockMediaPlayer.get()), stop(_)).Times(AtLeast(1));
                m_audioPlayer->onFocusChanged(FocusState::NONE, MixingBehavior::MUST_STOP);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::STOPPED, MY_WAIT_TIMEOUT));
                m_audioPlayer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::STOPPED, MY_WAIT_TIMEOUT));
                m_audioPlayer->onFocusChanged(FocusState::BACKGROUND, MixingBehavior::MUST_PAUSE);
                ASSERT_FALSE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PAUSED, MY_WAIT_TIMEOUT));
            }
            TEST_F(AudioPlayerTest, test_focusChangesInPausedState) {
                sendPlayDirective();
                EXPECT_CALL(*(m_mockMediaPlayer.get()), pause(_)).Times(1);
                m_audioPlayer->onFocusChanged(FocusState::BACKGROUND, MixingBehavior::MUST_PAUSE);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PAUSED, MY_WAIT_TIMEOUT));
                EXPECT_CALL(*(m_mockMediaPlayer.get()), resume(_)).Times(1);
                m_audioPlayer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PLAYING, MY_WAIT_TIMEOUT));
                EXPECT_CALL(*(m_mockMediaPlayer.get()), pause(_)).Times(1);
                m_audioPlayer->onFocusChanged(FocusState::BACKGROUND, MixingBehavior::MUST_PAUSE);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PAUSED, MY_WAIT_TIMEOUT));
                m_audioPlayer->onFocusChanged(FocusState::BACKGROUND, MixingBehavior::MUST_PAUSE);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PAUSED, MY_WAIT_TIMEOUT));
                EXPECT_CALL(*(m_mockMediaPlayer.get()), stop(_)).Times(1);
                m_audioPlayer->onFocusChanged(FocusState::NONE, MixingBehavior::MUST_STOP);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::STOPPED, MY_WAIT_TIMEOUT));
            }
            TEST_F(AudioPlayerTest, test_focusChangesInBufferUnderrunState) {
                sendPlayDirective();
                m_audioPlayer->onBufferUnderrun(m_mockMediaPlayer->getCurrentSourceId(), DEFAULT_MEDIA_PLAYER_STATE);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::BUFFER_UNDERRUN, MY_WAIT_TIMEOUT));
                m_audioPlayer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::BUFFER_UNDERRUN, MY_WAIT_TIMEOUT));
                EXPECT_CALL(*(m_mockMediaPlayer.get()), pause(_)).Times(1);
                m_audioPlayer->onFocusChanged(FocusState::BACKGROUND, MixingBehavior::MUST_PAUSE);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PAUSED, MY_WAIT_TIMEOUT));
                EXPECT_CALL(*(m_mockMediaPlayer.get()), resume(_)).Times(1);
                m_audioPlayer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PLAYING, MY_WAIT_TIMEOUT));
                m_audioPlayer->onBufferUnderrun(m_mockMediaPlayer->getCurrentSourceId(), DEFAULT_MEDIA_PLAYER_STATE);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::BUFFER_UNDERRUN, MY_WAIT_TIMEOUT));
                EXPECT_CALL(*(m_mockMediaPlayer.get()), stop(_)).Times(1);
                m_audioPlayer->onFocusChanged(FocusState::NONE, MixingBehavior::MUST_STOP);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::STOPPED, MY_WAIT_TIMEOUT));
            }
            TEST_F(AudioPlayerTest, test_focusChangeToBackgroundBeforeOnPlaybackStarted) {
                EXPECT_CALL(*(m_mockMediaPlayer.get()), play(_)).Times(1);
                sendPlayDirective();
                EXPECT_CALL(*(m_mockMediaPlayer.get()), stop(_)).Times(AtLeast(1));
                sendClearQueueDirective();
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::STOPPED, MY_WAIT_TIMEOUT));
                m_audioPlayer->onFocusChanged(FocusState::NONE, MixingBehavior::MUST_STOP);
                //EXPECT_CALL(*(m_mockFocusManager.get()), acquireChannel(CHANNEL_NAME, _)).Times(1).WillOnce(Return(true));
                EXPECT_CALL(*(m_mockMediaPlayerTrack2.get()), play(_)).Times(1);
                auto avsMessageHeader = std::make_shared<AVSMessageHeader>(NAMESPACE_AUDIO_PLAYER, NAME_PLAY, MESSAGE_ID_TEST_2);
                shared_ptr<AVSDirective> playDirective = AVSDirective::create("", avsMessageHeader, createEnqueuePayloadTest(OFFSET_IN_MILLISECONDS_TEST, AUDIO_ITEM_ID_2),
                                                              m_attachmentManager, CONTEXT_ID_TEST_2);
                m_audioPlayer->CapabilityAgent::preHandleDirective(playDirective, std::move(m_mockDirectiveHandlerResult));
                m_audioPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST_2);
                m_audioPlayer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PLAYING, MY_WAIT_TIMEOUT));
                m_audioPlayer->onFocusChanged(FocusState::BACKGROUND, MixingBehavior::MUST_PAUSE);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PAUSED, MY_WAIT_TIMEOUT));
            }
            TEST_F(AudioPlayerTest, test_playAfterOnPlaybackError) {
                EXPECT_CALL(*(m_mockMediaPlayer.get()), getOffset(_)).WillRepeatedly(Return(m_mockMediaPlayer->getOffset(m_mockMediaPlayer->getCurrentSourceId())));
                sendPlayDirective();
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PLAYING, MY_WAIT_TIMEOUT));
                EXPECT_CALL(*(m_mockFocusManager.get()), releaseChannel(CHANNEL_NAME, _)).Times(1).WillOnce(InvokeWithoutArgs(this, &AudioPlayerTest::wakeOnReleaseChannel));
                m_audioPlayer->onPlaybackError(m_mockMediaPlayer->getCurrentSourceId(),ErrorType::MEDIA_ERROR_UNKNOWN,"TEST_ERROR", DEFAULT_MEDIA_PLAYER_STATE);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::STOPPED, MY_WAIT_TIMEOUT));
                ASSERT_EQ(std::future_status::ready, m_wakeReleaseChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                m_audioPlayer->onFocusChanged(FocusState::NONE, MixingBehavior::MUST_STOP);
                EXPECT_CALL(*(m_mockMediaPlayerTrack2.get()), play(_)).Times(1);
                auto avsMessageHeader = std::make_shared<AVSMessageHeader>(NAMESPACE_AUDIO_PLAYER, NAME_PLAY, MESSAGE_ID_TEST_2);
                shared_ptr<AVSDirective> playDirective = AVSDirective::create("", avsMessageHeader, REPLACE_ALL_PAYLOAD_TEST, m_attachmentManager, CONTEXT_ID_TEST_2);
                m_wakeAcquireChannelPromise = promise<void>();
                m_wakeAcquireChannelFuture = m_wakeAcquireChannelPromise.get_future();
                //EXPECT_CALL(*(m_mockFocusManager.get()), acquireChannel(CHANNEL_NAME, _)).Times(1).WillOnce(InvokeWithoutArgs(this, &AudioPlayerTest::wakeOnAcquireChannel));
                m_audioPlayer->CapabilityAgent::preHandleDirective(playDirective, std::move(m_mockDirectiveHandlerResult));
                m_audioPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST_2);
                ASSERT_EQ(future_status::ready, m_wakeAcquireChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                m_audioPlayer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PLAYING, MY_WAIT_TIMEOUT));
            }
            TEST_F(AudioPlayerTest, test_playCallsCaptionManager) {
                //EXPECT_CALL(*(m_mockCaptionManager.get()), onCaption(_, _)).Times(1);
                sendPlayDirective();
            }
            TEST_F(AudioPlayerTest, test_playParsesCaptionPayload) {
                CaptionData expectedCaptionData = CaptionData(captions::CaptionFormat::WEBVTT, "WEBVTT\n\n1\n00:00.000 --> 00:01.260\nThe time is 2:17 PM.");
                //EXPECT_CALL(*(m_mockCaptionManager.get()), onCaption(_, expectedCaptionData)).Times(1);
                sendPlayDirective();
            }
            TEST_F(AudioPlayerTest, test_playbackStartedSwitchesHandler) {
                EXPECT_CALL(*m_mockPlaybackRouter, useDefaultHandlerWith(_));
                sendPlayDirective();
            }
            TEST_F(AudioPlayerTest, test_progressReportDelayElapsed) {
                m_expectedMessages.insert({PROGRESS_REPORT_DELAY_ELAPSED_NAME, 0});
                /*EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_)).Times(AtLeast(1))
                    .WillRepeatedly(Invoke([this]shared_ptr<MessageRequest> request) {
                        lock_guard<mutex> lock(m_mutex);
                        verifyMessageMap(request, &m_expectedMessages);
                        m_messageSentTrigger.notify_one();
                    }));*/
                sendPlayDirective(OFFSET_IN_MILLISECONDS_BEFORE_PROGRESS_REPORT_DELAY);
                this_thread::sleep_for(milliseconds(PROGRESS_REPORT_DELAY));
                unique_lock<mutex> lock(m_mutex);
                auto result = m_messageSentTrigger.wait_for(lock, MY_WAIT_TIMEOUT, [this] {
                    for (auto messageStatus : m_expectedMessages) {
                        if (messageStatus.second != 1) return false;
                    }
                    return true;
                });
                ASSERT_TRUE(result);
            }
            TEST_F(AudioPlayerTest, test_progressReportDelayElapsedDelayLessThanOffset) {
                m_expectedMessages.insert({PROGRESS_REPORT_DELAY_ELAPSED_NAME, 0});
                /*EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_)).Times(AtLeast(1))
                    .WillRepeatedly(Invoke([this]shared_ptr<MessageRequest> request) {
                        lock_guard<mutex> lock(m_mutex);
                        verifyMessageMap(request, &m_expectedMessages);
                        m_messageSentTrigger.notify_one();
                    }));*/
                sendPlayDirective(OFFSET_IN_MILLISECONDS_AFTER_PROGRESS_REPORT_DELAY);
                this_thread::sleep_for(milliseconds(PROGRESS_REPORT_DELAY));
                unique_lock<mutex> lock(m_mutex);
                auto result = m_messageSentTrigger.wait_for(lock, MY_WAIT_TIMEOUT, [this] {
                    for (auto messageStatus : m_expectedMessages) {
                        if (messageStatus.second != 0) return false;
                    }
                    return true;
                });
                ASSERT_TRUE(result);
            }
            TEST_F(AudioPlayerTest, testTimer_progressReportIntervalElapsed) {
                m_expectedMessages.insert({PROGRESS_REPORT_INTERVAL_ELAPSED_NAME, 0});
                EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_)).Times(AtLeast(1))
                    .WillRepeatedly(Invoke([this](shared_ptr<MessageRequest> request) {
                        lock_guard<mutex> lock(m_mutex);
                        verifyMessageMap(request, &m_expectedMessages);
                        m_messageSentTrigger.notify_one();
                    }));
                sendPlayDirective(OFFSET_IN_MILLISECONDS_BEFORE_PROGRESS_REPORT_INTERVAL);
                this_thread::sleep_for(TIME_FOR_TWO_AND_A_HALF_INTERVAL_PERIODS);
                unique_lock<mutex> lock(m_mutex);
                auto result = m_messageSentTrigger.wait_for(lock, MY_WAIT_TIMEOUT, [this] {
                    for (auto messageStatus : m_expectedMessages) {
                        if (messageStatus.second != 3) return false;
                    }
                    return true;
                });
                ASSERT_TRUE(result);
            }
            TEST_F(AudioPlayerTest, test_progressReportIntervalElapsedIntervalLessThanOffset) {
                m_expectedMessages.insert({PROGRESS_REPORT_INTERVAL_ELAPSED_NAME, 0});
                EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_)).Times(AtLeast(1))
                    .WillRepeatedly(Invoke([this](shared_ptr<MessageRequest> request) {
                        lock_guard<mutex> lock(m_mutex);
                        verifyMessageMap(request, &m_expectedMessages);
                        m_messageSentTrigger.notify_one();
                    }));
                sendPlayDirective(OFFSET_IN_MILLISECONDS_AFTER_PROGRESS_REPORT_INTERVAL);
                this_thread::sleep_for(TIME_FOR_TWO_AND_A_HALF_INTERVAL_PERIODS);
                unique_lock<mutex> lock(m_mutex);
                auto result = m_messageSentTrigger.wait_for(lock, MY_WAIT_TIMEOUT, [this] {
                    for (auto messageStatus : m_expectedMessages) {
                        if (messageStatus.second != 2) return false;
                    }
                    return true;
                });
                ASSERT_TRUE(result);
            }
            TEST_F(AudioPlayerTest, testSlow_playOnlyAfterForegroundFocus) {
                EXPECT_CALL(*(m_mockMediaPlayer.get()), getOffset(_)).WillRepeatedly(Return(m_mockMediaPlayer->getOffset(m_mockMediaPlayer->getCurrentSourceId())));
                sendPlayDirective();
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PLAYING, MY_WAIT_TIMEOUT));
                m_audioPlayer->onPlaybackStarted(m_mockMediaPlayer->getCurrentSourceId(), DEFAULT_MEDIA_PLAYER_STATE);
                m_audioPlayer->onFocusChanged(FocusState::BACKGROUND, MixingBehavior::MUST_PAUSE);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PAUSED, MY_WAIT_TIMEOUT));
                //EXPECT_CALL(*(m_mockFocusManager.get()), acquireChannel(CHANNEL_NAME, _)).Times(1).WillOnce(InvokeWithoutArgs(this, &AudioPlayerTest::wakeOnAcquireChannel));
                auto avsMessageHeader = std::make_shared<AVSMessageHeader>(NAMESPACE_AUDIO_PLAYER, NAME_PLAY, MESSAGE_ID_TEST_2);
                shared_ptr<AVSDirective> playDirective = AVSDirective::create("", avsMessageHeader, REPLACE_ALL_PAYLOAD_TEST, m_attachmentManager, CONTEXT_ID_TEST_2);
                m_wakeAcquireChannelPromise = promise<void>();
                m_audioPlayer->CapabilityAgent::preHandleDirective(playDirective, move(m_mockDirectiveHandlerResult));
                EXPECT_CALL(*(m_mockMediaPlayerTrack2.get()), play(_)).Times(0);
                m_audioPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST_2);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::STOPPED, MY_WAIT_TIMEOUT));
                EXPECT_CALL(*(m_mockMediaPlayerTrack2.get()), play(_)).Times(1);
                m_audioPlayer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PLAYING, MY_WAIT_TIMEOUT));
            }
            TEST_F(AudioPlayerTest, testSlow_focusChangeRaceOnPlay) {
                EXPECT_CALL(*(m_mockMediaPlayer.get()), getOffset(_)).WillRepeatedly(Return(m_mockMediaPlayer->getOffset(m_mockMediaPlayer->getCurrentSourceId())));
                sendPlayDirective();
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PLAYING, MY_WAIT_TIMEOUT));
                m_audioPlayer->onPlaybackStarted(m_mockMediaPlayer->getCurrentSourceId(), DEFAULT_MEDIA_PLAYER_STATE);
                m_audioPlayer->onFocusChanged(FocusState::BACKGROUND, MixingBehavior::MUST_PAUSE);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PAUSED, MY_WAIT_TIMEOUT));
                m_wakeAcquireChannelPromise = std::promise<void>();
                m_wakeAcquireChannelFuture = m_wakeAcquireChannelPromise.get_future();
                //EXPECT_CALL(*(m_mockFocusManager.get()), acquireChannel(CHANNEL_NAME, _)).Times(1).WillOnce(InvokeWithoutArgs(this, &AudioPlayerTest::wakeOnAcquireChannel));
                auto avsMessageHeader = std::make_shared<AVSMessageHeader>(NAMESPACE_AUDIO_PLAYER, NAME_PLAY, MESSAGE_ID_TEST_2);
                shared_ptr<AVSDirective> playDirective = AVSDirective::create("", avsMessageHeader, REPLACE_ALL_PAYLOAD_TEST, m_attachmentManager, CONTEXT_ID_TEST_2);
                m_audioPlayer->CapabilityAgent::preHandleDirective(playDirective, std::move(m_mockDirectiveHandlerResult));
                m_audioPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST_2);
                ASSERT_EQ(future_status::ready, m_wakeAcquireChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                m_audioPlayer->onFocusChanged(FocusState::NONE, MixingBehavior::MUST_STOP);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::STOPPED, MY_WAIT_TIMEOUT));
                m_audioPlayer->onFocusChanged(FocusState::BACKGROUND, MixingBehavior::MUST_PAUSE);
                this_thread::sleep_for(milliseconds(EVENT_PROCESS_DELAY));
                EXPECT_CALL(*(m_mockMediaPlayerTrack2.get()), play(_)).Times(1);
                m_audioPlayer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PLAYING, MY_WAIT_TIMEOUT));
            }
            TEST_F(AudioPlayerTest, testTimer_playbackStartedCallbackAfterFocusLost) {
                EXPECT_CALL(*(m_mockMediaPlayer.get()), getOffset(_)).WillRepeatedly(Return(m_mockMediaPlayer->getOffset(m_mockMediaPlayer->getCurrentSourceId())));
                auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_AUDIO_PLAYER, NAME_PLAY, MESSAGE_ID_TEST, PLAY_REQUEST_ID_TEST);
                shared_ptr<AVSDirective> playDirective = AVSDirective::create("", avsMessageHeader, createEnqueuePayloadTest(OFFSET_IN_MILLISECONDS_TEST),
                                                              m_attachmentManager, CONTEXT_ID_TEST);
                //EXPECT_CALL(*(m_mockFocusManager.get()), acquireChannel(CHANNEL_NAME, _)).Times(1).WillOnce(InvokeWithoutArgs(this, &AudioPlayerTest::wakeOnAcquireChannel));
                EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted());
                m_audioPlayer->CapabilityAgent::preHandleDirective(playDirective, std::move(m_mockDirectiveHandlerResult));
                m_audioPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                {
                    InSequence dummy;
                    promise<void> playCalledPromise;
                    future<void> playCalled = playCalledPromise.get_future();
                    ON_CALL(*m_mockMediaPlayer, play(_)).WillByDefault(InvokeWithoutArgs([&playCalledPromise] {
                        playCalledPromise.set_value();
                        return true;
                    }));
                    EXPECT_CALL(*m_mockMediaPlayer, stop(_)).Times(1);
                    ASSERT_EQ(future_status::ready, m_wakeAcquireChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                    m_audioPlayer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                    ASSERT_THAT(playCalled.wait_for(MY_WAIT_TIMEOUT), Ne(std::future_status::timeout));
                    m_audioPlayer->onFocusChanged(FocusState::NONE, MixingBehavior::MUST_STOP);
                    m_audioPlayer->onPlaybackStarted(m_mockMediaPlayer->getSourceId(), DEFAULT_MEDIA_PLAYER_STATE);
                    ASSERT_FALSE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PLAYING, MY_WAIT_TIMEOUT));
                }
            }
            void AudioPlayerTest::testPlayEnqueueFinishPlay() {
                sendPlayDirective();
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PLAYING, MY_WAIT_TIMEOUT));
                this_thread::sleep_for(milliseconds(EVENT_PROCESS_DELAY));
                for (int i = 0; i < 3; i++) {
                    string msgId = MESSAGE_ID_TEST + to_string(i);
                    auto avsMessageHeader = std::make_shared<AVSMessageHeader>(NAMESPACE_AUDIO_PLAYER, NAME_PLAY, msgId);
                    shared_ptr<AVSDirective> playDirective = AVSDirective::create("", avsMessageHeader, createEnqueuePayloadTest(OFFSET_IN_MILLISECONDS_TEST, AUDIO_ITEM_ID_1 + to_string(i)),
                                                                  m_attachmentManager, CONTEXT_ID_TEST + to_string(i));
                    m_audioPlayer->CapabilityAgent::preHandleDirective(playDirective, move(m_mockDirectiveHandlerResult));
                    m_audioPlayer->CapabilityAgent::handleDirective(msgId);
                }
                m_audioPlayer->onPlaybackFinished(m_mockMediaPlayer->getCurrentSourceId(), DEFAULT_MEDIA_PLAYER_STATE);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::FINISHED, MY_WAIT_TIMEOUT));
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PLAYING, MY_WAIT_TIMEOUT));
                m_audioPlayer->onPlaybackFinished(m_mockMediaPlayer->getCurrentSourceId(), DEFAULT_MEDIA_PLAYER_STATE);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::FINISHED, MY_WAIT_TIMEOUT));
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PLAYING, MY_WAIT_TIMEOUT));
                m_audioPlayer->onPlaybackFinished(m_mockMediaPlayer->getCurrentSourceId(), DEFAULT_MEDIA_PLAYER_STATE);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::FINISHED, MY_WAIT_TIMEOUT));
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PLAYING, MY_WAIT_TIMEOUT));
                m_audioPlayer->onPlaybackFinished(m_mockMediaPlayer->getCurrentSourceId(), DEFAULT_MEDIA_PLAYER_STATE);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::FINISHED, MY_WAIT_TIMEOUT));
            }
            TEST_F(AudioPlayerTest, test1PlayerPool_PlayEnqueueFinishPlay) {
                reSetUp(1);
                testPlayEnqueueFinishPlay();
            }
            TEST_F(AudioPlayerTest, test2PlayerPool_PlayEnqueueFinishPlay) {
                reSetUp(2);
                testPlayEnqueueFinishPlay();
            }
            TEST_F(AudioPlayerTest, test3PlayerPool_PlayEnqueueFinishPlay) {
                reSetUp(3);
                testPlayEnqueueFinishPlay();
            }
            TEST_F(AudioPlayerTest, testPlayRequestor) {
                auto avsMessageHeader = std::make_shared<AVSMessageHeader>(NAMESPACE_AUDIO_PLAYER, NAME_PLAY, MESSAGE_ID_TEST, PLAY_REQUEST_ID_TEST);
                shared_ptr<AVSDirective> playDirective = AVSDirective::create("", avsMessageHeader, PLAY_REQUESTOR_PAYLOAD_TEST, m_attachmentManager, CONTEXT_ID_TEST);
                //EXPECT_CALL(*(m_mockFocusManager.get()), acquireChannel(CHANNEL_NAME, _)).Times(1).WillOnce(InvokeWithoutArgs(this, &AudioPlayerTest::wakeOnAcquireChannel));
                EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted());
                m_audioPlayer->CapabilityAgent::preHandleDirective(playDirective, move(m_mockDirectiveHandlerResult));
                m_audioPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                ASSERT_EQ(std::future_status::ready, m_wakeAcquireChannelFuture.wait_for(MY_WAIT_TIMEOUT));
                m_audioPlayer->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PLAYING, MY_WAIT_TIMEOUT));
                auto playRequestor = m_testAudioPlayerObserver->getPlayRequestorObject();
                EXPECT_EQ(playRequestor.type, PLAY_REQUESTOR_TYPE_ALERT);
                EXPECT_EQ(playRequestor.id, PLAY_REQUESTOR_ID);
            }
            TEST_F(AudioPlayerTest, testUpdateProgressReportInterval) {
                sendPlayDirective();
                m_expectedMessages.insert({PROGRESS_REPORT_INTERVAL_UPDATED_NAME, 0});
                EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_)).Times(AtLeast(1))
                    .WillRepeatedly(Invoke([this](std::shared_ptr<avsCommon::avs::MessageRequest> request) {
                        lock_guard<mutex> lock(m_mutex);
                        verifyMessageMap(request, &m_expectedMessages);
                        m_messageSentTrigger.notify_one();
                    }));
                sendUpdateProgressReportIntervalDirective();
                unique_lock<std::mutex> lock(m_mutex);
                bool result;
                result = m_messageSentTrigger.wait_for(lock, MY_WAIT_TIMEOUT, [this] {
                    for (auto messageStatus : m_expectedMessages) {
                        if (messageStatus.second == 0) return false;
                    }
                    return true;
                });
                ASSERT_TRUE(result);
            }
            void AudioPlayerTest::verifyMessageOrder2Phase(const vector<string>& orderedMessageList, size_t index, function<void()> trigger1, function<void()> trigger2) {
                size_t nextIndex = 0;
                EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_)).Times(AtLeast(1))
                    .WillRepeatedly(Invoke([this, orderedMessageList, &nextIndex](shared_ptr<MessageRequest> request) {
                        lock_guard<mutex> lock(m_mutex);
                        if (nextIndex < orderedMessageList.size()) {
                            if (verifyMessage(request, orderedMessageList.at(nextIndex))) {
                                if (nextIndex < orderedMessageList.size()) nextIndex++;
                            }
                        }
                        m_messageSentTrigger.notify_one();
                    }));
                trigger1();
                {
                    bool phase2 = false;
                    bool result;
                    unique_lock<mutex> lock(m_mutex);
                    result = m_messageSentTrigger.wait_for(lock, MY_WAIT_TIMEOUT, [orderedMessageList, &nextIndex, &index, &phase2, &trigger2] {
                            if (nextIndex == index && !phase2) {
                                phase2 = true;
                                trigger2();
                            } else if (nextIndex == orderedMessageList.size()) return true;
                            return false;
                        });
                    ASSERT_TRUE(result);
                }
            }
            TEST_F(AudioPlayerTest, testTimer_playbackFinishedMessageOrder_1Player) {
                reSetUp(1);
                vector<string> expectedMessages;
                expectedMessages.push_back(PLAYBACK_STARTED_NAME);
                expectedMessages.push_back(PROGRESS_REPORT_DELAY_ELAPSED_NAME);
                expectedMessages.push_back(PLAYBACK_NEARLY_FINISHED_NAME);
                expectedMessages.push_back(PLAYBACK_FINISHED_NAME);
                verifyMessageOrder2Phase(expectedMessages, 2, [this] { sendPlayDirective(); }, [this] {
                                             m_audioPlayer->onPlaybackFinished(m_mockMediaPlayer->getCurrentSourceId(), DEFAULT_MEDIA_PLAYER_STATE);
                                         });
            }
            TEST_F(AudioPlayerTest, testTimer_playbackFinishedMessageOrder_2Players) {
                reSetUp(2);
                vector<string> expectedMessages;
                expectedMessages.push_back(PLAYBACK_STARTED_NAME);
                expectedMessages.push_back(PLAYBACK_NEARLY_FINISHED_NAME);
                expectedMessages.push_back(PROGRESS_REPORT_DELAY_ELAPSED_NAME);
                expectedMessages.push_back(PLAYBACK_FINISHED_NAME);
                verifyMessageOrder2Phase(expectedMessages, 3, [this] { sendPlayDirective(); }, [this] {
                                             m_audioPlayer->onPlaybackFinished(m_mockMediaPlayer->getCurrentSourceId(), DEFAULT_MEDIA_PLAYER_STATE);
                                         });
            }
            TEST_F(AudioPlayerTest, testTimer_playbackStoppedMessageOrder_1Player) {
                reSetUp(1);
                vector<string> expectedMessages;
                expectedMessages.push_back(PLAYBACK_STARTED_NAME);
                expectedMessages.push_back(PROGRESS_REPORT_DELAY_ELAPSED_NAME);
                expectedMessages.push_back(PLAYBACK_STOPPED_NAME);
                verifyMessageOrder2Phase(expectedMessages, 2, [this] { sendPlayDirective(); }, [this] {
                                             m_audioPlayer->onPlaybackStopped(m_mockMediaPlayer->getCurrentSourceId(), DEFAULT_MEDIA_PLAYER_STATE);
                                         });
            }
            TEST_F(AudioPlayerTest, testTimer_playbackStoppedMessageOrder_2Players) {
                reSetUp(2);
                vector<string> expectedMessages;
                expectedMessages.push_back(PLAYBACK_STARTED_NAME);
                expectedMessages.push_back(PLAYBACK_NEARLY_FINISHED_NAME);
                expectedMessages.push_back(PROGRESS_REPORT_DELAY_ELAPSED_NAME);
                expectedMessages.push_back(PLAYBACK_STOPPED_NAME);
                verifyMessageOrder2Phase(expectedMessages, 3, [this] { sendPlayDirective(); }, [this] {
                                             m_audioPlayer->onPlaybackStopped(m_mockMediaPlayer->getCurrentSourceId(), DEFAULT_MEDIA_PLAYER_STATE);
                                         });
            }
            TEST_F(AudioPlayerTest, test_publishedCapabiltiesContainsFingerprint) {
                unordered_set<shared_ptr<CapabilityConfiguration>> caps = m_audioPlayer->getCapabilityConfigurations();
                auto cap = *caps.begin();
                auto configuration = cap->additionalConfigurations.find(CAPABILITY_INTERFACE_CONFIGURATIONS_KEY);
                ASSERT_NE(configuration, cap->additionalConfigurations.end());
                JsonGenerator expectedConfigurations;
                expectedConfigurations.startObject(FINGERPRINT_KEY);
                expectedConfigurations.addMember(FINGERPRINT_PACKAGE_KEY, FINGERPRINT.package);
                expectedConfigurations.addMember(FINGERPRINT_BUILD_TYPE_KEY, FINGERPRINT.buildType);
                expectedConfigurations.addMember(FINGERPRINT_VERSION_NUMBER_KEY, FINGERPRINT.versionNumber);
                ASSERT_EQ(expectedConfigurations.toString(), configuration->second);
            }
            TEST_F(AudioPlayerTest, test_localStop) {
                sendPlayDirective();
                EXPECT_CALL(*(m_mockMediaPlayer.get()), stop(_)).Times(AtLeast(1));
                ASSERT_TRUE(m_audioPlayer->localOperation(LocalPlaybackHandlerInterface::PlaybackOperation::STOP_PLAYBACK));
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::STOPPED, MY_WAIT_TIMEOUT));
            }
            TEST_F(AudioPlayerTest, test_localPause) {
                sendPlayDirective();
                //EXPECT_CALL(*(m_mockMediaPlayer.get()), stop(_, _)).Times(AtLeast(1));
                ASSERT_TRUE(m_audioPlayer->localOperation(LocalPlaybackHandlerInterface::PlaybackOperation::PAUSE_PLAYBACK));
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::STOPPED, MY_WAIT_TIMEOUT));
            }
            TEST_F(AudioPlayerTest, test_localResumeAfterPaused) {
                sendPlayDirective();
                ASSERT_TRUE(m_audioPlayer->localOperation(LocalPlaybackHandlerInterface::PlaybackOperation::PAUSE_PLAYBACK));
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::STOPPED, MY_WAIT_TIMEOUT));
                EXPECT_CALL(*(m_mockMediaPlayer.get()), play(_)).Times(AtLeast(1));
                m_mockMediaPlayer->resetWaitTimer();
                ASSERT_TRUE(m_audioPlayer->localOperation(LocalPlaybackHandlerInterface::PlaybackOperation::RESUME_PLAYBACK));
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PLAYING, MY_WAIT_TIMEOUT));
            }
            TEST_F(AudioPlayerTest, test_localSeekTo) {
                sendPlayDirective();
                EXPECT_CALL(*(m_mockMediaPlayer.get()), seekTo(_, _, _)).Times(AtLeast(1));
                milliseconds position = milliseconds::zero();
                EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_)).Times(AtLeast(1))
                    .WillRepeatedly(Invoke([this, &position](std::shared_ptr<avsCommon::avs::MessageRequest> request) {
                        lock_guard<mutex> lock(m_mutex);
                        MediaPlayerState state;
                        if (extractMediaPlayerState(request, PLAYBACK_STARTED_NAME, &state)) {
                            position = state.offset;
                        }
                        m_messageSentTrigger.notify_one();
                    }));
                auto pos = milliseconds(5000);
                m_audioPlayer->localSeekTo(pos, true);
                {
                    bool result;
                    unique_lock<mutex> lock(m_mutex);
                    result = m_messageSentTrigger.wait_for(lock, MY_WAIT_TIMEOUT, [&position, &pos] {
                        if (position == pos) return true;
                        return false;
                    });
                    ASSERT_TRUE(result);
                }
            }
            TEST_F(AudioPlayerTest, test_localSeekToWhileLocalStopped) {
                sendPlayDirective();
                EXPECT_CALL(*(m_mockMediaPlayer.get()), seekTo(_, _, _)).Times(AtLeast(1));
                milliseconds position = milliseconds::zero();
                EXPECT_CALL(*(m_mockMessageSender.get()), sendMessage(_)).Times(AtLeast(1))
                    .WillRepeatedly(Invoke([this, &position](std::shared_ptr<avsCommon::avs::MessageRequest> request) {
                        lock_guard<std::mutex> lock(m_mutex);
                        MediaPlayerState state;
                        if (extractMediaPlayerState(request, PLAYBACK_STOPPED_NAME, &state)) position = state.offset;
                        m_messageSentTrigger.notify_one();
                    }));
                ASSERT_TRUE(m_audioPlayer->localOperation(LocalPlaybackHandlerInterface::PlaybackOperation::PAUSE_PLAYBACK));
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::STOPPED, MY_WAIT_TIMEOUT));
                auto pos = milliseconds(5000);
                m_audioPlayer->localSeekTo(pos, true);
                {
                    bool result;
                    unique_lock<mutex> lock(m_mutex);
                    result = m_messageSentTrigger.wait_for(lock, MY_WAIT_TIMEOUT, [&position, &pos] {
                        if (position == pos) return true;
                        return false;
                    });
                    ASSERT_TRUE(result);
                }
                EXPECT_CALL(*(m_mockMediaPlayer.get()), play(_)).Times(AtLeast(1));
                m_mockMediaPlayer->resetWaitTimer();
                ASSERT_TRUE(m_audioPlayer->localOperation(LocalPlaybackHandlerInterface::PlaybackOperation::RESUME_PLAYBACK));
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PLAYING, MY_WAIT_TIMEOUT));
            }
            TEST_F(AudioPlayerTest, DISABLED_test_endOffset) {
                auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_AUDIO_PLAYER, NAME_PLAY, MESSAGE_ID_TEST_2);
                string endOffsetPayload = createPayloadWithEndOffset(100L, 1500L);
                shared_ptr<AVSDirective> playDirective = AVSDirective::create("", avsMessageHeader, endOffsetPayload, m_attachmentManager, CONTEXT_ID_TEST_2);
                m_audioPlayer->CapabilityAgent::preHandleDirective(playDirective, move(m_mockDirectiveHandlerResult));
                m_mockMediaPlayer->waitUntilNextSetSource(MY_WAIT_TIMEOUT);
                m_audioPlayer->onBufferingComplete(m_mockMediaPlayer->getLatestSourceId(), DEFAULT_MEDIA_PLAYER_STATE);
                m_audioPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST_2);
                m_audioPlayer->onFocusChanged(FocusState::FOREGROUND, avs::MixingBehavior::PRIMARY);
                ASSERT_TRUE(m_testAudioPlayerObserver->waitFor(PlayerActivity::PLAYING, MY_WAIT_TIMEOUT));
            }
            TEST_F(AudioPlayerTest, test_badEndOffset) {
                badEndOffsetDirective(100L, 50L);
            }
            TEST_F(AudioPlayerTest, test_badEndOffsetEqualValue) {
                badEndOffsetDirective(100L, 100L);
            }
        }
    }
}