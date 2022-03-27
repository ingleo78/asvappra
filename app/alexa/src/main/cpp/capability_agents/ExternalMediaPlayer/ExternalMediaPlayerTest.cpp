#include <chrono>
#include <future>
#include <map>
#include <memory>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <json/document.h>
#include <json/en.h>
#include <json/stringbuffer.h>
#include <json/writer.h>
#include <avs/attachment/AttachmentManager.h>
#include <sdkinterfaces/MockExceptionEncounteredSender.h>
#include <sdkinterfaces/MockChannelVolumeInterface.h>
#include <sdkinterfaces/MockContextManager.h>
#include <sdkinterfaces/MockDirectiveHandlerResult.h>
#include <sdkinterfaces/MockDirectiveSequencer.h>
#include <sdkinterfaces/MockExceptionEncounteredSender.h>
#include <sdkinterfaces/MockFocusManager.h>
#include <sdkinterfaces/MockMessageSender.h>
#include <sdkinterfaces/MockPlaybackRouter.h>
#include <sdkinterfaces/MockRenderPlayerInfoCardsObserverInterface.h>
#include <sdkinterfaces/MockSpeakerManager.h>
#include <json/JSONUtils.h>
#include <logger/ConsoleLogger.h>
#include <media_player/MockMediaPlayer.h>
#include <memory/Memory.h>
#include <metrics/MockMetricRecorder.h>
#include <certified_sender/MockCertifiedSender.h>
#include "ExternalMediaPlayer.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace externalMediaPlayer {
            namespace test {
                using namespace mediaPlayer;
                using namespace memory;
                using namespace testing;
                using namespace json::jsonUtils;
                using namespace avs::attachment;
                using namespace sdkInterfaces::test;
                using namespace mediaPlayer::test;
                using namespace metrics::test;
                using namespace certifiedSender::test;
                using namespace capabilityAgents::externalMediaPlayer;
                static const unsigned int PROVIDE_STATE_TOKEN_TEST{1};
                static milliseconds MY_WAIT_TIMEOUT(1000);
                static const string EXTERNALMEDIAPLAYER_STATE_NAMESPACE = "ExternalMediaPlayer";
                static const string PLAYBACKSTATEREPORTER_STATE_NAMESPACE = "Alexa.PlaybackStateReporter";
                static const string EXTERNALMEDIAPLAYER_NAME = "ExternalMediaPlayerState";
                static const string PLAYBACKSTATEREPORTER_NAME = "playbackState";
                static const string EXTERNALMEDIAPLAYER_NAMESPACE = "ExternalMediaPlayer";
                static const string PLAYBACKCONTROLLER_NAMESPACE = "Alexa.PlaybackController";
                static const string PLAYLISTCONTROLLER_NAMESPACE = "Alexa.PlaylistController";
                static const string SEEKCONTROLLER_NAMESPACE = "Alexa.SeekController";
                static const string FAVORITESCONTROLLER_NAMESPACE = "Alexa.FavoritesController";
                static const string PLAYER_USER_NAME = "userName";
                static const string PLAYER_ID = "testPlayerId";
                static const string PLAYER_TRACK = "testTrack";
                static const string PLAYER_STATE = "IDLE";
                static const NamespaceAndName PLAY_DIRECTIVE{EXTERNALMEDIAPLAYER_NAMESPACE, "Play"};
                static const NamespaceAndName LOGIN_DIRECTIVE{EXTERNALMEDIAPLAYER_NAMESPACE, "Login"};
                static const NamespaceAndName LOGOUT_DIRECTIVE{EXTERNALMEDIAPLAYER_NAMESPACE, "Logout"};
                static const NamespaceAndName AUTHORIZEDISCOVEREDPLAYERS_DIRECTIVE{EXTERNALMEDIAPLAYER_NAMESPACE,"AuthorizeDiscoveredPlayers"};
                static const NamespaceAndName RESUME_DIRECTIVE{PLAYBACKCONTROLLER_NAMESPACE, "Play"};
                static const NamespaceAndName PAUSE_DIRECTIVE{PLAYBACKCONTROLLER_NAMESPACE, "Pause"};
                static const NamespaceAndName STOP_DIRECTIVE{PLAYBACKCONTROLLER_NAMESPACE, "Stop"};
                static const NamespaceAndName NEXT_DIRECTIVE{PLAYBACKCONTROLLER_NAMESPACE, "Next"};
                static const NamespaceAndName PREVIOUS_DIRECTIVE{PLAYBACKCONTROLLER_NAMESPACE, "Previous"};
                static const NamespaceAndName STARTOVER_DIRECTIVE{PLAYBACKCONTROLLER_NAMESPACE, "StartOver"};
                static const NamespaceAndName REWIND_DIRECTIVE{PLAYBACKCONTROLLER_NAMESPACE, "Rewind"};
                static const NamespaceAndName FASTFORWARD_DIRECTIVE{PLAYBACKCONTROLLER_NAMESPACE, "FastForward"};
                static const NamespaceAndName ENABLEREPEATONE_DIRECTIVE{PLAYLISTCONTROLLER_NAMESPACE, "EnableRepeatOne"};
                static const NamespaceAndName ENABLEREPEAT_DIRECTIVE{PLAYLISTCONTROLLER_NAMESPACE, "EnableRepeat"};
                static const NamespaceAndName DISABLEREPEAT_DIRECTIVE{PLAYLISTCONTROLLER_NAMESPACE, "DisableRepeat"};
                static const NamespaceAndName ENABLESHUFFLE_DIRECTIVE{PLAYLISTCONTROLLER_NAMESPACE, "EnableShuffle"};
                static const NamespaceAndName DISABLESHUFFLE_DIRECTIVE{PLAYLISTCONTROLLER_NAMESPACE, "DisableShuffle"};
                static const NamespaceAndName SEEK_DIRECTIVE{SEEKCONTROLLER_NAMESPACE, "SetSeekPosition"};
                static const NamespaceAndName ADJUSTSEEK_DIRECTIVE{SEEKCONTROLLER_NAMESPACE, "AdjustSeekPosition"};
                static const NamespaceAndName FAVORITE_DIRECTIVE{FAVORITESCONTROLLER_NAMESPACE, "Favorite"};
                static const NamespaceAndName UNFAVORITE_DIRECTIVE{FAVORITESCONTROLLER_NAMESPACE, "Unfavorite"};
                static const NamespaceAndName SESSION_STATE{EXTERNALMEDIAPLAYER_STATE_NAMESPACE, EXTERNALMEDIAPLAYER_NAME};
                static const NamespaceAndName PLAYBACK_STATE{PLAYBACKSTATEREPORTER_STATE_NAMESPACE, PLAYBACKSTATEREPORTER_NAME};
                static const NamespaceAndName REPORT_DISCOVERED_PLAYERS{EXTERNALMEDIAPLAYER_NAMESPACE, "ReportDiscoveredPlayers"};
                static const NamespaceAndName AUTHORIZATION_COMPLETE{EXTERNALMEDIAPLAYER_NAMESPACE, "AuthorizationComplete"};
                static const PlayRequestor testPlayRequestor{.type = "ALERT", .id = "123"};
                static const string IDLE_PLAYBACK_STATE = R"({"state":"IDLE","supportedOperations":[],"shuffle":"NOT_SHUFFLED",
                                                          "repeat":"NOT_REPEATED","favorite":"NOT_RATED","positionMilliseconds":0,"players":[{
                                                          "playerId":"","state":"IDLE","supportedOperations":[],"positionMilliseconds":0,
                                                          "shuffle":"NOT_SHUFFLED","repeat":"NOT_REPEATED","favorite":"NOT_RATED","media":{
                                                          "type":"","value":{"playbackSource":"","playbackSourceId":"","trackName":"",
                                                          "trackId":"","trackNumber":"","artist":"","artistId":"","album":"","albumId":"",
                                                          "coverUrls":{"tiny":"","small":"","medium":"","large":""},"coverId":"",
                                                          "mediaProvider":"","mediaType":"TRACK","durationInMilliseconds":0}}}]})";
                static AdapterState createAdapterState() {
                    AdapterSessionState sessionState;
                    sessionState.loggedIn = false;
                    sessionState.userName = PLAYER_USER_NAME;
                    sessionState.playerId = PLAYER_ID;
                    AdapterPlaybackState playbackState;
                    playbackState.state = PLAYER_STATE;
                    playbackState.trackName = PLAYER_TRACK;
                    playbackState.playRequestor = testPlayRequestor;
                    AdapterState adapterState;
                    adapterState.sessionState = sessionState;
                    adapterState.playbackState = playbackState;
                    return adapterState;
                }
                static const string MESSAGE_ID_TEST = "MessageId_Test";
                static const string MESSAGE_ID_TEST2 = "MessageId_Test2";
                static const string DIALOG_REQUEST_ID_TEST = "DialogId_Test";
                static const string TAG ="ExternalMediaPlayerTest";
                static const string MSP1_PLAYER_ID = "MSP1_PLAYERID";
                static const string MSP1_SKILLTOKEN = "MSP1_SKILLTOKEN";
                static const string MSP1_LOCAL_PLAYER_ID = "MSP1_LOCAL_PLAYER_ID";
                static const string MSP2_LOCAL_PLAYER_ID = "MSP2_LOCAL_PLAYER_ID";
                static const string MSP2_PLAYER_ID = "MSP2_PLAYERID";
                static const string MSP2_SKILLTOKEN = "MSP2_SKILLTOKEN";
                #define LX(event) LogEntry(TAG, event)
                class MockExternalMediaPlayerAdapter : public ExternalMediaAdapterInterface {
                public:
                    static shared_ptr<ExternalMediaAdapterInterface> getInstance(shared_ptr<MetricRecorderInterface> metricRecorder,
                                                                                 shared_ptr<MediaPlayerInterface> mediaPlayer,
                                                                                 shared_ptr<ChannelVolumeInterface> speaker,
                                                                                 shared_ptr<SpeakerManagerInterface> speakerManager,
                                                                                 shared_ptr<MessageSenderInterface> messageSender,
                                                                                 shared_ptr<FocusManagerInterface> focusManager,
                                                                                 shared_ptr<ContextManagerInterface> contextManager,
                                                                                 shared_ptr<ExternalMediaPlayerInterface> externalMediaPlayer);
                    static std::shared_ptr<MockExternalMediaPlayerAdapter> m_currentActiveMediaPlayerAdapter;
                    MOCK_METHOD0(doShutdown, void());
                    MOCK_METHOD0(init, void());
                    MOCK_METHOD0(deInit, void());
                    MOCK_METHOD4(handleLogin, void(const string& accessToken, const string& userName,bool forceLogin, milliseconds tokenRefreshInterval));
                    MOCK_METHOD0(handleLogout, void());
                    MOCK_METHOD8(handlePlay, void(string& playContextToken, int64_t index, milliseconds offset, const string& skillToken,
                                 const string& playbackSessionId, const string& navigation, bool preload, const PlayRequestor& playRequestor));
                    MOCK_METHOD1(handlePlayControl, void(RequestType requestType));
                    MOCK_METHOD1(handleSeek, void(milliseconds offset));
                    MOCK_METHOD1(handleAdjustSeek, void(milliseconds deltaOffset));
                    MOCK_METHOD3(handleAuthorized, void(bool authorized, const string& playerId, const string& defaultSkillToken));
                    MOCK_METHOD1(handleSetVolume, void(int8_t volume));
                    MOCK_METHOD1(handleSetMute, void(bool));
                    MOCK_METHOD0(getState, AdapterState());
                    MOCK_METHOD0(getOffset, milliseconds());
                private:
                    MockExternalMediaPlayerAdapter();
                };
                shared_ptr<MockExternalMediaPlayerAdapter> MockExternalMediaPlayerAdapter::m_currentActiveMediaPlayerAdapter;
                MockExternalMediaPlayerAdapter::MockExternalMediaPlayerAdapter() : RequiresShutdown("MockExternalMediaPlayerAdapter"),
                                                                                   ExternalMediaAdapterInterface("MockExternalMediaPlayerAdapter") {}
                shared_ptr<ExternalMediaAdapterInterface> MockExternalMediaPlayerAdapter::getInstance(shared_ptr<MetricRecorderInterface> metricRecorder,
                                                                                                      shared_ptr<MediaPlayerInterface> mediaPlayer,
                                                                                                      shared_ptr<ChannelVolumeInterface> speaker,
                                                                                                      shared_ptr<SpeakerManagerInterface> speakerManager,
                                                                                                      shared_ptr<MessageSenderInterface> messageSender,
                                                                                                      shared_ptr<FocusManagerInterface> focusManager,
                                                                                                      shared_ptr<ContextManagerInterface> contextManager,
                                                                                                      shared_ptr<ExternalMediaPlayerInterface> externalMediaPlayer) {
                    MockExternalMediaPlayerAdapter::m_currentActiveMediaPlayerAdapter = shared_ptr<MockExternalMediaPlayerAdapter>(new MockExternalMediaPlayerAdapter());
                    return MockExternalMediaPlayerAdapter::m_currentActiveMediaPlayerAdapter;
                }
                class MockExternalMediaPlayerObserver : public ExternalMediaPlayerObserverInterface {
                public:
                    static shared_ptr<MockExternalMediaPlayerObserver> getInstance();
                    //MOCK_METHOD2(onLoginStateProvided, void(const string&, const ObservableSessionProperties));
                    //MOCK_METHOD2(onPlaybackStateProvided, void(const string&, const ObservablePlaybackStateProperties));
                private:
                    MockExternalMediaPlayerObserver();
                };
                shared_ptr<MockExternalMediaPlayerObserver> MockExternalMediaPlayerObserver::getInstance() {
                    return shared_ptr<MockExternalMediaPlayerObserver>(new MockExternalMediaPlayerObserver());
                }
                MockExternalMediaPlayerObserver::MockExternalMediaPlayerObserver() {}
                static string createAuthorizeDiscoveredPlayersPayload(unordered_set<string> players = unordered_set<string>()) {
                    string payload = R"({"players" : [)";
                    for (auto it = players.begin(); it != players.end(); it++) {
                        if (it != players.begin()) payload += ",";
                        payload += *it;
                    }
                    payload += R"(]})";
                    return payload;
                }
                static string createPlayerJson(const string& localPlayerId, bool authorized, const string& playerId, const string& skillToken) {
                    return R"({"localPlayerId" : ")" + localPlayerId + R"(","authorized" : )" + (authorized ? "true" : "false") + R"(,
                           "metadata" : {"playerId" : ")" + playerId + R"(","skillToken" : ")" + skillToken + R"("}})";
                }
                static string getIdleSessionStateJson(string agent) {
                    string idle_session_state = R"({"agent":")" + string(agent) + R"(","spiVersion":")" + string(ExternalMediaPlayer::SPI_VERSION) + R"(",
                                                "playerInFocus":"","players":[{"playerId":"","endpointId":"","loggedIn":false,"username":"",
                                                "isGuest":false,"launched":false,"active":false,"spiVersion":"","playerCookie":"",
                                                "skillToken":"","playbackSessionId":""}]})";
                    return idle_session_state;
                }
                static string createPlayPayloadWithParseError(const string& playContext, int index, int64_t offsetInMilliseconds,
                                                              const string& playerId, const string& skillToken, const string& playbackSessionId,
                                                              const string& navigation, bool preload) {
                    const string PLAY_PAYLOAD_TEST = "{\"playbackContextToken\":\"" + playContext + "\",\"offsetInMilliseconds\":" +
                                                     to_string(offsetInMilliseconds) + "\",\"playerId\":\"" + playerId + "\","
                                                     "\"index\":\"" + to_string(index) + "\",\"skillToken\":\"" + skillToken + "\","
                                                     "\"playbackSessionId\":\"" + playbackSessionId + "\",\"navigation\":\"" + navigation +
                                                     "\",\"preload\":" + (preload ? "true" : "false") + "}";
                    return PLAY_PAYLOAD_TEST;
                }
                static string createPayloadWithPlayerId(const string& playerId) {
                    const string PLAYERID_PAYLOAD_TEST = "{\"playerId\":\"" + playerId + "\"}";
                    return PLAYERID_PAYLOAD_TEST;
                }
                static string createPlayPayload(const string& playContext, int index, int64_t offsetInMilliseconds, const string& playerId,
                                                const string& skillToken, const string& playbackSessionId, const string& navigation,
                                                bool preload) {
                    const string PLAY_PAYLOAD_TEST = "{\"playbackContextToken\":\"" + playContext + "\",\"offsetInMilliseconds\":\"" +
                                                     to_string(offsetInMilliseconds) + "\",\"playerId\":\"" + playerId + "\","
                                                     "\"index\":\"" + to_string(index) + "\",\"skillToken\":\"" + skillToken + "\","
                                                     "\"playbackSessionId\":\"" + playbackSessionId + "\",\"navigation\":\"" + navigation +
                                                     "\",\"preload\":" + (preload ? "true" : "false") + "}";
                    return PLAY_PAYLOAD_TEST;
                }
                static string createPlayPayloadWithPlayRequestor(const string& playContext, int index, int64_t offsetInMilliseconds,
                                                                 const string& playerId, const string& skillToken, const string& playbackSessionId,
                                                                 const string& navigation, bool preload, const PlayRequestor& playRequestor) {
                    const string PLAY_PAYLOAD_TEST = "{\"playbackContextToken\":\"" + playContext + "\",\"offsetInMilliseconds\":\"" +
                                                     to_string(offsetInMilliseconds) + "\",\"playerId\":\"" + playerId + "\",\"index\":\"" +
                                                     to_string(index) + "\",\"skillToken\":\"" + skillToken + "\",\"playbackSessionId\":\"" +
                                                     playbackSessionId + "\",\"navigation\":\"" + navigation + "\",\"preload\":" +
                                                     (preload ? "true" : "false") + ",\"playRequestor\":{\"type\":\"" + playRequestor.type +
                                                     "\",\"id\":\"" + playRequestor.id + "\"}}";
                    return PLAY_PAYLOAD_TEST;
                }
                static string createPlayPayloadNoContext(int index, int64_t offsetInMilliseconds, const string& playerId, const string& skillToken,
                                                         const string& playbackSessionId, const string& navigation, bool preload) {
                    const string PLAY_PAYLOAD_TEST = "{\"offsetInMilliseconds\":\"" + to_string(offsetInMilliseconds) + "\",\"playerId\":\"" +
                                                     playerId + "\",\"index\":\"" + to_string(index) + "\",\"skillToken\":\"" + skillToken +
                                                     "\",\"playbackSessionId\":\"" + playbackSessionId + "\",\"navigation\":\"" + navigation +
                                                     "\",\"preload\":" + (preload ? "true" : "false") + "}";
                    return PLAY_PAYLOAD_TEST;
                }
                static string createPlayPayloadNoPlayerId(const string& playContext, int index, int64_t offsetInMilliseconds,
                                                          const string& skillToken, const string& playbackSessionId, const string& navigation,
                                                          bool preload) {
                    const string PLAY_PAYLOAD_TEST = "{\"playbackContextToken\":\"" + playContext + "\",\"offsetInMilliseconds\":\"" +
                                                     to_string(offsetInMilliseconds) + "\",\"index\":\"" + to_string(index) + "\","
                                                     "\"skillToken\":\"" + skillToken + "\",\"playbackSessionId\":\"" + playbackSessionId +
                                                     "\",\"navigation\":\"" + navigation + "\",\"preload\":" + (preload ? "true" : "false") +
                                                     "}";
                    return PLAY_PAYLOAD_TEST;
                }
                static string createPlayPayloadNoIndex(const string& playContext, int64_t offsetInMilliseconds, const string& playerId,
                                                       const string& skillToken, const string& playbackSessionId, const string& navigation,
                                                       bool preload) {
                    const string PLAY_PAYLOAD_TEST = "{\"playbackContextToken\":\"" + playContext + "\",\"offsetInMilliseconds\":" +
                                                     to_string(offsetInMilliseconds) + ",\"playerId\":\"" + playerId + "\","
                                                     "\"skillToken\":\"" + skillToken + "\",\"playbackSessionId\":\"" + playbackSessionId +
                                                     "\",\"navigation\":\"" + navigation + "\",\"preload\":" + (preload ? "true" : "false") +
                                                     "}";
                    return PLAY_PAYLOAD_TEST;
                }
                static string createPlayPayloadNoOffset(const string& playContext, int index, const string& playerId, const string& skillToken,
                                                        const string& playbackSessionId, const string& navigation, bool preload) {
                    const string PLAY_PAYLOAD_TEST = "{\"playbackContextToken\":\"" + playContext + "\",\"playerId\":\"" + playerId + "\","
                                                     "\"index\":\"" + to_string(index) + "\",\"skillToken\":\"" + skillToken + "\","
                                                     "\"playbackSessionId\":\"" + playbackSessionId + "\",\"navigation\":\"" + navigation +
                                                     "\",\"preload\":" + (preload ? "true" : "false") + "}";
                    return PLAY_PAYLOAD_TEST;
                }
                static string createLoginPayload(const string& accessToken, const string& userName, int64_t refreshInterval, bool forceLogin,
                                                 const string& playerId) {
                    const string LOGIN_PAYLOAD_TEST = "{\"playerId\":\"" + playerId + "\",\"accessToken\":\"" + accessToken + "\","
                                                      "\"tokenRefreshIntervalInMilliseconds\":" + to_string(refreshInterval) + ","
                                                      "\"forceLogin\": true" + ",\"username\":\"" + userName + "\"}";
                    return LOGIN_PAYLOAD_TEST;
                }
                static string createSeekPayload(int64_t timeOffset, const string& playerId, bool adjustSeek) {
                    string SEEK_PAYLOAD_TEST;
                    if (adjustSeek) {
                        SEEK_PAYLOAD_TEST = "{\"playerId\":\"" + playerId + "\",\"deltaPositionMilliseconds\":" + to_string(timeOffset) + "}";
                    } else SEEK_PAYLOAD_TEST = "{\"playerId\":\"" + playerId + "\",\"positionMilliseconds\":" + to_string(timeOffset) + "}";
                    return SEEK_PAYLOAD_TEST;
                }
                class ExternalMediaPlayerTest : public Test {
                public:
                    ExternalMediaPlayerTest();
                    void SetUp() override;
                    void TearDown() override;
                    void verifyState(const string& providedState, const string& expectedState);
                    void sendAuthorizeDiscoveredPlayersDirective(const string& payload, unique_ptr<DirectiveHandlerResultInterface> resultHandler = nullptr);
                    SetStateResult wakeOnSetState();
                    SetStateResult resetWakeOnSetState();
                    ExternalMediaPlayer::AdapterMediaPlayerMap m_adapterMediaPlayerMap;
                    ExternalMediaPlayer::AdapterSpeakerMap m_adapterSpeakerMap;
                    ExternalMediaPlayer::AdapterCreationMap m_adapterMap;
                    shared_ptr<ExternalMediaPlayer> m_externalMediaPlayer;
                    shared_ptr<MockMediaPlayer> m_mockMediaPlayer;
                    shared_ptr<MockChannelVolumeInterface> m_mockSpeakerInterface;
                    shared_ptr<MockSpeakerManager> m_mockSpeakerManager;
                    shared_ptr<MockMetricRecorder> m_metricRecorder;
                    shared_ptr<MockContextManager> m_mockContextManager;
                    shared_ptr<MockFocusManager> m_mockFocusManager;
                    unique_ptr<MockDirectiveHandlerResult> m_mockDirectiveHandlerResult;
                    shared_ptr<MockMessageSender> m_mockMessageSender;
                    shared_ptr<certifiedSender::test::MockCertifiedSender> m_mockCertifiedSender;
                    shared_ptr<MockExceptionEncounteredSender> m_mockExceptionSender;
                    shared_ptr<MockPlaybackRouter> m_mockPlaybackRouter;
                    shared_ptr<AttachmentManager> m_attachmentManager;
                    promise<void> m_wakeSetStatePromise;
                    future<void> m_wakeSetStateFuture;
                };
                ExternalMediaPlayerTest::ExternalMediaPlayerTest() :
                        m_wakeSetStatePromise{},
                        m_wakeSetStateFuture{m_wakeSetStatePromise.get_future()} {
                }
                void ExternalMediaPlayerTest::SetUp() {
                    m_mockSpeakerInterface = make_shared<NiceMock<MockChannelVolumeInterface>>();
                    m_mockSpeakerManager = make_shared<NiceMock<MockSpeakerManager>>();
                    m_metricRecorder = make_shared<NiceMock<MockMetricRecorder>>();
                    m_mockMessageSender = make_shared<NiceMock<MockMessageSender>>();
                    //m_mockFocusManager = make_shared<NiceMock<MockFocusManager>>();
                    m_mockContextManager = make_shared<NiceMock<MockContextManager>>();
                    m_mockExceptionSender = make_shared<NiceMock<MockExceptionEncounteredSender>>();
                    m_mockMediaPlayer = MockMediaPlayer::create();
                    m_mockPlaybackRouter = make_shared<NiceMock<MockPlaybackRouter>>();
                    m_attachmentManager = make_shared<AttachmentManager>(AttachmentManager::AttachmentType::IN_PROCESS);
                    m_mockCertifiedSender = make_shared<MockCertifiedSender>();
                    m_adapterMediaPlayerMap.insert(make_pair(MSP1_LOCAL_PLAYER_ID, m_mockMediaPlayer));
                    m_adapterSpeakerMap.insert(make_pair(MSP1_LOCAL_PLAYER_ID, m_mockSpeakerInterface));
                    m_adapterMap.insert(make_pair(MSP1_LOCAL_PLAYER_ID, &MockExternalMediaPlayerAdapter::getInstance));
                    m_externalMediaPlayer = ExternalMediaPlayer::create(m_adapterMediaPlayerMap, m_adapterSpeakerMap, m_adapterMap,
                                                           m_mockSpeakerManager, m_mockMessageSender, m_mockCertifiedSender->get(),
                                                            m_mockFocusManager, m_mockContextManager, m_mockExceptionSender,
                                                           m_mockPlaybackRouter, m_metricRecorder);
                    m_mockDirectiveHandlerResult = unique_ptr<MockDirectiveHandlerResult>(new MockDirectiveHandlerResult);
                    ASSERT_TRUE(m_externalMediaPlayer);
                    const string playersJson = createPlayerJson(MSP1_LOCAL_PLAYER_ID, true, MSP1_PLAYER_ID, MSP1_SKILLTOKEN);
                    sendAuthorizeDiscoveredPlayersDirective(createAuthorizeDiscoveredPlayersPayload({playersJson}));
                }
                void ExternalMediaPlayerTest::TearDown() {
                    EXPECT_CALL(*(MockExternalMediaPlayerAdapter::m_currentActiveMediaPlayerAdapter), doShutdown());
                    m_externalMediaPlayer->shutdown();
                    m_mockMediaPlayer->shutdown();
                    MockExternalMediaPlayerAdapter::m_currentActiveMediaPlayerAdapter.reset();
                }
                SetStateResult ExternalMediaPlayerTest::wakeOnSetState() {
                    m_wakeSetStatePromise.set_value();
                    return SetStateResult::SUCCESS;
                }
                SetStateResult ExternalMediaPlayerTest::resetWakeOnSetState() {
                    m_wakeSetStatePromise = promise<void>();
                    m_wakeSetStateFuture = m_wakeSetStatePromise.get_future();
                    return SetStateResult::SUCCESS;
                }
                void ExternalMediaPlayerTest::verifyState(const string& providedState, const string& expectedState) {
                    Document providedStateParsed;
                    providedStateParsed.Parse(providedState.data());
                    Document expectedStateParsed;
                    expectedStateParsed.Parse(expectedState.data());
                    EXPECT_EQ(providedStateParsed, expectedStateParsed);
                }
                TEST_F(ExternalMediaPlayerTest, test_createWithNullPointers) {
                    ExternalMediaPlayer::AdapterCreationMap adapterMap;
                    ExternalMediaPlayer::AdapterMediaPlayerMap adapterMediaPlayerMap;
                    ExternalMediaPlayer::AdapterSpeakerMap adapterSpeakerMap;
                    auto testExternalMediaPlayer = ExternalMediaPlayer::create(adapterMediaPlayerMap, adapterSpeakerMap, adapterMap,
                                                                  nullptr, m_mockMessageSender, m_mockCertifiedSender->get(),
                                                                    m_mockFocusManager, m_mockContextManager, m_mockExceptionSender,
                                                                   m_mockPlaybackRouter, m_metricRecorder);
                    EXPECT_EQ(testExternalMediaPlayer, nullptr);
                    testExternalMediaPlayer = ExternalMediaPlayer::create(adapterMediaPlayerMap, adapterSpeakerMap, adapterMap,
                                                             m_mockSpeakerManager, nullptr, m_mockCertifiedSender->get(),
                                                              m_mockFocusManager, m_mockContextManager, m_mockExceptionSender,
                                                             m_mockPlaybackRouter,m_metricRecorder);
                    EXPECT_EQ(testExternalMediaPlayer, nullptr);
                    testExternalMediaPlayer = ExternalMediaPlayer::create(adapterMediaPlayerMap, adapterSpeakerMap, adapterMap,
                                                             m_mockSpeakerManager, m_mockMessageSender, m_mockCertifiedSender->get(),
                                                              nullptr,m_mockContextManager,m_mockExceptionSender,
                                                             m_mockPlaybackRouter,m_metricRecorder);
                    EXPECT_EQ(testExternalMediaPlayer, nullptr);
                    testExternalMediaPlayer = ExternalMediaPlayer::create(adapterMediaPlayerMap, adapterSpeakerMap, adapterMap,
                                                            m_mockSpeakerManager, m_mockMessageSender,m_mockCertifiedSender->get(),
                                                              m_mockFocusManager, nullptr, m_mockExceptionSender,
                                                             m_mockPlaybackRouter, m_metricRecorder);
                    EXPECT_EQ(testExternalMediaPlayer, nullptr);
                    testExternalMediaPlayer = ExternalMediaPlayer::create(adapterMediaPlayerMap, adapterSpeakerMap, adapterMap,
                                                            m_mockSpeakerManager,m_mockMessageSender,m_mockCertifiedSender->get(),
                                                              m_mockFocusManager,m_mockContextManager,nullptr,
                                                             m_mockPlaybackRouter,m_metricRecorder);
                    EXPECT_EQ(testExternalMediaPlayer, nullptr);
                    testExternalMediaPlayer = ExternalMediaPlayer::create(adapterMediaPlayerMap, adapterSpeakerMap, adapterMap,
                                                            m_mockSpeakerManager,m_mockMessageSender,
                                                                          m_mockCertifiedSender->get(),m_mockFocusManager,
                                                             m_mockContextManager,m_mockExceptionSender,
                                                             nullptr,m_metricRecorder);
                    EXPECT_EQ(testExternalMediaPlayer, nullptr);
                }
                TEST_F(ExternalMediaPlayerTest, test_createWithAdapterCreationFailures) {
                    ExternalMediaPlayer::AdapterCreationMap adapterMap;
                    ExternalMediaPlayer::AdapterMediaPlayerMap adapterMediaPlayerMap;
                    ExternalMediaPlayer::AdapterSpeakerMap adapterSpeakerMap;
                    auto testExternalMediaPlayer = ExternalMediaPlayer::create(adapterMediaPlayerMap, adapterSpeakerMap, adapterMap,
                                                                  m_mockSpeakerManager,m_mockMessageSender,
                                                             m_mockCertifiedSender->get(),m_mockFocusManager,
                                                                  m_mockContextManager,m_mockExceptionSender,
                                                                   m_mockPlaybackRouter,m_metricRecorder);
                    ASSERT_TRUE(testExternalMediaPlayer);
                    testExternalMediaPlayer->shutdown();
                    adapterMap.clear();
                    adapterMediaPlayerMap.clear();
                    adapterMediaPlayerMap.insert(make_pair(MSP1_LOCAL_PLAYER_ID, m_mockMediaPlayer));
                    adapterMap.insert(make_pair(MSP2_LOCAL_PLAYER_ID, &MockExternalMediaPlayerAdapter::getInstance));
                    testExternalMediaPlayer = ExternalMediaPlayer::create(adapterMediaPlayerMap, adapterSpeakerMap, adapterMap,
                                                             m_mockSpeakerManager,m_mockMessageSender,
                                                        m_mockCertifiedSender->get(),m_mockFocusManager,
                                                             m_mockContextManager,m_mockExceptionSender,
                                                              m_mockPlaybackRouter,m_metricRecorder);
                    ASSERT_TRUE(testExternalMediaPlayer);
                    testExternalMediaPlayer->shutdown();
                }
                TEST_F(ExternalMediaPlayerTest, test_getConfiguration) {
                    auto configuration = m_externalMediaPlayer->getConfiguration();
                    auto audioNonBlockingPolicy = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, false);
                    auto neitherNonBlockingPolicy = BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false);
                    ASSERT_EQ(configuration[PLAY_DIRECTIVE], audioNonBlockingPolicy);
                    ASSERT_EQ(configuration[AUTHORIZEDISCOVEREDPLAYERS_DIRECTIVE], audioNonBlockingPolicy);
                    ASSERT_EQ(configuration[LOGIN_DIRECTIVE], neitherNonBlockingPolicy);
                    ASSERT_EQ(configuration[LOGOUT_DIRECTIVE], neitherNonBlockingPolicy);
                    ASSERT_EQ(configuration[RESUME_DIRECTIVE], audioNonBlockingPolicy);
                    ASSERT_EQ(configuration[PAUSE_DIRECTIVE], audioNonBlockingPolicy);
                    ASSERT_EQ(configuration[STOP_DIRECTIVE], audioNonBlockingPolicy);
                    ASSERT_EQ(configuration[NEXT_DIRECTIVE], audioNonBlockingPolicy);
                    ASSERT_EQ(configuration[PREVIOUS_DIRECTIVE], audioNonBlockingPolicy);
                    ASSERT_EQ(configuration[STARTOVER_DIRECTIVE], audioNonBlockingPolicy);
                    ASSERT_EQ(configuration[REWIND_DIRECTIVE], audioNonBlockingPolicy);
                    ASSERT_EQ(configuration[FASTFORWARD_DIRECTIVE], audioNonBlockingPolicy);
                    ASSERT_EQ(configuration[ENABLEREPEATONE_DIRECTIVE], neitherNonBlockingPolicy);
                    ASSERT_EQ(configuration[ENABLEREPEAT_DIRECTIVE], neitherNonBlockingPolicy);
                    ASSERT_EQ(configuration[DISABLEREPEAT_DIRECTIVE], neitherNonBlockingPolicy);
                    ASSERT_EQ(configuration[ENABLESHUFFLE_DIRECTIVE], neitherNonBlockingPolicy);
                    ASSERT_EQ(configuration[DISABLESHUFFLE_DIRECTIVE], neitherNonBlockingPolicy);
                    ASSERT_EQ(configuration[SEEK_DIRECTIVE], audioNonBlockingPolicy);
                    ASSERT_EQ(configuration[ADJUSTSEEK_DIRECTIVE], audioNonBlockingPolicy);
                    ASSERT_EQ(configuration[FAVORITE_DIRECTIVE], neitherNonBlockingPolicy);
                    ASSERT_EQ(configuration[UNFAVORITE_DIRECTIVE], neitherNonBlockingPolicy);
                }
                void ExternalMediaPlayerTest::sendAuthorizeDiscoveredPlayersDirective(
                    const std::string& payload,
                    std::unique_ptr<DirectiveHandlerResultInterface> resultHandler) {
                    if (!resultHandler) resultHandler = std::unique_ptr<MockDirectiveHandlerResult>(new MockDirectiveHandlerResult);
                    auto avsMessageHeader = std::make_shared<AVSMessageHeader>(
                        AUTHORIZEDISCOVEREDPLAYERS_DIRECTIVE.nameSpace, AUTHORIZEDISCOVEREDPLAYERS_DIRECTIVE.name, MESSAGE_ID_TEST2);
                    std::shared_ptr<AVSDirective> directive =
                        AVSDirective::create("", avsMessageHeader, payload, m_attachmentManager, "");
                    m_externalMediaPlayer->CapabilityAgent::preHandleDirective(directive, std::move(resultHandler));
                    m_externalMediaPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST2);
                }
                TEST_F(ExternalMediaPlayerTest, test_callingProvideSessionState) {
                    EXPECT_CALL(*(m_mockContextManager.get()), setState(SESSION_STATE, _, StateRefreshPolicy::ALWAYS, PROVIDE_STATE_TOKEN_TEST))
                        .Times(1).WillOnce(DoAll(Invoke([this](const NamespaceAndName& namespaceAndName, const string& jsonState,
                                           const StateRefreshPolicy& refreshPolicy, const unsigned int stateRequestToken) {
                                              string agent = "";
                                              verifyState(jsonState, getIdleSessionStateJson(agent));
                                           }),InvokeWithoutArgs(this, &ExternalMediaPlayerTest::wakeOnSetState)));
                    EXPECT_CALL(*(MockExternalMediaPlayerAdapter::m_currentActiveMediaPlayerAdapter), getState());
                    m_externalMediaPlayer->provideState(SESSION_STATE, PROVIDE_STATE_TOKEN_TEST);
                    ASSERT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                }
                TEST_F(ExternalMediaPlayerTest, test_callingProvidePlaybackState) {
                    EXPECT_CALL(*(m_mockContextManager.get()),setState(PLAYBACK_STATE, _, StateRefreshPolicy::ALWAYS, PROVIDE_STATE_TOKEN_TEST))
                        .Times(1).WillOnce(DoAll(Invoke([this](const NamespaceAndName& namespaceAndName, const string& jsonState,
                                                 const StateRefreshPolicy& refreshPolicy, const unsigned int stateRequestToken) { verifyState(jsonState, IDLE_PLAYBACK_STATE); }),
                                             InvokeWithoutArgs(this, &ExternalMediaPlayerTest::wakeOnSetState)));
                    EXPECT_CALL(*(MockExternalMediaPlayerAdapter::m_currentActiveMediaPlayerAdapter), getState()).Times(AtLeast(1));
                    m_externalMediaPlayer->provideState(PLAYBACK_STATE, PROVIDE_STATE_TOKEN_TEST);
                    ASSERT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                }
                TEST_F(ExternalMediaPlayerTest, test_playParserError) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(PLAY_DIRECTIVE.nameSpace, PLAY_DIRECTIVE.name, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader,createPlayPayloadWithParseError("XXX",
                                                                        0, 0, "Adapter", "YYY", "ZZZ",
                                                                     "DEFAULT", false),m_attachmentManager,"");
                    EXPECT_CALL(*(m_mockExceptionSender.get()), sendExceptionEncountered(_, _, _));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setFailed(_));
                    m_externalMediaPlayer->CapabilityAgent::preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    m_externalMediaPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                }
                TEST_F(ExternalMediaPlayerTest, test_playNoAdapter) {
                    auto avsMessageHeader = std::make_shared<AVSMessageHeader>(PLAY_DIRECTIVE.nameSpace, PLAY_DIRECTIVE.name, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader,createPlayPayload("XXX", 0,
                                                              0, "Adapter", "YYY", "ZZZ",
                                                                     "DEFAULT", false),m_attachmentManager,
                                                             "");
                    EXPECT_CALL(*(m_mockExceptionSender.get()), sendExceptionEncountered(_, _, _));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setFailed(_));
                    m_externalMediaPlayer->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
                    m_externalMediaPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                }
                TEST_F(ExternalMediaPlayerTest, test_playNoPlayContext) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(PLAY_DIRECTIVE.nameSpace, PLAY_DIRECTIVE.name, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader,createPlayPayloadNoContext(0, 0,
                                                                              MSP1_PLAYER_ID, "YYY", "ZZZ", "DEFAULT",
                                                                      false),m_attachmentManager,"");
                    EXPECT_CALL(*(m_mockExceptionSender.get()), sendExceptionEncountered(_, _, _));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setFailed(_));
                    m_externalMediaPlayer->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
                    m_externalMediaPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                }
                TEST_F(ExternalMediaPlayerTest, test_playNoPlayerId) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(PLAY_DIRECTIVE.nameSpace, PLAY_DIRECTIVE.name, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader,createPlayPayloadNoPlayerId("XXX", 0,
                                                              0, "YYY", "ZZZ", "DEFAULT",
                                                                      false),m_attachmentManager,"");
                    EXPECT_CALL(*(m_mockExceptionSender.get()), sendExceptionEncountered(_, _, _));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setFailed(_));
                    m_externalMediaPlayer->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
                    m_externalMediaPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                }
                TEST_F(ExternalMediaPlayerTest, test_playNoOffset) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(PLAY_DIRECTIVE.nameSpace, PLAY_DIRECTIVE.name, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader,createPlayPayloadNoOffset("XXX",
                                                                        0, MSP1_PLAYER_ID, "YYY", "ZZZ", "DEFAULT", false),
                                                              m_attachmentManager,"");
                    EXPECT_CALL(*(MockExternalMediaPlayerAdapter::m_currentActiveMediaPlayerAdapter),handlePlay(_, _, _, _, _, _, _, PlayRequestor{}));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted());
                    m_externalMediaPlayer->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
                    m_externalMediaPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                }
                TEST_F(ExternalMediaPlayerTest, testPlaywithPlayRequestor) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(PLAY_DIRECTIVE.nameSpace, PLAY_DIRECTIVE.name, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader,createPlayPayloadWithPlayRequestor("XXX",
                                                                        0, 0, MSP1_PLAYER_ID, "YYY", "ZZZ", "DEFAULT",
                                                                      false, testPlayRequestor),m_attachmentManager,
                                                             "");
                    EXPECT_CALL(*(MockExternalMediaPlayerAdapter::m_currentActiveMediaPlayerAdapter),handlePlay(_, _, _, _, _, _, _, testPlayRequestor));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted());
                    m_externalMediaPlayer->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
                    m_externalMediaPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                }
                TEST_F(ExternalMediaPlayerTest, test_playNoIndex) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(PLAY_DIRECTIVE.nameSpace, PLAY_DIRECTIVE.name, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader,createPlayPayloadNoIndex("XXX",
                                                              0, MSP1_PLAYER_ID, "YYY", "ZZZ", "DEFAULT", false),
                                                              m_attachmentManager,"");
                    EXPECT_CALL(*(MockExternalMediaPlayerAdapter::m_currentActiveMediaPlayerAdapter), handlePlay(_, _, _, _, _, _, _, _));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted());
                    m_externalMediaPlayer->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
                    m_externalMediaPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                }
                TEST_F(ExternalMediaPlayerTest, test_logout) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(LOGOUT_DIRECTIVE.nameSpace, LOGOUT_DIRECTIVE.name, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, createPayloadWithPlayerId(MSP1_PLAYER_ID),
                                                              m_attachmentManager, "");
                    EXPECT_CALL(*(MockExternalMediaPlayerAdapter::m_currentActiveMediaPlayerAdapter), handleLogout());
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted());
                    m_externalMediaPlayer->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
                    m_externalMediaPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                }
                TEST_F(ExternalMediaPlayerTest, test_login) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(LOGIN_DIRECTIVE.nameSpace, LOGIN_DIRECTIVE.name, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader,createLoginPayload("XXX",
                                                                     "msploginuser", 1000, false, MSP1_PLAYER_ID),
                                                              m_attachmentManager,"");
                    EXPECT_CALL(*(MockExternalMediaPlayerAdapter::m_currentActiveMediaPlayerAdapter), handleLogin(_, _, _, _));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted());
                    m_externalMediaPlayer->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
                    m_externalMediaPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                }
                TEST_F(ExternalMediaPlayerTest, test_loginStateChangeObserverIsNotified) {
                    auto observer = MockExternalMediaPlayerObserver::getInstance();
                    m_externalMediaPlayer->addObserver(observer);
                    EXPECT_CALL(*(m_mockContextManager.get()), setState(SESSION_STATE, _, StateRefreshPolicy::ALWAYS, PROVIDE_STATE_TOKEN_TEST))
                        .Times(1).WillOnce(InvokeWithoutArgs(this, &ExternalMediaPlayerTest::wakeOnSetState));
                    EXPECT_CALL(*(MockExternalMediaPlayerAdapter::m_currentActiveMediaPlayerAdapter), getState()).WillRepeatedly(Return(createAdapterState()));
                    ObservableSessionProperties observableSessionProperties{false, PLAYER_USER_NAME};
                    //EXPECT_CALL(*(observer), onLoginStateProvided(PLAYER_ID, observableSessionProperties)).Times(1);
                    m_externalMediaPlayer->provideState(SESSION_STATE, PROVIDE_STATE_TOKEN_TEST);
                    ASSERT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                }
                TEST_F(ExternalMediaPlayerTest, test_playbackStateChangeObserverIsNotified) {
                    auto observer = MockExternalMediaPlayerObserver::getInstance();
                    m_externalMediaPlayer->addObserver(observer);
                    EXPECT_CALL(*(m_mockContextManager.get()),setState(PLAYBACK_STATE, _, StateRefreshPolicy::ALWAYS, PROVIDE_STATE_TOKEN_TEST))
                        .Times(1).WillRepeatedly(InvokeWithoutArgs(this, &ExternalMediaPlayerTest::wakeOnSetState));
                    EXPECT_CALL(*(MockExternalMediaPlayerAdapter::m_currentActiveMediaPlayerAdapter), getState())
                        .WillRepeatedly(Return(createAdapterState()));
                    ObservablePlaybackStateProperties observablePlaybackStateProperties{PLAYER_STATE, PLAYER_TRACK, testPlayRequestor};
                    //EXPECT_CALL(*(observer), onPlaybackStateProvided(PLAYER_ID, observablePlaybackStateProperties)).Times(1);
                    m_externalMediaPlayer->provideState(PLAYBACK_STATE, PROVIDE_STATE_TOKEN_TEST);
                    ASSERT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                }
                TEST_F(ExternalMediaPlayerTest, test_loginStateChangeObserverRemoval) {
                    auto observer = MockExternalMediaPlayerObserver::getInstance();
                    m_externalMediaPlayer->addObserver(observer);
                    EXPECT_CALL(*(m_mockContextManager.get()), setState(SESSION_STATE, _, StateRefreshPolicy::ALWAYS, PROVIDE_STATE_TOKEN_TEST))
                        .Times(2).WillRepeatedly(InvokeWithoutArgs(this, &ExternalMediaPlayerTest::wakeOnSetState));
                    EXPECT_CALL(*(MockExternalMediaPlayerAdapter::m_currentActiveMediaPlayerAdapter), getState()).WillRepeatedly(Return(createAdapterState()));
                    //EXPECT_CALL(*(observer), onLoginStateProvided(_, _)).Times(1);
                    m_externalMediaPlayer->provideState(SESSION_STATE, PROVIDE_STATE_TOKEN_TEST);
                    ASSERT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                    this->resetWakeOnSetState();
                    m_externalMediaPlayer->removeObserver(observer);
                    //EXPECT_CALL(*(observer), onLoginStateProvided(_, _)).Times(0);
                    m_externalMediaPlayer->provideState(SESSION_STATE, PROVIDE_STATE_TOKEN_TEST);
                    ASSERT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                }
                TEST_F(ExternalMediaPlayerTest, test_playbackStateChangeObserverRemoval) {
                    auto observer = MockExternalMediaPlayerObserver::getInstance();
                    m_externalMediaPlayer->addObserver(observer);
                    EXPECT_CALL(*(m_mockContextManager.get()),setState(PLAYBACK_STATE, _, StateRefreshPolicy::ALWAYS, PROVIDE_STATE_TOKEN_TEST))
                        .Times(2).WillRepeatedly(InvokeWithoutArgs(this, &ExternalMediaPlayerTest::wakeOnSetState));
                    EXPECT_CALL(*(MockExternalMediaPlayerAdapter::m_currentActiveMediaPlayerAdapter), getState()).WillRepeatedly(Return(createAdapterState()));
                    //EXPECT_CALL(*(observer), onPlaybackStateProvided(_, _)).Times(1);
                    m_externalMediaPlayer->provideState(PLAYBACK_STATE, PROVIDE_STATE_TOKEN_TEST);
                    ASSERT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                    this->resetWakeOnSetState();
                    m_externalMediaPlayer->removeObserver(observer);
                    //EXPECT_CALL(*(observer), onPlaybackStateProvided(_, _)).Times(0);
                    m_externalMediaPlayer->provideState(PLAYBACK_STATE, PROVIDE_STATE_TOKEN_TEST);
                    ASSERT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                }
                TEST_F(ExternalMediaPlayerTest, test_play) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(RESUME_DIRECTIVE.nameSpace, RESUME_DIRECTIVE.name, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, createPayloadWithPlayerId(MSP1_PLAYER_ID),
                                                              m_attachmentManager, "");
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted());
                    EXPECT_CALL(*(MockExternalMediaPlayerAdapter::m_currentActiveMediaPlayerAdapter), handlePlayControl(_));
                    m_externalMediaPlayer->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
                    m_externalMediaPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                }
                TEST_F(ExternalMediaPlayerTest, test_pause) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(PAUSE_DIRECTIVE.nameSpace, PAUSE_DIRECTIVE.name, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, createPayloadWithPlayerId(MSP1_PLAYER_ID),
                                                              m_attachmentManager, "");
                    EXPECT_CALL(*(MockExternalMediaPlayerAdapter::m_currentActiveMediaPlayerAdapter), handlePlayControl(_));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted());
                    m_externalMediaPlayer->CapabilityAgent::preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    m_externalMediaPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                }
                TEST_F(ExternalMediaPlayerTest, testStop) {
                    auto avsMessageHeader = std::make_shared<AVSMessageHeader>(STOP_DIRECTIVE.nameSpace, STOP_DIRECTIVE.name, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, createPayloadWithPlayerId(MSP1_PLAYER_ID),
                                                              m_attachmentManager, "");
                    EXPECT_CALL(*(MockExternalMediaPlayerAdapter::m_currentActiveMediaPlayerAdapter), handlePlayControl(_));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted());
                    m_externalMediaPlayer->CapabilityAgent::preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    m_externalMediaPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                }
                TEST_F(ExternalMediaPlayerTest, test_stop) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(STOP_DIRECTIVE.nameSpace, STOP_DIRECTIVE.name, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, createPayloadWithPlayerId(MSP1_PLAYER_ID),
                                                              m_attachmentManager, "");
                    EXPECT_CALL(*(MockExternalMediaPlayerAdapter::m_currentActiveMediaPlayerAdapter), handlePlayControl(_));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted());
                    m_externalMediaPlayer->CapabilityAgent::preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    m_externalMediaPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                }
                TEST_F(ExternalMediaPlayerTest, test_next) {
                    auto avsMessageHeader = std::make_shared<AVSMessageHeader>(NEXT_DIRECTIVE.nameSpace, NEXT_DIRECTIVE.name, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, createPayloadWithPlayerId(MSP1_PLAYER_ID),
                                                              m_attachmentManager, "");
                    EXPECT_CALL(*(MockExternalMediaPlayerAdapter::m_currentActiveMediaPlayerAdapter), handlePlayControl(_));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted());
                    m_externalMediaPlayer->CapabilityAgent::preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    m_externalMediaPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                }
                TEST_F(ExternalMediaPlayerTest, test_previous) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(PREVIOUS_DIRECTIVE.nameSpace, PREVIOUS_DIRECTIVE.name, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, createPayloadWithPlayerId(MSP1_PLAYER_ID),
                                                              m_attachmentManager, "");
                    EXPECT_CALL(*(MockExternalMediaPlayerAdapter::m_currentActiveMediaPlayerAdapter), handlePlayControl(_));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted());
                    m_externalMediaPlayer->CapabilityAgent::preHandleDirective(directive,move(m_mockDirectiveHandlerResult));
                    m_externalMediaPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                }
                TEST_F(ExternalMediaPlayerTest, test_startOver) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(STARTOVER_DIRECTIVE.nameSpace, STARTOVER_DIRECTIVE.name, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, createPayloadWithPlayerId(MSP1_PLAYER_ID),
                                                              m_attachmentManager, "");
                    EXPECT_CALL(*(MockExternalMediaPlayerAdapter::m_currentActiveMediaPlayerAdapter), handlePlayControl(_));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted());
                    m_externalMediaPlayer->CapabilityAgent::preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    m_externalMediaPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                }
                TEST_F(ExternalMediaPlayerTest, test_rewind) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(REWIND_DIRECTIVE.nameSpace, REWIND_DIRECTIVE.name, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, createPayloadWithPlayerId(MSP1_PLAYER_ID),
                                                              m_attachmentManager, "");
                    EXPECT_CALL(*(MockExternalMediaPlayerAdapter::m_currentActiveMediaPlayerAdapter), handlePlayControl(_));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted());
                    m_externalMediaPlayer->CapabilityAgent::preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    m_externalMediaPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                }
                TEST_F(ExternalMediaPlayerTest, test_fastForward) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(FASTFORWARD_DIRECTIVE.nameSpace, FASTFORWARD_DIRECTIVE.name, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, createPayloadWithPlayerId(MSP1_PLAYER_ID),
                                                              m_attachmentManager, "");
                    EXPECT_CALL(*(MockExternalMediaPlayerAdapter::m_currentActiveMediaPlayerAdapter), handlePlayControl(_));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted());
                    m_externalMediaPlayer->CapabilityAgent::preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    m_externalMediaPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                }
                TEST_F(ExternalMediaPlayerTest, test_enableRepeatOne) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(ENABLEREPEATONE_DIRECTIVE.nameSpace, ENABLEREPEATONE_DIRECTIVE.name, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, createPayloadWithPlayerId(MSP1_PLAYER_ID),
                                                              m_attachmentManager, "");
                    EXPECT_CALL(*(MockExternalMediaPlayerAdapter::m_currentActiveMediaPlayerAdapter), handlePlayControl(_));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted());
                    m_externalMediaPlayer->CapabilityAgent::preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    m_externalMediaPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                }
                TEST_F(ExternalMediaPlayerTest, test_enableRepeat) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(ENABLEREPEAT_DIRECTIVE.nameSpace, ENABLEREPEAT_DIRECTIVE.name, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, createPayloadWithPlayerId(MSP1_PLAYER_ID),
                                                              m_attachmentManager, "");
                    EXPECT_CALL(*(MockExternalMediaPlayerAdapter::m_currentActiveMediaPlayerAdapter), handlePlayControl(_));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted());
                    m_externalMediaPlayer->CapabilityAgent::preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    m_externalMediaPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                }
                TEST_F(ExternalMediaPlayerTest, test_disableRepeat) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(DISABLEREPEAT_DIRECTIVE.nameSpace, DISABLEREPEAT_DIRECTIVE.name, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, createPayloadWithPlayerId(MSP1_PLAYER_ID), m_attachmentManager, "");
                    EXPECT_CALL(*(MockExternalMediaPlayerAdapter::m_currentActiveMediaPlayerAdapter), handlePlayControl(_));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted());
                    m_externalMediaPlayer->CapabilityAgent::preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    m_externalMediaPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                }
                TEST_F(ExternalMediaPlayerTest, test_enableShuffle) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(ENABLESHUFFLE_DIRECTIVE.nameSpace, ENABLESHUFFLE_DIRECTIVE.name, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, createPayloadWithPlayerId(MSP1_PLAYER_ID), m_attachmentManager, "");
                    EXPECT_CALL(*(MockExternalMediaPlayerAdapter::m_currentActiveMediaPlayerAdapter), handlePlayControl(_));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted());
                    m_externalMediaPlayer->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
                    m_externalMediaPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                }
                TEST_F(ExternalMediaPlayerTest, test_disableShuffle) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(DISABLESHUFFLE_DIRECTIVE.nameSpace, DISABLESHUFFLE_DIRECTIVE.name, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, createPayloadWithPlayerId(MSP1_PLAYER_ID), m_attachmentManager, "");
                    EXPECT_CALL(*(MockExternalMediaPlayerAdapter::m_currentActiveMediaPlayerAdapter), handlePlayControl(_));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted());
                    m_externalMediaPlayer->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
                    m_externalMediaPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                }
                TEST_F(ExternalMediaPlayerTest, test_favorite) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(FAVORITE_DIRECTIVE.nameSpace, FAVORITE_DIRECTIVE.name, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, createPayloadWithPlayerId(MSP1_PLAYER_ID), m_attachmentManager, "");
                    EXPECT_CALL(*(MockExternalMediaPlayerAdapter::m_currentActiveMediaPlayerAdapter), handlePlayControl(_));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted());
                    m_externalMediaPlayer->CapabilityAgent::preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    m_externalMediaPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                }
                TEST_F(ExternalMediaPlayerTest, test_unfavorite) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(UNFAVORITE_DIRECTIVE.nameSpace, UNFAVORITE_DIRECTIVE.name, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, createPayloadWithPlayerId(MSP1_PLAYER_ID), m_attachmentManager, "");
                    EXPECT_CALL(*(MockExternalMediaPlayerAdapter::m_currentActiveMediaPlayerAdapter), handlePlayControl(_));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted());
                    m_externalMediaPlayer->CapabilityAgent::preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    m_externalMediaPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                }
                TEST_F(ExternalMediaPlayerTest, test_incorrectDirective) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(FAVORITE_DIRECTIVE.nameSpace, PREVIOUS_DIRECTIVE.name, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, createPayloadWithPlayerId(MSP1_PLAYER_ID), m_attachmentManager, "");
                    EXPECT_CALL(*(m_mockExceptionSender.get()), sendExceptionEncountered(_, _, _));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setFailed(_));
                    m_externalMediaPlayer->CapabilityAgent::preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    m_externalMediaPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                }
                TEST_F(ExternalMediaPlayerTest, test_seekFailure) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(SEEK_DIRECTIVE.nameSpace, SEEK_DIRECTIVE.name, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, createSeekPayload(100, MSP1_PLAYER_ID, true), m_attachmentManager, "");
                    EXPECT_CALL(*(m_mockExceptionSender.get()), sendExceptionEncountered(_, _, _));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setFailed(_));
                    m_externalMediaPlayer->CapabilityAgent::preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    m_externalMediaPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                }
                TEST_F(ExternalMediaPlayerTest, test_seekSuccess) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(SEEK_DIRECTIVE.nameSpace, SEEK_DIRECTIVE.name, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, createSeekPayload(100, MSP1_PLAYER_ID, false), m_attachmentManager, "");
                    EXPECT_CALL(*(MockExternalMediaPlayerAdapter::m_currentActiveMediaPlayerAdapter), handleSeek(_));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted());
                    m_externalMediaPlayer->CapabilityAgent::preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    m_externalMediaPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                }
                TEST_F(ExternalMediaPlayerTest, test_adjustSeekFailure) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(ADJUSTSEEK_DIRECTIVE.nameSpace, ADJUSTSEEK_DIRECTIVE.name, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, createSeekPayload(100, MSP1_PLAYER_ID, false), m_attachmentManager, "");
                    EXPECT_CALL(*(m_mockExceptionSender.get()), sendExceptionEncountered(_, _, _));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setFailed(_));
                    m_externalMediaPlayer->CapabilityAgent::preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    m_externalMediaPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                }
                TEST_F(ExternalMediaPlayerTest, test_adjustSeekFailure2) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(ADJUSTSEEK_DIRECTIVE.nameSpace, ADJUSTSEEK_DIRECTIVE.name, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, createSeekPayload(86400014, MSP1_PLAYER_ID, true), m_attachmentManager, "");
                    EXPECT_CALL(*(m_mockExceptionSender.get()), sendExceptionEncountered(_, _, _));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setFailed(_));
                    m_externalMediaPlayer->CapabilityAgent::preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    m_externalMediaPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                }
                TEST_F(ExternalMediaPlayerTest, test_adjustSeekSuccess) {
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(ADJUSTSEEK_DIRECTIVE.nameSpace, ADJUSTSEEK_DIRECTIVE.name, MESSAGE_ID_TEST, DIALOG_REQUEST_ID_TEST);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, createSeekPayload(86400000, MSP1_PLAYER_ID, true), m_attachmentManager, "");
                    EXPECT_CALL(*(MockExternalMediaPlayerAdapter::m_currentActiveMediaPlayerAdapter), handleAdjustSeek(_));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted());
                    m_externalMediaPlayer->CapabilityAgent::preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    m_externalMediaPlayer->CapabilityAgent::handleDirective(MESSAGE_ID_TEST);
                }
                MATCHER_P2(EventNamed, expectedNameSpace, expectedName,"") {
                    shared_ptr<MessageRequest> request = arg;
                    if (!request) return false;
                    Document document;
                    ParseResult result = document.Parse(request->getJsonContent().data());
                    if (!result) return false;
                    Value::ConstMemberIterator eventIt;
                    Value::ConstMemberIterator headerIt;
                    rapidjson::Value _document{document.GetString(), strlen(document.GetString())};
                    if (!jsonUtils::findNode(_document, "event", &eventIt) || !jsonUtils::findNode(eventIt->value, "header", &headerIt)) {
                        return false;
                    }
                    string name;
                    string nameSpace;
                    if (!jsonUtils::retrieveValue(headerIt->value, "name", &name) ||
                        !jsonUtils::retrieveValue(headerIt->value, "namespace", &nameSpace) || nameSpace != expectedNameSpace ||
                        name != expectedName) {
                        return false;
                    }
                    return true;
                }
                static void veryifyAuthorizationCompletePayload(shared_ptr<MessageRequest> request, unordered_map<string, string> expectedAuthorized,
                                                                unordered_set<string> expectedDeauthorized = unordered_set<string>()) {
                    Document document;
                    ParseResult result = document.Parse(request->getJsonContent().data());
                    if (!result) { FAIL(); }
                    Value::ConstMemberIterator eventIt;
                    Value::ConstMemberIterator payloadIt;
                    Value::ConstMemberIterator authorizedIt;
                    Value::ConstMemberIterator deauthorizedIt;
                    rapidjson::Value _document{document.GetString(), strlen(document.GetString())};
                    if (!findNode(_document, "event", &eventIt) || !findNode(eventIt->value, "payload", &payloadIt) ||
                        !findNode(payloadIt->value, "authorized", &authorizedIt) ||
                        !findNode(payloadIt->value, "deauthorized", &deauthorizedIt)) {
                        FAIL();
                    }
                    unordered_map<string, string> authorized;
                    unordered_set<string> deauthorized;
                    for (Value::ConstValueIterator it = authorizedIt->value.Begin(); it != authorizedIt->value.End(); it++) {
                        string playerId;
                        string skillToken;
                        if (!retrieveValue(*it, "playerId", &playerId) || !retrieveValue(*it, "skillToken", &skillToken)) { FAIL(); }
                        authorized[playerId] = skillToken;
                    }
                    for (Value::ConstValueIterator it = deauthorizedIt->value.Begin(); it != deauthorizedIt->value.End(); it++) {
                        string localPlayerId;
                        if (!retrieveValue(*it, "localPlayerId", &localPlayerId)) { FAIL(); }
                        deauthorized.insert(localPlayerId);
                    }
                    ASSERT_THAT(authorized, ContainerEq(expectedAuthorized));
                    ASSERT_THAT(deauthorized, ContainerEq(expectedDeauthorized));
                }
                TEST_F(ExternalMediaPlayerTest, testReportDiscoveredPlayers) {
                    promise<void> eventPromise;
                    future<void> eventFuture = eventPromise.get_future();
                    auto mockCertifiedSender = make_shared<certifiedSender::test::MockCertifiedSender>();
                    shared_ptr<ConnectionStatusObserverInterface> connectionObserver = static_pointer_cast<ConnectionStatusObserverInterface>(mockCertifiedSender->get());
                    connectionObserver->onConnectionStatusChanged(ConnectionStatusObserverInterface::Status::CONNECTED,
                                                           ConnectionStatusObserverInterface::ChangedReason::SUCCESS);
                    EXPECT_CALL(*(mockCertifiedSender->getMockMessageSender()),sendMessage(EventNamed(REPORT_DISCOVERED_PLAYERS.nameSpace, REPORT_DISCOVERED_PLAYERS.name)))
                        .Times(1).WillOnce(InvokeWithoutArgs([&eventPromise]() { eventPromise.set_value(); }));
                    auto messageSender = make_shared<NiceMock<MockMessageSender>>();
                    m_externalMediaPlayer = ExternalMediaPlayer::create(m_adapterMediaPlayerMap, m_adapterSpeakerMap, m_adapterMap,m_mockSpeakerManager,
                                                           m_mockMessageSender, mockCertifiedSender->get(),m_mockFocusManager,
                                                           m_mockContextManager,m_mockExceptionSender,
                                                           m_mockPlaybackRouter,m_metricRecorder);
                    ASSERT_TRUE(future_status::ready == eventFuture.wait_for(MY_WAIT_TIMEOUT));
                }
                TEST_F(ExternalMediaPlayerTest, testTimer_AuthorizeDiscoveredPlayersSuccess) {
                    std::promise<void> authorizationCompletePromise;
                    std::future<void> authorizationCompleteFuture = authorizationCompletePromise.get_future();
                    ON_CALL(*m_mockContextManager, getContext(_, _, _)).WillByDefault(InvokeWithoutArgs([this]() {
                        m_externalMediaPlayer->onContextAvailable("");
                        return 0;
                    }));
                    auto messageSender = make_shared<NiceMock<MockMessageSender>>();
                    m_externalMediaPlayer = ExternalMediaPlayer::create(m_adapterMediaPlayerMap, m_adapterSpeakerMap, m_adapterMap,
                                                          m_mockSpeakerManager, messageSender,m_mockCertifiedSender->get(),
                                                            m_mockFocusManager,m_mockContextManager,m_mockExceptionSender,
                                                           m_mockPlaybackRouter,m_metricRecorder);
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted());
                    EXPECT_CALL(*messageSender, sendMessage(EventNamed(AUTHORIZATION_COMPLETE.nameSpace, AUTHORIZATION_COMPLETE.name)))
                        .Times(1).WillOnce(Invoke([&authorizationCompletePromise](std::shared_ptr<MessageRequest> request) {
                            veryifyAuthorizationCompletePayload(request, {{MSP1_PLAYER_ID, MSP1_SKILLTOKEN}});
                            authorizationCompletePromise.set_value();
                        }));
                    EXPECT_CALL(*(MockExternalMediaPlayerAdapter::m_currentActiveMediaPlayerAdapter),handleAuthorized(true, MSP1_PLAYER_ID, MSP1_SKILLTOKEN));
                    const string playersJson = createPlayerJson(MSP1_LOCAL_PLAYER_ID, true, MSP1_PLAYER_ID, MSP1_SKILLTOKEN);
                    sendAuthorizeDiscoveredPlayersDirective(createAuthorizeDiscoveredPlayersPayload({playersJson}),move(m_mockDirectiveHandlerResult));
                    ASSERT_TRUE(future_status::ready == authorizationCompleteFuture.wait_for(MY_WAIT_TIMEOUT));
                }
                TEST_F(ExternalMediaPlayerTest, testMultipleAuthorizeDiscoveredPlayersSuccess) {
                    ON_CALL(*m_mockContextManager, getContext(_, _, _)).WillByDefault(InvokeWithoutArgs([this]() {
                        m_externalMediaPlayer->onContextAvailable("");
                        return 0;
                    }));
                    auto messageSender = make_shared<NiceMock<MockMessageSender>>();
                    m_externalMediaPlayer = ExternalMediaPlayer::create(m_adapterMediaPlayerMap, m_adapterSpeakerMap, m_adapterMap,
                                                          m_mockSpeakerManager, messageSender,m_mockCertifiedSender->get(),
                                                            m_mockFocusManager,m_mockContextManager,
                                                          m_mockExceptionSender,m_mockPlaybackRouter,
                                                           m_metricRecorder);
                    promise<void> authorizationCompletePromise;
                    future<void> authorizationCompleteFuture = authorizationCompletePromise.get_future();
                    auto mockDirectiveHandlerResult = unique_ptr<MockDirectiveHandlerResult>(new MockDirectiveHandlerResult);
                    EXPECT_CALL(*mockDirectiveHandlerResult, setCompleted());
                    EXPECT_CALL(*(MockExternalMediaPlayerAdapter::m_currentActiveMediaPlayerAdapter),handleAuthorized(true, MSP1_PLAYER_ID, MSP1_SKILLTOKEN));
                    promise<void> authorizationCompletePromise2;
                    future<void> authorizationCompleteFuture2 = authorizationCompletePromise2.get_future();
                    auto mockDirectiveHandlerResult2 = unique_ptr<MockDirectiveHandlerResult>(new MockDirectiveHandlerResult);
                    EXPECT_CALL(*mockDirectiveHandlerResult2, setCompleted());
                    EXPECT_CALL(*(MockExternalMediaPlayerAdapter::m_currentActiveMediaPlayerAdapter), handleAuthorized(false, "", ""));
                    {
                        InSequence s;
                        EXPECT_CALL(*messageSender, sendMessage(EventNamed(AUTHORIZATION_COMPLETE.nameSpace, AUTHORIZATION_COMPLETE.name)))
                            .Times(1).WillOnce(Invoke([&authorizationCompletePromise](std::shared_ptr<MessageRequest> request) {
                                veryifyAuthorizationCompletePayload(request, {{MSP1_PLAYER_ID, MSP1_SKILLTOKEN}});
                                authorizationCompletePromise.set_value();
                            }));
                        EXPECT_CALL(*messageSender, sendMessage(EventNamed(AUTHORIZATION_COMPLETE.nameSpace, AUTHORIZATION_COMPLETE.name)))
                            .Times(1).WillOnce(Invoke([&authorizationCompletePromise2](std::shared_ptr<MessageRequest> request) {
                                veryifyAuthorizationCompletePayload(request, unordered_map<string, string>(), {MSP1_LOCAL_PLAYER_ID});
                                authorizationCompletePromise2.set_value();
                            }));
                    }
                    const string playersJson = createPlayerJson(MSP1_LOCAL_PLAYER_ID, true, MSP1_PLAYER_ID, MSP1_SKILLTOKEN);
                    sendAuthorizeDiscoveredPlayersDirective(createAuthorizeDiscoveredPlayersPayload({playersJson}), move(mockDirectiveHandlerResult));
                    sendAuthorizeDiscoveredPlayersDirective(createAuthorizeDiscoveredPlayersPayload(), move(mockDirectiveHandlerResult2));
                    ASSERT_TRUE(future_status::ready == authorizationCompleteFuture.wait_for(MY_WAIT_TIMEOUT));
                    ASSERT_TRUE(future_status::ready == authorizationCompleteFuture2.wait_for(MY_WAIT_TIMEOUT));
                }
                TEST_F(ExternalMediaPlayerTest, testSetPlayerInFocusSucceedsForAuthorized) {
                    EXPECT_CALL(*m_mockContextManager, setState(SESSION_STATE, _, _, _))
                        .WillOnce(Invoke([this](const NamespaceAndName& namespaceAndName, const string& jsonState,
                                  const StateRefreshPolicy& refreshPolicy, const unsigned int stateRequestToken) {
                            Document document;
                            ParseResult result = document.Parse(jsonState.data());
                            if (!result) return SetStateResult::SUCCESS;
                            string playerInFocus;
                            if (!retrieveValue(document, "playerInFocus", &playerInFocus)) return SetStateResult::SUCCESS;
                            if (MSP1_PLAYER_ID == playerInFocus) wakeOnSetState();
                            return SetStateResult::SUCCESS;
                        }));
                    m_externalMediaPlayer->setPlayerInFocus(MSP1_PLAYER_ID);
                    m_externalMediaPlayer->provideState(SESSION_STATE, PROVIDE_STATE_TOKEN_TEST);
                    ASSERT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                }
                TEST_F(ExternalMediaPlayerTest, testSetPlayerInFocusFailsForAuthorized) {
                    const string INVALID_ID = "invalidPlayerId";
                    EXPECT_CALL(*m_mockPlaybackRouter, setHandler(_)).Times(0);
                    EXPECT_CALL(*m_mockContextManager, setState(_, Not(HasSubstr(INVALID_ID)), _, _))
                        .WillOnce(InvokeWithoutArgs(this, &ExternalMediaPlayerTest::wakeOnSetState));
                    m_externalMediaPlayer->setPlayerInFocus(INVALID_ID);
                    m_externalMediaPlayer->provideState(SESSION_STATE, PROVIDE_STATE_TOKEN_TEST);
                    ASSERT_TRUE(future_status::ready == m_wakeSetStateFuture.wait_for(MY_WAIT_TIMEOUT));
                }
                TEST_F(ExternalMediaPlayerTest, testSetPlayerInFocusNotfiesTemplateRuntimeObserver) {
                    promise<void> promise;
                    future<void> future = promise.get_future();
                    auto renderCardObserver = make_shared<MockRenderPlayerInfoCardsObserver>();
                    m_externalMediaPlayer->setObserver(renderCardObserver);
                    /*EXPECT_CALL(*renderCardObserver, onRenderPlayerCardsInfoChanged(_, _))
                        .WillOnce(Invoke([&promise](PlayerActivity state, const RenderPlayerInfoCardsObserverInterface::Context& context) { promise.set_value(); }));*/
                    m_externalMediaPlayer->setPlayerInFocus(MSP1_PLAYER_ID);
                    m_externalMediaPlayer->provideState(PLAYBACK_STATE, PROVIDE_STATE_TOKEN_TEST);
                    ASSERT_TRUE(future_status::ready == future.wait_for(MY_WAIT_TIMEOUT));
                }
            }
        }
    }
}