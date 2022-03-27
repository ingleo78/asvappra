#include <chrono>
#include <thread>
#include <gtest/gtest.h>
#include <json/document.h>
#include <avs/MessageRequest.h>
#include <json/JSONUtils.h>
#include <sdkinterfaces/ContextRequestToken.h>
#include <sdkinterfaces/MockMessageSender.h>
#include <sdkinterfaces/MockContextManager.h>
#include <sdkinterfaces/MockPlaybackRouter.h>
#include "PlaybackController.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace playbackController {
            namespace test {
                using namespace chrono;
                using namespace rapidjson;
                using namespace json;
                using namespace testing;
                using namespace sdkInterfaces::test;
                static const string PLAYBACK_CONTROLLER_NAMESPACE = "PlaybackController";
                static const string PLAYBACK_PLAY_NAME = "PlayCommandIssued";
                static const string PLAYBACK_PAUSE_NAME = "PauseCommandIssued";
                static const string PLAYBACK_NEXT_NAME = "NextCommandIssued";
                static const string PLAYBACK_PREVIOUS_NAME = "PreviousCommandIssued";
                static const string PLAYBACK_BUTTON_NAME = "ButtonCommandIssued";
                static const string PLAYBACK_SKIPFORWARD_NAME = "SKIPFORWARD";
                static const string PLAYBACK_SKIPBACKWARD_NAME = "SKIPBACKWARD";
                static const string PLAYBACK_TOGGLE_NAME = "ToggleCommandIssued";
                static const string PLAYBACK_SHUFFLE_NAME = "SHUFFLE";
                static const string PLAYBACK_LOOP_NAME = "LOOP";
                static const string PLAYBACK_REPEAT_NAME = "REPEAT";
                static const string PLAYBACK_THUMBSUP_NAME = "THUMBSUP";
                static const string PLAYBACK_THUMBSDOWN_NAME = "THUMBSDOWN";
                static const string PLAYBACK_SELECTED_NAME = "SELECT";
                static const string PLAYBACK_DESELECTED_NAME = "DESELECT";
                static const string TEST_EXCEPTION_TEXT = "Exception test";
                static const milliseconds TEST_RESULT_WAIT_PERIOD{100};
                static const ContextRequestToken CONTEXT_REQUEST_TOKEN{1};
                static const string MOCK_CONTEXT = "{\"context\":[{\"header\":{\"name\":\"SpeechState\",\"namespace\":\"SpeechSynthesizer\""
                                                   "},\"payload\":{\"playerActivity\":\"FINISHED\",\"offsetInMilliseconds\":0,"
                                                   "\"token\":\"\"}}]}";
                static string checkMessageRequest(shared_ptr<MessageRequest> messageRequest, const string& expected_payload_name,
                                                  const string& expected_payload_action) {
                    const string error = "ERROR";
                    Document jsonContent(rapidjson::kObjectType);
                    if (jsonContent.Parse(messageRequest->getJsonContent().data()).HasParseError()) return error;
                    rapidjson::Value::ConstMemberIterator eventNode;
                    rapidjson::Value _jsonContent{jsonContent.GetString(), strlen(jsonContent.GetString())};
                    if (!jsonUtils::findNode(_jsonContent, "event", &eventNode)) return error;
                    rapidjson::Value::ConstMemberIterator payloadNode;
                    if (!jsonUtils::findNode(eventNode->value, "payload", &payloadNode)) return error;
                    if (!payloadNode->value.ObjectEmpty() && ("" == expected_payload_name)) return error;
                    string eventPayloadName;
                    jsonUtils::retrieveValue(payloadNode->value, "name", &eventPayloadName);
                    if (eventPayloadName != expected_payload_name) return error;
                    string eventPayloadAction;
                    jsonUtils::retrieveValue(payloadNode->value, "action", &eventPayloadAction);
                    if (eventPayloadAction != expected_payload_action) return error;
                    rapidjson::Value::ConstMemberIterator headerIt;
                    if (!jsonUtils::findNode(eventNode->value, "header", &headerIt)) return error;
                    string avsNamespace;
                    if (!jsonUtils::retrieveValue(headerIt->value, "namespace", &avsNamespace)) return error;
                    if (avsNamespace != PLAYBACK_CONTROLLER_NAMESPACE) return error;
                    string avsName;
                    if (!jsonUtils::retrieveValue(headerIt->value, "name", &avsName)) return error;
                    return avsName;
                }
                class PlaybackControllerTest : public Test {
                public:
                    void SetUp() override;
                    void TearDown() override;
                protected:
                    void verifyButtonPressed(function<void()> func, const string& expectedMessageName, const string& expectedMessagePayloadName);
                    void verifyTogglePressed(function<void()> func, const string& expectedMessageName, const string& expectedMessagePayloadName,
                                             const string& expectedMessagePayloadAction);
                    void checkMessageRequestAndReleaseTrigger(shared_ptr<MessageRequest> messageRequest, bool sendException, const string& expected_name,
                                                              const string& expected_payload_name, const string& expected_payload_action);
                    void checkGetContextAndReleaseTrigger(shared_ptr<ContextRequesterInterface> contextRequester);
                    MessageRequestObserverInterface::Status m_messageStatus;
                    shared_ptr<StrictMock<MockContextManager>> m_mockContextManager;
                    shared_ptr<StrictMock<MockMessageSender>> m_mockMessageSender;
                    shared_ptr<PlaybackController> m_playbackController;
                    condition_variable m_messageTrigger;
                    condition_variable m_contextTrigger;
                    mutex m_mutex;
                };
                void PlaybackControllerTest::SetUp() {
                    m_mockContextManager = make_shared<StrictMock<MockContextManager>>();
                    m_mockMessageSender = make_shared<StrictMock<MockMessageSender>>();
                    m_messageStatus = MessageRequestObserverInterface::Status::SUCCESS;
                    m_playbackController = PlaybackController::create(m_mockContextManager, m_mockMessageSender);
                    ASSERT_NE(nullptr, m_playbackController);
                }
                void PlaybackControllerTest::TearDown() {
                    if (m_playbackController) m_playbackController->shutdown();
                }
                void PlaybackControllerTest::verifyButtonPressed(function<void()> func, const string& expectedMessageName,
                                                                 const string& expectedMessagePayloadName = "") {
                    unique_lock<mutex> exitLock(m_mutex);
                    EXPECT_CALL(*m_mockContextManager, getContext(_, _, _))
                        .WillOnce(WithArg<0>(Invoke([this](shared_ptr<ContextRequesterInterface> contextRequester)->ContextRequestToken {
                            checkGetContextAndReleaseTrigger(contextRequester);
                            return CONTEXT_REQUEST_TOKEN;
                        })));
                    func();
                    m_contextTrigger.wait_for(exitLock, TEST_RESULT_WAIT_PERIOD);
                    EXPECT_CALL(*m_mockMessageSender, sendMessage(_))
                        .WillOnce(Invoke([this, expectedMessageName, expectedMessagePayloadName](shared_ptr<MessageRequest> request) {
                            checkMessageRequestAndReleaseTrigger(request, false, expectedMessageName, expectedMessagePayloadName, "");
                        }));
                    m_playbackController->onContextAvailable(MOCK_CONTEXT);
                    m_messageTrigger.wait_for(exitLock, TEST_RESULT_WAIT_PERIOD);
                }
                void PlaybackControllerTest::verifyTogglePressed(function<void()> func, const string& expectedMessageName,
                                                                 const string& expectedMessagePayloadName,
                                                                 const string& expectedMessagePayloadAction = "") {
                    unique_lock<mutex> exitLock(m_mutex);
                    EXPECT_CALL(*m_mockContextManager, getContext(_, _, _))
                        .WillOnce(WithArg<0>(Invoke([this](shared_ptr<ContextRequesterInterface> contextRequester)->ContextRequestToken {
                            checkGetContextAndReleaseTrigger(contextRequester);
                            return CONTEXT_REQUEST_TOKEN;
                        })));
                    func();
                    m_contextTrigger.wait_for(exitLock, TEST_RESULT_WAIT_PERIOD);
                    EXPECT_CALL(*m_mockMessageSender, sendMessage(_))
                        .WillOnce(Invoke([this, expectedMessageName, expectedMessagePayloadName, expectedMessagePayloadAction](shared_ptr<MessageRequest> request) {
                            checkMessageRequestAndReleaseTrigger(request, false, expectedMessageName, expectedMessagePayloadName, expectedMessagePayloadAction);
                        }));
                    m_playbackController->onContextAvailable(MOCK_CONTEXT);
                    m_messageTrigger.wait_for(exitLock, TEST_RESULT_WAIT_PERIOD);
                }
                void PlaybackControllerTest::checkGetContextAndReleaseTrigger(shared_ptr<ContextRequesterInterface> contextRequester) {
                    m_contextTrigger.notify_one();
                    EXPECT_THAT(contextRequester, NotNull());
                }
                void PlaybackControllerTest::checkMessageRequestAndReleaseTrigger(shared_ptr<MessageRequest> messageRequest, bool sendException,
                                                                                  const string& expected_name,
                                                                                  const string& expected_payload_name = "",
                                                                                  const string& expected_payload_action = "") {
                    auto returnValue = checkMessageRequest(messageRequest, expected_payload_name, expected_payload_action);
                    m_messageTrigger.notify_one();
                    if (sendException) messageRequest->exceptionReceived(TEST_EXCEPTION_TEXT);
                    else messageRequest->sendCompleted(m_messageStatus);
                    EXPECT_EQ(returnValue, expected_name);
                }
                TEST_F(PlaybackControllerTest, test_createSuccessfully) {
                    ASSERT_NE(nullptr, PlaybackController::create(m_mockContextManager, m_mockMessageSender));
                }
                TEST_F(PlaybackControllerTest, test_createWithError) {
                    ASSERT_EQ(nullptr, PlaybackController::create(m_mockContextManager, nullptr));
                    ASSERT_EQ(nullptr, PlaybackController::create(nullptr, m_mockMessageSender));
                    ASSERT_EQ(nullptr, PlaybackController::create(nullptr, nullptr));
                }
                TEST_F(PlaybackControllerTest, test_playButtonPressed) {
                    PlaybackControllerTest::verifyButtonPressed(
                        [this]() { m_playbackController->onButtonPressed(PlaybackButton::PLAY); }, PLAYBACK_PLAY_NAME);
                }
                TEST_F(PlaybackControllerTest, test_pauseButtonPressed) {
                    ASSERT_NE(nullptr, m_playbackController);
                    PlaybackControllerTest::verifyButtonPressed([this]() { m_playbackController->onButtonPressed(PlaybackButton::PAUSE); }, PLAYBACK_PAUSE_NAME);
                }
                TEST_F(PlaybackControllerTest, test_nextButtonPressed) {
                    PlaybackControllerTest::verifyButtonPressed(
                        [this]() { m_playbackController->onButtonPressed(PlaybackButton::NEXT); }, PLAYBACK_NEXT_NAME);
                }
                TEST_F(PlaybackControllerTest, test_previousButtonPressed) {
                    PlaybackControllerTest::verifyButtonPressed([this]() { m_playbackController->onButtonPressed(PlaybackButton::PREVIOUS); }, PLAYBACK_PREVIOUS_NAME);
                }
                TEST_F(PlaybackControllerTest, test_skipForwardButtonPressed) {
                    PlaybackControllerTest::verifyButtonPressed([this]() { m_playbackController->onButtonPressed(PlaybackButton::SKIP_FORWARD); },
                                                                PLAYBACK_BUTTON_NAME, PLAYBACK_SKIPFORWARD_NAME);
                }
                TEST_F(PlaybackControllerTest, test_skipBackwardButtonPressed) {
                    PlaybackControllerTest::verifyButtonPressed([this]() { m_playbackController->onButtonPressed(PlaybackButton::SKIP_BACKWARD); },
                                                                PLAYBACK_BUTTON_NAME, PLAYBACK_SKIPBACKWARD_NAME);
                }
                TEST_F(PlaybackControllerTest, test_shuffleTogglePressed) {
                    PlaybackControllerTest::verifyTogglePressed([this]() { m_playbackController->onTogglePressed(PlaybackToggle::SHUFFLE, true); },
                                                                PLAYBACK_TOGGLE_NAME, PLAYBACK_SHUFFLE_NAME, PLAYBACK_SELECTED_NAME);
                    PlaybackControllerTest::verifyTogglePressed([this]() { m_playbackController->onTogglePressed(PlaybackToggle::SHUFFLE, false); },
                                                                PLAYBACK_TOGGLE_NAME, PLAYBACK_SHUFFLE_NAME, PLAYBACK_DESELECTED_NAME);
                }
                TEST_F(PlaybackControllerTest, test_loopTogglePressed) {
                    PlaybackControllerTest::verifyTogglePressed([this]() { m_playbackController->onTogglePressed(PlaybackToggle::LOOP, true); },
                                                                PLAYBACK_TOGGLE_NAME, PLAYBACK_LOOP_NAME, PLAYBACK_SELECTED_NAME);
                    PlaybackControllerTest::verifyTogglePressed([this]() { m_playbackController->onTogglePressed(PlaybackToggle::LOOP, false); },
                                                                PLAYBACK_TOGGLE_NAME, PLAYBACK_LOOP_NAME, PLAYBACK_DESELECTED_NAME);
                }
                TEST_F(PlaybackControllerTest, test_repeatTogglePressed) {
                    PlaybackControllerTest::verifyTogglePressed([this]() { m_playbackController->onTogglePressed(PlaybackToggle::REPEAT, true); },
                                                                PLAYBACK_TOGGLE_NAME, PLAYBACK_REPEAT_NAME, PLAYBACK_SELECTED_NAME);
                    PlaybackControllerTest::verifyTogglePressed([this]() { m_playbackController->onTogglePressed(PlaybackToggle::REPEAT, false); },
                                                                PLAYBACK_TOGGLE_NAME, PLAYBACK_REPEAT_NAME, PLAYBACK_DESELECTED_NAME);
                }
                TEST_F(PlaybackControllerTest, test_thumbsUpTogglePressed) {
                    PlaybackControllerTest::verifyTogglePressed([this]() { m_playbackController->onTogglePressed(PlaybackToggle::THUMBS_UP, true); },
                                                                PLAYBACK_TOGGLE_NAME, PLAYBACK_THUMBSUP_NAME, PLAYBACK_SELECTED_NAME);
                    PlaybackControllerTest::verifyTogglePressed([this]() { m_playbackController->onTogglePressed(PlaybackToggle::THUMBS_UP, false); },
                                                                PLAYBACK_TOGGLE_NAME, PLAYBACK_THUMBSUP_NAME, PLAYBACK_DESELECTED_NAME);
                }
                TEST_F(PlaybackControllerTest, test_thumbsDownTogglePressed) {
                    PlaybackControllerTest::verifyTogglePressed([this]() { m_playbackController->onTogglePressed(PlaybackToggle::THUMBS_DOWN, true); },
                                                                PLAYBACK_TOGGLE_NAME, PLAYBACK_THUMBSDOWN_NAME, PLAYBACK_SELECTED_NAME);
                    PlaybackControllerTest::verifyTogglePressed([this]() { m_playbackController->onTogglePressed(PlaybackToggle::THUMBS_DOWN, false); },
                                                                PLAYBACK_TOGGLE_NAME, PLAYBACK_THUMBSDOWN_NAME, PLAYBACK_DESELECTED_NAME);
                }
                TEST_F(PlaybackControllerTest, test_getContextFailure) {
                    std::unique_lock<std::mutex> exitLock(m_mutex);
                    EXPECT_CALL(*m_mockContextManager, getContext(_, _, _))
                        .WillOnce(WithArg<0>(Invoke([this](shared_ptr<ContextRequesterInterface> contextRequester) -> ContextRequestToken {
                            checkGetContextAndReleaseTrigger(contextRequester);
                            return CONTEXT_REQUEST_TOKEN;
                        })));
                    m_playbackController->onButtonPressed(PlaybackButton::PLAY);
                    m_playbackController->onButtonPressed(PlaybackButton::PAUSE);
                    m_contextTrigger.wait_for(exitLock, TEST_RESULT_WAIT_PERIOD);
                    EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(0);
                    EXPECT_CALL(*m_mockContextManager, getContext(_, _, _))
                        .WillOnce(WithArg<0>(Invoke([this](shared_ptr<ContextRequesterInterface> contextRequester) -> ContextRequestToken {
                            checkGetContextAndReleaseTrigger(contextRequester);
                            return CONTEXT_REQUEST_TOKEN;
                        })));
                    m_playbackController->onContextFailure(ContextRequestError::BUILD_CONTEXT_ERROR);
                    m_contextTrigger.wait_for(exitLock, TEST_RESULT_WAIT_PERIOD);
                    EXPECT_CALL(*m_mockMessageSender, sendMessage(_))
                        .WillOnce(Invoke([this](std::shared_ptr<MessageRequest> request) {
                            checkMessageRequestAndReleaseTrigger(request, false, PLAYBACK_PAUSE_NAME);
                        }));
                    m_playbackController->onContextAvailable(MOCK_CONTEXT);
                    m_messageTrigger.wait_for(exitLock, TEST_RESULT_WAIT_PERIOD);
                }
                TEST_F(PlaybackControllerTest, test_sendMessageFailure) {
                    unique_lock<mutex> exitLock(m_mutex);
                    m_messageStatus = MessageRequestObserverInterface::Status::INTERNAL_ERROR;
                    EXPECT_CALL(*m_mockContextManager, getContext(_, _, _))
                        .WillOnce(WithArg<0>(Invoke([this](shared_ptr<ContextRequesterInterface> contextRequester)->ContextRequestToken {
                            checkGetContextAndReleaseTrigger(contextRequester);
                            return CONTEXT_REQUEST_TOKEN;
                        })));
                    m_playbackController->onButtonPressed(PlaybackButton::NEXT);
                    m_contextTrigger.wait_for(exitLock, TEST_RESULT_WAIT_PERIOD);
                    EXPECT_CALL(*m_mockMessageSender, sendMessage(_))
                        .WillOnce(Invoke([this](shared_ptr<MessageRequest> request) {
                            checkMessageRequestAndReleaseTrigger(request, false, PLAYBACK_NEXT_NAME);
                        }));
                    m_playbackController->onContextAvailable(MOCK_CONTEXT);
                    m_messageTrigger.wait_for(exitLock, TEST_RESULT_WAIT_PERIOD);
                }
                TEST_F(PlaybackControllerTest, test_sendMessageException) {
                    unique_lock<mutex> exitLock(m_mutex);
                    m_messageStatus = MessageRequestObserverInterface::Status::INTERNAL_ERROR;
                    EXPECT_CALL(*m_mockContextManager, getContext(_, _, _))
                        .WillOnce(WithArg<0>(Invoke([this](shared_ptr<ContextRequesterInterface> contextRequester)->ContextRequestToken {
                            checkGetContextAndReleaseTrigger(contextRequester);
                            return CONTEXT_REQUEST_TOKEN;
                        })));
                    m_playbackController->onButtonPressed(PlaybackButton::NEXT);
                    m_contextTrigger.wait_for(exitLock, TEST_RESULT_WAIT_PERIOD);
                    EXPECT_CALL(*m_mockMessageSender, sendMessage(_))
                        .WillOnce(Invoke([this](shared_ptr<MessageRequest> request) {
                            checkMessageRequestAndReleaseTrigger(request, true, PLAYBACK_NEXT_NAME);
                        }));
                    m_playbackController->onContextAvailable(MOCK_CONTEXT);
                    m_messageTrigger.wait_for(exitLock, TEST_RESULT_WAIT_PERIOD);
                }
            }
        }
    }
}