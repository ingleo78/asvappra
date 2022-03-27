#include <future>
#include <memory>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <json/document.h>
#include <avs/attachment/MockAttachmentManager.h>
#include <sdkinterfaces/MediaPropertiesInterface.h>
#include <sdkinterfaces/MockDirectiveHandlerResult.h>
#include <sdkinterfaces/MockExceptionEncounteredSender.h>
#include <sdkinterfaces/MockFocusManager.h>
#include <sdkinterfaces/RenderPlayerInfoCardsObserverInterface.h>
#include <sdkinterfaces/RenderPlayerInfoCardsProviderInterface.h>
#include <sdkinterfaces/TemplateRuntimeObserverInterface.h>
#include <memory/Memory.h>
#include "TemplateRuntime.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace templateRuntime {
            namespace test {
                using namespace attachment;
                using namespace memory;
                using namespace rapidjson;
                using namespace testing;
                using namespace attachment::test;
                using namespace avsCommon::sdkInterfaces;
                using namespace sdkInterfaces::test;
                static milliseconds TIMEOUT(1000);
                static milliseconds TEMPLATE_TIMEOUT(5000);
                static milliseconds TEMPLATE_NOT_CLEAR_TIMEOUT(2500);
                static milliseconds PLAYER_FINISHED_TIMEOUT(5000);
                static const string NAMESPACE{"TemplateRuntime"};
                static const string UNKNOWN_DIRECTIVE{"Unknown"};
                static const NamespaceAndName TEMPLATE{NAMESPACE, "RenderTemplate"};
                static const NamespaceAndName PLAYER_INFO{NAMESPACE, "RenderPlayerInfo"};
                static const string MESSAGE_ID("messageId");
                static const string AUDIO_ITEM_ID("AudioItemId abcdefgh");
                static const string AUDIO_ITEM_ID_1("AudioItemId 12345678");
                static const string TEMPLATE_PAYLOAD = "{\"token\":\"TOKEN1\",\"type\":\"BodyTemplate1\",\"title\":{\"mainTitle\":"
                                                       "\"MAIN_TITLE\",\"subTitle\":\"SUB_TITLE\"}}";
                static const string PLAYERINFO_PAYLOAD = "{\"audioItemId\":\"" + AUDIO_ITEM_ID + "\",\"content\":{\"title\":\"TITLE\","
                                                         "\"header\":\"HEADER\"}}";
                static const string MALFORM_PLAYERINFO_PAYLOAD = "{\"audioItemId\"::::\"" + AUDIO_ITEM_ID + "\",\"content\":{{{{"
                                                                 "\"title\":\"TITLE\",\"header\":\"HEADER\"}}";
                class MockMediaPropertiesFetcher : public MediaPropertiesInterface {
                public:
                    MOCK_METHOD0(getAudioItemOffset, milliseconds());
                    MOCK_METHOD0(getAudioItemDuration, milliseconds());
                };
                class MockRenderInfoCardsPlayer : public RenderPlayerInfoCardsProviderInterface {
                public:
                    MOCK_METHOD1(
                        setObserver,
                        void(shared_ptr<RenderPlayerInfoCardsObserverInterface> observer));
                };
                class MockGui : public TemplateRuntimeObserverInterface {
                public:
                    //MOCK_METHOD2(renderTemplateCard, void(const string& jsonPayload, FocusState focusState));
                    MOCK_METHOD0(clearTemplateCard, void());
                    MOCK_METHOD3(renderPlayerInfoCard, void(const string& jsonPayload, AudioPlayerInfo audioPlayerInfo, FocusState focusState));
                    MOCK_METHOD0(clearPlayerInfoCard, void());
                };
                class TemplateRuntimeTest : public Test {
                public:
                    void SetUp() override;
                    void TearDown() override;
                    void wakeOnSetCompleted();
                    void wakeOnRenderTemplateCard();
                    void wakeOnRenderPlayerInfoCard();
                    void wakeOnClearTemplateCard();
                    void wakeOnClearPlayerInfoCard();
                    void wakeOnReleaseChannel();
                    TemplateRuntimeTest() : m_wakeSetCompletedPromise{}, m_wakeSetCompletedFuture{m_wakeSetCompletedPromise.get_future()},
                                            m_wakeRenderTemplateCardPromise{}, m_wakeRenderTemplateCardFuture{m_wakeRenderTemplateCardPromise.get_future()},
                                            m_wakeRenderPlayerInfoCardPromise{}, m_wakeRenderPlayerInfoCardFuture{m_wakeRenderPlayerInfoCardPromise.get_future()},
                                            m_wakeClearTemplateCardPromise{}, m_wakeClearTemplateCardFuture{m_wakeClearTemplateCardPromise.get_future()},
                                            m_wakeClearPlayerInfoCardPromise{}, m_wakeClearPlayerInfoCardFuture{m_wakeClearPlayerInfoCardPromise.get_future()},
                                            m_wakeReleaseChannelPromise{}, m_wakeReleaseChannelFuture{m_wakeReleaseChannelPromise.get_future()} {}
                protected:
                    promise<void> m_wakeSetCompletedPromise;
                    future<void> m_wakeSetCompletedFuture;
                    promise<void> m_wakeRenderTemplateCardPromise;
                    future<void> m_wakeRenderTemplateCardFuture;
                    promise<void> m_wakeRenderPlayerInfoCardPromise;
                    future<void> m_wakeRenderPlayerInfoCardFuture;
                    promise<void> m_wakeClearTemplateCardPromise;
                    future<void> m_wakeClearTemplateCardFuture;
                    promise<void> m_wakeClearPlayerInfoCardPromise;
                    future<void> m_wakeClearPlayerInfoCardFuture;
                    promise<void> m_wakeReleaseChannelPromise;
                    future<void> m_wakeReleaseChannelFuture;
                    shared_ptr<NiceMock<MockRenderInfoCardsPlayer>> m_mockRenderPlayerInfoCardsProvider;
                    shared_ptr<MockMediaPropertiesFetcher> m_mediaPropertiesFetcher;
                    shared_ptr<StrictMock<MockExceptionEncounteredSender>> m_mockExceptionSender;
                    unique_ptr<StrictMock<MockDirectiveHandlerResult>> m_mockDirectiveHandlerResult;
                    shared_ptr<MockFocusManager> m_mockFocusManager;
                    shared_ptr<StrictMock<MockGui>> m_mockGui;
                    shared_ptr<TemplateRuntime> m_templateRuntime;
                };
                void TemplateRuntimeTest::SetUp() {
                    m_mockExceptionSender = make_shared<StrictMock<MockExceptionEncounteredSender>>();
                    m_mockDirectiveHandlerResult = make_unique<StrictMock<MockDirectiveHandlerResult>>();
                    //m_mockFocusManager = make_shared<NiceMock<MockFocusManager>>();
                    m_mediaPropertiesFetcher = make_shared<NiceMock<MockMediaPropertiesFetcher>>();
                    m_mockRenderPlayerInfoCardsProvider = make_shared<NiceMock<MockRenderInfoCardsPlayer>>();
                    m_mockGui = make_shared<StrictMock<MockGui>>();
                    m_templateRuntime = TemplateRuntime::create({m_mockRenderPlayerInfoCardsProvider},
                                                                m_mockFocusManager, m_mockExceptionSender);
                    m_templateRuntime->addObserver(m_mockGui);
                    /*ON_CALL(*m_mockFocusManager, acquireChannel(_, _, _)).WillByDefault(InvokeWithoutArgs([this] {
                        m_templateRuntime->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                        return true;
                    }));
                    ON_CALL(*m_mockFocusManager, releaseChannel(_, _)).WillByDefault(InvokeWithoutArgs([this] {
                        auto releaseChannelSuccess = make_shared<promise<bool>>();
                        future<bool> returnValue = releaseChannelSuccess->get_future();
                        m_templateRuntime->onFocusChanged(FocusState::NONE, MixingBehavior::MUST_STOP);
                        releaseChannelSuccess->set_value(true);
                        return returnValue;
                    }));*/
                }
                void TemplateRuntimeTest::TearDown() {
                    if (m_templateRuntime) {
                        m_templateRuntime->shutdown();
                        m_templateRuntime.reset();
                    }
                }
                void TemplateRuntimeTest::wakeOnSetCompleted() {
                    m_wakeSetCompletedPromise.set_value();
                }
                void TemplateRuntimeTest::wakeOnRenderTemplateCard() {
                    m_wakeRenderTemplateCardPromise.set_value();
                }
                void TemplateRuntimeTest::wakeOnRenderPlayerInfoCard() {
                    m_wakeRenderPlayerInfoCardPromise.set_value();
                }
                void TemplateRuntimeTest::wakeOnClearTemplateCard() {
                    m_wakeClearTemplateCardPromise.set_value();
                }
                void TemplateRuntimeTest::wakeOnClearPlayerInfoCard() {
                    m_wakeClearPlayerInfoCardPromise.set_value();
                }
                void TemplateRuntimeTest::wakeOnReleaseChannel() {
                    m_wakeReleaseChannelPromise.set_value();
                }
                TEST_F(TemplateRuntimeTest, test_nullAudioPlayerInterface) {
                    auto templateRuntime = TemplateRuntime::create({nullptr}, m_mockFocusManager,
                                                                   m_mockExceptionSender);
                    ASSERT_EQ(templateRuntime, nullptr);
                }
                TEST_F(TemplateRuntimeTest, test_nullFocusManagerInterface) {
                    auto templateRuntime = TemplateRuntime::create({m_mockRenderPlayerInfoCardsProvider},
                                                                   nullptr, m_mockExceptionSender);
                    ASSERT_EQ(templateRuntime, nullptr);
                }
                TEST_F(TemplateRuntimeTest, test_nullExceptionSender) {
                    auto templateRuntime = TemplateRuntime::create({m_mockRenderPlayerInfoCardsProvider},
                                                                   m_mockFocusManager, nullptr);
                    ASSERT_EQ(templateRuntime, nullptr);
                }
                TEST_F(TemplateRuntimeTest, test_renderInfoCardsPlayersAddRemoveObserver) {
                    auto mockRenderInfoCardsProvider1 = make_shared<NiceMock<MockRenderInfoCardsPlayer>>();
                    auto mockRenderInfoCardsProvider2 = make_shared<NiceMock<MockRenderInfoCardsPlayer>>();
                    auto mockExceptionSender = make_shared<StrictMock<MockExceptionEncounteredSender>>();
                    //auto mockFocusManager = make_shared<NiceMock<MockFocusManager>>();
                    Expectation setObserver1 = EXPECT_CALL(*mockRenderInfoCardsProvider1, setObserver(NotNull())).Times(Exactly(1));
                    EXPECT_CALL(*mockRenderInfoCardsProvider1, setObserver(IsNull())).Times(Exactly(1)).After(setObserver1);
                    Expectation setObserver2 = EXPECT_CALL(*mockRenderInfoCardsProvider2, setObserver(NotNull())).Times(Exactly(1));
                    EXPECT_CALL(*mockRenderInfoCardsProvider2, setObserver(IsNull())).Times(Exactly(1)).After(setObserver2);
                    //auto templateRuntime = TemplateRuntime::create({mockRenderInfoCardsProvider1, mockRenderInfoCardsProvider2},
                    //                                               mockFocusManager, mockExceptionSender);
                    //templateRuntime->shutdown();
                }
                TEST_F(TemplateRuntimeTest, test_unknownDirective) {
                    auto attachmentManager = std::make_shared<StrictMock<MockAttachmentManager>>();
                    auto avsMessageHeader = std::make_shared<AVSMessageHeader>(NAMESPACE, UNKNOWN_DIRECTIVE, MESSAGE_ID);
                    std::shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, "", attachmentManager, "");
                    EXPECT_CALL(*m_mockExceptionSender, sendExceptionEncountered(_, _, _)).Times(Exactly(1));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setFailed(_)).Times(Exactly(1))
                        .WillOnce(InvokeWithoutArgs(this, &TemplateRuntimeTest::wakeOnSetCompleted));
                    m_templateRuntime->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
                    m_templateRuntime->CapabilityAgent::handleDirective(MESSAGE_ID);
                    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
                }
                TEST_F(TemplateRuntimeTest, testSlow_renderTemplateDirective) {
                    auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(TEMPLATE.nameSpace, TEMPLATE.name, MESSAGE_ID);
                    std::shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, TEMPLATE_PAYLOAD,
                                                                                   attachmentManager, "");
                    /*EXPECT_CALL(*m_mockGui, renderTemplateCard(TEMPLATE_PAYLOAD, _)).Times(Exactly(1))
                        .WillOnce(InvokeWithoutArgs(this, &TemplateRuntimeTest::wakeOnRenderTemplateCard));*/
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).Times(Exactly(1))
                        .WillOnce(InvokeWithoutArgs(this, &TemplateRuntimeTest::wakeOnSetCompleted));
                    EXPECT_CALL(*m_mockGui, clearTemplateCard()).Times(Exactly(1))
                        .WillOnce(InvokeWithoutArgs(this, &TemplateRuntimeTest::wakeOnClearTemplateCard));
                    m_templateRuntime->CapabilityAgent::preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    m_templateRuntime->CapabilityAgent::handleDirective(MESSAGE_ID);
                    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
                    m_wakeRenderTemplateCardFuture.wait_for(TIMEOUT);
                    m_templateRuntime->onDialogUXStateChanged(DialogUXState::IDLE);
                    m_wakeClearTemplateCardFuture.wait_for(TEMPLATE_TIMEOUT);
                }
                TEST_F(TemplateRuntimeTest, testRenderTemplateDirectiveWillNotClearCardAfterGoingToExpectingStateAfterGoingToIDLESlowTest) {
                    auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(TEMPLATE.nameSpace, TEMPLATE.name, MESSAGE_ID);
                    std::shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, TEMPLATE_PAYLOAD,
                                                                                   attachmentManager, "");
                    /*EXPECT_CALL(*m_mockGui, renderTemplateCard(TEMPLATE_PAYLOAD, _)).Times(Exactly(1))
                        .WillOnce(InvokeWithoutArgs(this, &TemplateRuntimeTest::wakeOnRenderTemplateCard));*/
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).Times(Exactly(1))
                        .WillOnce(InvokeWithoutArgs(this, &TemplateRuntimeTest::wakeOnSetCompleted));
                    EXPECT_CALL(*m_mockGui, clearTemplateCard()).Times(Exactly(0));
                    m_templateRuntime->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
                    m_templateRuntime->CapabilityAgent::handleDirective(MESSAGE_ID);
                    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
                    m_wakeRenderTemplateCardFuture.wait_for(TIMEOUT);
                    m_templateRuntime->onDialogUXStateChanged(DialogUXState::IDLE);
                    m_templateRuntime->onDialogUXStateChanged(DialogUXState::EXPECTING);
                    EXPECT_EQ(m_wakeClearTemplateCardFuture.wait_for(TEMPLATE_NOT_CLEAR_TIMEOUT), future_status::timeout);
                    m_templateRuntime->onDialogUXStateChanged(DialogUXState::IDLE);
                    m_templateRuntime->onDialogUXStateChanged(DialogUXState::SPEAKING);
                    EXPECT_EQ(m_wakeClearTemplateCardFuture.wait_for(TEMPLATE_NOT_CLEAR_TIMEOUT), future_status::timeout);
                }
                TEST_F(TemplateRuntimeTest, test_handleDirectiveImmediately) {
                    auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(TEMPLATE.nameSpace, TEMPLATE.name, MESSAGE_ID);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, TEMPLATE_PAYLOAD,
                                                                              attachmentManager, "");
                    /*EXPECT_CALL(*m_mockGui, renderTemplateCard(TEMPLATE_PAYLOAD, _)).Times(Exactly(1))
                        .WillOnce(InvokeWithoutArgs(this, &TemplateRuntimeTest::wakeOnRenderTemplateCard));*/
                    m_templateRuntime->handleDirectiveImmediately(directive);
                    m_wakeRenderTemplateCardFuture.wait_for(TIMEOUT);
                }
                TEST_F(TemplateRuntimeTest, testSlow_renderPlayerInfoDirectiveBefore) {
                    auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(PLAYER_INFO.nameSpace, PLAYER_INFO.name, MESSAGE_ID);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, PLAYERINFO_PAYLOAD,
                                                                              attachmentManager, "");
                    InSequence s;
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).Times(Exactly(1))
                        .WillOnce(InvokeWithoutArgs(this, &TemplateRuntimeTest::wakeOnSetCompleted));
                    //EXPECT_CALL(*m_mockGui, renderTemplateCard(_, _)).Times(Exactly(0));
                    EXPECT_CALL(*m_mockGui, renderPlayerInfoCard(_, _, _)).Times(Exactly(0));
                    m_templateRuntime->CapabilityAgent::preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    m_templateRuntime->CapabilityAgent::handleDirective(MESSAGE_ID);
                    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
                    EXPECT_CALL(*m_mockGui, renderPlayerInfoCard(PLAYERINFO_PAYLOAD, _, _)).Times(Exactly(2))
                        .WillOnce(InvokeWithoutArgs(this, &TemplateRuntimeTest::wakeOnRenderPlayerInfoCard))
                        .WillOnce(InvokeWithoutArgs([] {}));
                    RenderPlayerInfoCardsObserverInterface::Context context;
                    context.mediaProperties = m_mediaPropertiesFetcher;
                    context.audioItemId = AUDIO_ITEM_ID;
                    context.offset = TIMEOUT;
                    m_templateRuntime->onRenderPlayerCardsInfoChanged(PlayerActivity::PLAYING, context);
                    m_wakeRenderPlayerInfoCardFuture.wait_for(TIMEOUT);
                    EXPECT_CALL(*m_mockGui, clearPlayerInfoCard()).Times(Exactly(1))
                        .WillOnce(InvokeWithoutArgs(this, &TemplateRuntimeTest::wakeOnClearPlayerInfoCard));
                    m_templateRuntime->onRenderPlayerCardsInfoChanged(PlayerActivity::FINISHED, context);
                    m_wakeClearPlayerInfoCardFuture.wait_for(PLAYER_FINISHED_TIMEOUT);
                }
                TEST_F(TemplateRuntimeTest, test_renderPlayerInfoDirectiveAfter) {
                    auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(PLAYER_INFO.nameSpace, PLAYER_INFO.name, MESSAGE_ID);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, PLAYERINFO_PAYLOAD,
                                                                              attachmentManager, "");
                    EXPECT_CALL(*m_mockGui, renderPlayerInfoCard(PLAYERINFO_PAYLOAD, _, _)).Times(Exactly(1))
                        .WillOnce(InvokeWithoutArgs(this, &TemplateRuntimeTest::wakeOnRenderPlayerInfoCard));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).Times(Exactly(1))
                        .WillOnce(InvokeWithoutArgs(this, &TemplateRuntimeTest::wakeOnSetCompleted));
                    RenderPlayerInfoCardsObserverInterface::Context context;
                    context.mediaProperties = m_mediaPropertiesFetcher;
                    context.audioItemId = AUDIO_ITEM_ID;
                    context.offset = TIMEOUT;
                    m_templateRuntime->onRenderPlayerCardsInfoChanged(PlayerActivity::PLAYING, context);
                    m_templateRuntime->CapabilityAgent::preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    m_templateRuntime->CapabilityAgent::handleDirective(MESSAGE_ID);
                    m_wakeRenderPlayerInfoCardFuture.wait_for(TIMEOUT);
                    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
                }
                TEST_F(TemplateRuntimeTest, test_renderPlayerInfoDirectiveWithoutAudioItemId) {
                    auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(PLAYER_INFO.nameSpace, PLAYER_INFO.name, MESSAGE_ID);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, TEMPLATE_PAYLOAD,
                                                                              attachmentManager, "");
                    EXPECT_CALL(*m_mockExceptionSender, sendExceptionEncountered(_, _, _)).Times(Exactly(1));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setFailed(_)).Times(Exactly(1))
                        .WillOnce(InvokeWithoutArgs(this, &TemplateRuntimeTest::wakeOnSetCompleted));
                    m_templateRuntime->CapabilityAgent::preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    m_templateRuntime->CapabilityAgent::handleDirective(MESSAGE_ID);
                    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
                }
                TEST_F(TemplateRuntimeTest, test_malformedRenderPlayerInfoDirective) {
                    auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(PLAYER_INFO.nameSpace, PLAYER_INFO.name, MESSAGE_ID);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, MALFORM_PLAYERINFO_PAYLOAD,
                                                                              attachmentManager, "");
                    EXPECT_CALL(*m_mockExceptionSender, sendExceptionEncountered(_, _, _)).Times(Exactly(1));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setFailed(_)).Times(Exactly(1))
                        .WillOnce(InvokeWithoutArgs(this, &TemplateRuntimeTest::wakeOnSetCompleted));
                    m_templateRuntime->CapabilityAgent::preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    m_templateRuntime->CapabilityAgent::handleDirective(MESSAGE_ID);
                    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
                }
                TEST_F(TemplateRuntimeTest, test_renderPlayerInfoDirectiveDifferentAudioItemId) {
                    auto attachmentManager = std::make_shared<StrictMock<MockAttachmentManager>>();
                    auto avsMessageHeader = std::make_shared<AVSMessageHeader>(PLAYER_INFO.nameSpace, PLAYER_INFO.name, MESSAGE_ID);
                    std::shared_ptr<AVSDirective> directive =AVSDirective::create("", avsMessageHeader, PLAYERINFO_PAYLOAD, attachmentManager, "");
                    EXPECT_CALL(*m_mockGui, renderPlayerInfoCard(PLAYERINFO_PAYLOAD, _, _)).Times(Exactly(0));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).Times(Exactly(1))
                        .WillOnce(InvokeWithoutArgs(this, &TemplateRuntimeTest::wakeOnSetCompleted));
                    RenderPlayerInfoCardsObserverInterface::Context context;
                    context.mediaProperties = m_mediaPropertiesFetcher;
                    context.audioItemId = AUDIO_ITEM_ID_1;
                    context.offset = TIMEOUT;
                    m_templateRuntime->onRenderPlayerCardsInfoChanged(avsCommon::avs::PlayerActivity::PLAYING, context);
                    m_templateRuntime->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
                    m_templateRuntime->CapabilityAgent::handleDirective(MESSAGE_ID);
                    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
                    EXPECT_CALL(*m_mockGui, renderPlayerInfoCard(PLAYERINFO_PAYLOAD, _, _)).Times(Exactly(1))
                        .WillOnce(InvokeWithoutArgs(this, &TemplateRuntimeTest::wakeOnRenderPlayerInfoCard));
                    context.audioItemId = AUDIO_ITEM_ID;
                    m_templateRuntime->onRenderPlayerCardsInfoChanged(avsCommon::avs::PlayerActivity::PLAYING, context);
                    m_wakeRenderPlayerInfoCardFuture.wait_for(TIMEOUT);
                }
                TEST_F(TemplateRuntimeTest, test_renderPlayerInfoDirectiveWithTwoProviders) {
                    auto anotherMediaPropertiesFetcher = make_shared<NiceMock<MockMediaPropertiesFetcher>>();
                    auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(PLAYER_INFO.nameSpace, PLAYER_INFO.name, MESSAGE_ID);
                    std::shared_ptr<AVSDirective> directive =AVSDirective::create("", avsMessageHeader, PLAYERINFO_PAYLOAD,
                                                                                  attachmentManager, "");
                    EXPECT_CALL(*m_mockGui, renderPlayerInfoCard(PLAYERINFO_PAYLOAD, _, _)).Times(Exactly(1))
                        .WillOnce(InvokeWithoutArgs(this, &TemplateRuntimeTest::wakeOnRenderPlayerInfoCard));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).Times(Exactly(1))
                        .WillOnce(InvokeWithoutArgs(this, &TemplateRuntimeTest::wakeOnSetCompleted));
                    EXPECT_CALL(*anotherMediaPropertiesFetcher, getAudioItemOffset()).Times(Exactly(1))
                        .WillOnce(Return(milliseconds::zero()));
                    EXPECT_CALL(*m_mediaPropertiesFetcher, getAudioItemOffset()).Times(Exactly(0));
                    RenderPlayerInfoCardsObserverInterface::Context context;
                    context.mediaProperties = m_mediaPropertiesFetcher;
                    context.audioItemId = AUDIO_ITEM_ID_1;
                    context.offset = TIMEOUT;
                    m_templateRuntime->onRenderPlayerCardsInfoChanged(PlayerActivity::PLAYING, context);
                    RenderPlayerInfoCardsObserverInterface::Context context1;
                    context1.mediaProperties = anotherMediaPropertiesFetcher;
                    context1.audioItemId = AUDIO_ITEM_ID;
                    context1.offset = TIMEOUT;
                    m_templateRuntime->onRenderPlayerCardsInfoChanged(PlayerActivity::PLAYING, context1);
                    m_templateRuntime->CapabilityAgent::preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    m_templateRuntime->CapabilityAgent::handleDirective(MESSAGE_ID);
                    m_wakeRenderPlayerInfoCardFuture.wait_for(TIMEOUT);
                    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
                }
                TEST_F(TemplateRuntimeTest, test_renderPlayerInfoDirectiveAudioStateUpdate) {
                    auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(PLAYER_INFO.nameSpace, PLAYER_INFO.name, MESSAGE_ID);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, PLAYERINFO_PAYLOAD,
                                                                              attachmentManager, "");
                    InSequence s;
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).Times(Exactly(1))
                        .WillOnce(InvokeWithoutArgs(this, &TemplateRuntimeTest::wakeOnSetCompleted));
                    m_templateRuntime->CapabilityAgent::preHandleDirective(directive,move(m_mockDirectiveHandlerResult));
                    m_templateRuntime->CapabilityAgent::handleDirective(MESSAGE_ID);
                    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
                    RenderPlayerInfoCardsObserverInterface::Context context;
                    context.mediaProperties = m_mediaPropertiesFetcher;
                    context.audioItemId = AUDIO_ITEM_ID;
                    promise<void> wakePlayPromise;
                    future<void> wakePlayFuture = wakePlayPromise.get_future();
                    context.offset = milliseconds(100);
                    EXPECT_CALL(*m_mockGui, renderPlayerInfoCard(PLAYERINFO_PAYLOAD, _, _)).Times(Exactly(1))
                        .WillOnce(Invoke([&wakePlayPromise, context](const string& jsonPayload, AudioPlayerInfo audioPlayerInfo, FocusState focusState) {
                            EXPECT_EQ(audioPlayerInfo.audioPlayerState,PlayerActivity::PLAYING);
                            EXPECT_EQ(audioPlayerInfo.offset, context.offset);
                            wakePlayPromise.set_value();
                        }));
                    m_templateRuntime->onRenderPlayerCardsInfoChanged(PlayerActivity::PLAYING, context);
                    wakePlayFuture.wait_for(TIMEOUT);
                    promise<void> wakePausePromise;
                    future<void> wakePauseFuture = wakePausePromise.get_future();
                    context.offset = milliseconds(200);
                    EXPECT_CALL(*m_mockGui, renderPlayerInfoCard(PLAYERINFO_PAYLOAD, _, _)).Times(Exactly(1))
                        .WillOnce(Invoke([&wakePausePromise, context](const string& jsonPayload, AudioPlayerInfo audioPlayerInfo, FocusState focusState) {
                            EXPECT_EQ(audioPlayerInfo.audioPlayerState,PlayerActivity::PAUSED);
                            EXPECT_EQ(audioPlayerInfo.offset, context.offset);
                            wakePausePromise.set_value();
                        }));
                    m_templateRuntime->onRenderPlayerCardsInfoChanged(PlayerActivity::PAUSED, context);
                    wakePauseFuture.wait_for(TIMEOUT);
                    promise<void> wakeStopPromise;
                    future<void> wakeStopFuture = wakeStopPromise.get_future();
                    context.offset = milliseconds(300);
                    EXPECT_CALL(*m_mockGui, renderPlayerInfoCard(PLAYERINFO_PAYLOAD, _, _)).Times(Exactly(1))
                        .WillOnce(Invoke([&wakeStopPromise, context](const string& jsonPayload, AudioPlayerInfo audioPlayerInfo, FocusState focusState) {
                            EXPECT_EQ(audioPlayerInfo.audioPlayerState, PlayerActivity::STOPPED);
                            EXPECT_EQ(audioPlayerInfo.offset, context.offset);
                            wakeStopPromise.set_value();
                        }));
                    m_templateRuntime->onRenderPlayerCardsInfoChanged(PlayerActivity::STOPPED, context);
                    wakeStopFuture.wait_for(TIMEOUT);
                    promise<void> wakeFinishPromise;
                    future<void> wakeFinishFuture = wakeFinishPromise.get_future();
                    context.offset = milliseconds(400);
                    EXPECT_CALL(*m_mockGui, renderPlayerInfoCard(PLAYERINFO_PAYLOAD, _, _)).Times(Exactly(1))
                        .WillOnce(Invoke([&wakeFinishPromise, context](const string& jsonPayload, AudioPlayerInfo audioPlayerInfo, FocusState focusState) {
                            EXPECT_EQ(audioPlayerInfo.audioPlayerState, PlayerActivity::FINISHED);
                            EXPECT_EQ(audioPlayerInfo.offset, context.offset);
                            wakeFinishPromise.set_value();
                        }));
                    m_templateRuntime->onRenderPlayerCardsInfoChanged(PlayerActivity::FINISHED, context);
                    wakeFinishFuture.wait_for(TIMEOUT);
                }
                TEST_F(TemplateRuntimeTest, test_focusNone) {
                    auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(TEMPLATE.nameSpace, TEMPLATE.name, MESSAGE_ID);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, TEMPLATE_PAYLOAD,
                                                                              attachmentManager, "");
                    /*EXPECT_CALL(*m_mockGui, renderTemplateCard(TEMPLATE_PAYLOAD, _)).Times(Exactly(1))
                        .WillOnce(InvokeWithoutArgs(this, &TemplateRuntimeTest::wakeOnRenderTemplateCard));*/
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).Times(Exactly(1))
                        .WillOnce(InvokeWithoutArgs(this, &TemplateRuntimeTest::wakeOnSetCompleted));
                    EXPECT_CALL(*m_mockGui, clearTemplateCard()).Times(Exactly(1))
                        .WillOnce(InvokeWithoutArgs(this, &TemplateRuntimeTest::wakeOnClearTemplateCard));
                    m_templateRuntime->CapabilityAgent::preHandleDirective(directive, move(m_mockDirectiveHandlerResult));
                    m_templateRuntime->CapabilityAgent::handleDirective(MESSAGE_ID);
                    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
                    m_wakeRenderTemplateCardFuture.wait_for(TIMEOUT);
                    m_templateRuntime->onFocusChanged(FocusState::NONE, MixingBehavior::MUST_STOP);
                    m_wakeClearTemplateCardFuture.wait_for(TIMEOUT);
                }
                TEST_F(TemplateRuntimeTest, test_displayCardCleared) {
                    auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(TEMPLATE.nameSpace, TEMPLATE.name, MESSAGE_ID);
                    std::shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, TEMPLATE_PAYLOAD,
                                                                                   attachmentManager, "");
                    /*EXPECT_CALL(*m_mockGui, renderTemplateCard(TEMPLATE_PAYLOAD, _)).Times(Exactly(1))
                        .WillOnce(InvokeWithoutArgs(this, &TemplateRuntimeTest::wakeOnRenderTemplateCard));*/
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).Times(Exactly(1))
                        .WillOnce(InvokeWithoutArgs(this, &TemplateRuntimeTest::wakeOnSetCompleted));
                    EXPECT_CALL(*m_mockGui, clearTemplateCard()).Times(Exactly(0));
                    /*EXPECT_CALL(*m_mockFocusManager, releaseChannel(_, _)).Times(Exactly(1)).WillOnce(InvokeWithoutArgs([this] {
                        auto releaseChannelSuccess = make_shared<std::promise<bool>>();
                        future<bool> returnValue = releaseChannelSuccess->get_future();
                        m_templateRuntime->onFocusChanged(FocusState::NONE, MixingBehavior::MUST_STOP);
                        releaseChannelSuccess->set_value(true);
                        wakeOnReleaseChannel();
                        return returnValue;
                    }));*/
                    m_templateRuntime->CapabilityAgent::preHandleDirective(directive,move(m_mockDirectiveHandlerResult));
                    m_templateRuntime->CapabilityAgent::handleDirective(MESSAGE_ID);
                    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
                    m_wakeRenderTemplateCardFuture.wait_for(TIMEOUT);
                    m_templateRuntime->displayCardCleared();
                    m_wakeReleaseChannelFuture.wait_for(TIMEOUT);
                }
                TEST_F(TemplateRuntimeTest, test_reacquireChannel) {
                    auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                    auto avsMessageHeader = make_shared<AVSMessageHeader>(PLAYER_INFO.nameSpace, PLAYER_INFO.name, MESSAGE_ID);
                    shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, PLAYERINFO_PAYLOAD,
                                                                              attachmentManager, "");
                    EXPECT_CALL(*m_mockGui, renderPlayerInfoCard(PLAYERINFO_PAYLOAD, _, _)).Times(Exactly(1))
                        .WillOnce(InvokeWithoutArgs(this, &TemplateRuntimeTest::wakeOnRenderPlayerInfoCard));
                    RenderPlayerInfoCardsObserverInterface::Context context;
                    context.mediaProperties = m_mediaPropertiesFetcher;
                    context.audioItemId = AUDIO_ITEM_ID;
                    context.offset = TIMEOUT;
                    m_templateRuntime->onRenderPlayerCardsInfoChanged(PlayerActivity::PLAYING, context);
                    m_templateRuntime->handleDirectiveImmediately(directive);
                    m_wakeRenderPlayerInfoCardFuture.wait_for(TIMEOUT);
                    /*EXPECT_CALL(*m_mockFocusManager, releaseChannel(_, _)).Times(Exactly(1)).WillOnce(InvokeWithoutArgs([this] {
                        auto releaseChannelSuccess = make_shared<promise<bool>>();
                        future<bool> returnValue = releaseChannelSuccess->get_future();
                        releaseChannelSuccess->set_value(true);
                        wakeOnReleaseChannel();
                        return returnValue;
                    }));*/
                    m_templateRuntime->displayCardCleared();
                    m_wakeReleaseChannelFuture.wait_for(TIMEOUT);
                    auto avsMessageHeader1 = make_shared<AVSMessageHeader>(TEMPLATE.nameSpace, TEMPLATE.name, MESSAGE_ID);
                    shared_ptr<AVSDirective> directive1 = AVSDirective::create("", avsMessageHeader1, TEMPLATE_PAYLOAD,
                                                                               attachmentManager, "");
                    /*EXPECT_CALL(*m_mockGui, renderTemplateCard(TEMPLATE_PAYLOAD, _)).Times(Exactly(1))
                        .WillOnce(InvokeWithoutArgs(this, &TemplateRuntimeTest::wakeOnRenderTemplateCard));*/
                    m_templateRuntime->handleDirectiveImmediately(directive1);
                    m_templateRuntime->onFocusChanged(FocusState::NONE, MixingBehavior::MUST_STOP);
                    m_wakeRenderTemplateCardFuture.wait_for(TIMEOUT);
                }
                TEST_F(TemplateRuntimeTest, testTimer_RenderPlayerInfoAfterPlayerActivityChanged) {
                    const std::string messageId1{"messageId1"};
                    auto attachmentManager = make_shared<StrictMock<MockAttachmentManager>>();
                    auto avsMessageHeader1 = make_shared<AVSMessageHeader>(PLAYER_INFO.nameSpace, PLAYER_INFO.name, messageId1);
                    shared_ptr<AVSDirective> directive1 = AVSDirective::create("", avsMessageHeader1, PLAYERINFO_PAYLOAD,
                                                                               attachmentManager, "");
                    RenderPlayerInfoCardsObserverInterface::Context context;
                    context.mediaProperties = m_mediaPropertiesFetcher;
                    context.audioItemId = AUDIO_ITEM_ID;
                    m_templateRuntime->onRenderPlayerCardsInfoChanged(PlayerActivity::PLAYING, context);
                    InSequence s;
                    //EXPECT_CALL(*m_mockFocusManager, acquireChannel(_, _, _)).WillOnce(Return(true));
                    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).Times(Exactly(1))
                        .WillOnce(InvokeWithoutArgs(this, &TemplateRuntimeTest::wakeOnSetCompleted));
                    m_templateRuntime->CapabilityAgent::preHandleDirective(directive1, std::move(m_mockDirectiveHandlerResult));
                    m_templateRuntime->CapabilityAgent::handleDirective(messageId1);
                    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
                    promise<void> wakePlayPromise;
                    future<void> wakePlayFuture = wakePlayPromise.get_future();
                    context.offset = milliseconds(100);
                    EXPECT_CALL(*m_mockGui, renderPlayerInfoCard(PLAYERINFO_PAYLOAD, _, _)).Times(0);
                    /*EXPECT_CALL(*m_mockFocusManager, releaseChannel(_, _)).Times(Exactly(1)).WillOnce(InvokeWithoutArgs([this] {
                        auto releaseChannelSuccess = make_shared<promise<bool>>();
                        future<bool> returnValue = releaseChannelSuccess->get_future();
                        m_templateRuntime->onFocusChanged<(FocusState::NONE, MixingBehavior::MUST_STOP);
                        releaseChannelSuccess->set_value(true);
                        wakeOnReleaseChannel();
                        return returnValue;
                    }));*/
                    const string messageId2{"messageId2"};
                    auto avsMessageHeader2 = make_shared<AVSMessageHeader>(PLAYER_INFO.nameSpace, PLAYER_INFO.name, messageId2);
                    auto mockDirectiveHandlerResult1 = make_unique<StrictMock<MockDirectiveHandlerResult>>();
                    shared_ptr<AVSDirective> directive2 = AVSDirective::create("", avsMessageHeader1, PLAYERINFO_PAYLOAD,
                                                                               attachmentManager, "");
                    m_templateRuntime->CapabilityAgent::preHandleDirective(directive2, move(m_mockDirectiveHandlerResult));
                    m_templateRuntime->CapabilityAgent::handleDirective(messageId2);
                    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
                    m_wakeRenderTemplateCardFuture.wait_for(TIMEOUT);
                    m_templateRuntime->displayCardCleared();
                    m_wakeReleaseChannelFuture.wait_for(TIMEOUT);
                    context.audioItemId = AUDIO_ITEM_ID_1;
                    m_templateRuntime->onRenderPlayerCardsInfoChanged(PlayerActivity::PLAYING, context);
                    m_templateRuntime->onFocusChanged(FocusState::FOREGROUND, MixingBehavior::PRIMARY);
                    m_templateRuntime->displayCardCleared();
                    m_wakeReleaseChannelFuture.wait_for(TIMEOUT);
                }
            }
        }
    }
}