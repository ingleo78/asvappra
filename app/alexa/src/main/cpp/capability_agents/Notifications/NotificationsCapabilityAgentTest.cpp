#include <chrono>
#include <iostream>
#include <queue>
#include <sstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <avs/attachment/MockAttachmentManager.h>
#include <avs/IndicatorState.h>
#include <avs/Initialization/AlexaClientSDKInit.h>
#include <sdkinterfaces/Audio/NotificationsAudioFactoryInterface.h>
#include <sdkinterfaces/MockContextManager.h>
#include <sdkinterfaces/MockDirectiveHandlerResult.h>
#include <sdkinterfaces/MockDirectiveSequencer.h>
#include <sdkinterfaces/MockExceptionEncounteredSender.h>
#include <json/JSONUtils.h>
#include <logger/ConsoleLogger.h>
#include <metrics/MockMetricRecorder.h>
#include <registration_manager/CustomerDataManager.h>
#include <acsdk_alerts_interfaces/NotificationRendererInterface.h>
#include <acsdk_alerts_interfaces/NotificationRendererObserverInterface.h>
#include "NotificationIndicator.h"
#include "NotificationsCapabilityAgent.h"

namespace alexaClientSDK {
    namespace acsdkNotifications {
        namespace test {
            using namespace avs;
            using namespace attachment;
            using namespace initialization;
            using namespace testing;
            using namespace attachment::test;
            using namespace metrics::test;
            using namespace sdkInterfaces::test;
            static milliseconds MY_WAIT_TIMEOUT(1000);
            static milliseconds RENDER_TIME(10);
            static const string NAMESPACE_NOTIFICATIONS("Notifications");
            static const string NAME_SET_INDICATOR("SetIndicator");
            static const string NAME_CLEAR_INDICATOR("ClearIndicator");
            static const NamespaceAndName NAMESPACE_AND_NAME_INDICATOR_STATE{NAMESPACE_NOTIFICATIONS, "IndicatorState"};
            static const string MESSAGE_ID_TEST("MessageId_Test");
            static const string MESSAGE_ID_TEST2("MessageId_Test2");
            static const string CONTEXT_ID_TEST("ContextId_Test");
            static const string ASSET_ID1("assetId1");
            static const string ASSET_ID2("assetId2");
            static const string ASSET_URL1("assetUrl1");
            static const string ASSET_URL2("assetUrl2");
            static const string DEFAULT_NOTIFICATION_AUDIO{"default notification audio"};
            static const string NOTIFICATIONS_CONFIG_JSON = "{\"notifications\":{\"databaseFilePath\":\"notificationsUnitTest.db\"}}";
            static const string TAG("NotificationsCapabilityAgentTest");
            #define LX(event) LogEntry(TAG, event)
            class TestNotificationsAudioFactory : public NotificationsAudioFactoryInterface {
            public:
                function<pair<unique_ptr<istream>, const MediaType>()> notificationDefault() const override;
            private:
                static pair<unique_ptr<istream>, const MediaType> defaultNotification() {
                    return pair<unique_ptr<istream>, const MediaType>(unique_ptr<istream>(new stringstream(DEFAULT_NOTIFICATION_AUDIO)),
                                                                  MediaType::MPEG);
                }
            };
            function<pair<unique_ptr<istream>, const MediaType>()>
            TestNotificationsAudioFactory::notificationDefault() const {
                return defaultNotification;
            }
            class TestNotificationsObserver : public NotificationsObserverInterface {
            public:
                TestNotificationsObserver();
                bool waitFor(IndicatorState state, milliseconds timeout);
                bool waitFor(int count, milliseconds timeout);
                void onSetIndicator(IndicatorState state) override;
                void onNotificationReceived() override;
            private:
                IndicatorState m_indicatorState;
                int m_indicationCount;
                mutex m_mutex;
                condition_variable m_conditionVariable;
            };
            TestNotificationsObserver::TestNotificationsObserver() : m_indicatorState{IndicatorState::OFF}, m_indicationCount{0} {}
            bool TestNotificationsObserver::waitFor(IndicatorState state, milliseconds timeout) {
                unique_lock<mutex> lock(m_mutex);
                return m_conditionVariable.wait_for(lock, timeout, [this, state] { return m_indicatorState == state; });
            }
            bool TestNotificationsObserver::waitFor(int count, milliseconds timeout) {
                unique_lock<mutex> lock(m_mutex);
                return m_conditionVariable.wait_for(lock, timeout, [this, count] { return m_indicationCount == count; });
            }
            void TestNotificationsObserver::onSetIndicator(IndicatorState state) {
                ACSDK_INFO(LX("onSetIndicator").d("indicatorState", indicatorStateToInt(state)));
                lock_guard<mutex> lock(m_mutex);
                m_indicatorState = state;
                m_conditionVariable.notify_all();
            }
            void TestNotificationsObserver::onNotificationReceived() {
                ACSDK_INFO(LX("onNotificationReceived"));
                lock_guard<mutex> lock(m_mutex);
                ++m_indicationCount;
                m_conditionVariable.notify_all();
            }
            class TestNotificationsStorage : public NotificationsStorageInterface {
            public:
                bool createDatabase() override;
                bool open() override;
                void close() override;
                bool enqueue(const NotificationIndicator& notificationIndicator) override;
                bool dequeue() override;
                bool peek(NotificationIndicator* notificationIndicator) override;
                bool setIndicatorState(IndicatorState state) override;
                bool getIndicatorState(IndicatorState* state) override;
                bool checkForEmptyQueue(bool* empty) override;
                bool clearNotificationIndicators() override;
                bool getQueueSize(int* size) override;
                bool waitForQueueSizeToBe(size_t size, milliseconds timeout = MY_WAIT_TIMEOUT);
            private:
                queue<NotificationIndicator> m_notificationQueue;
                IndicatorState m_indicatorState;
                mutex m_mutex;
                condition_variable m_conditionVariable;
            };
            bool TestNotificationsStorage::createDatabase() {
                if (!setIndicatorState(IndicatorState::OFF)) {
                    ACSDK_ERROR(LX("createTestDatabaseFailed").d("reason", "failed to set default indicator state"));
                    return false;
                }
                return true;
            }
            bool TestNotificationsStorage::open() {
                return true;
            }
            void TestNotificationsStorage::close() {}
            bool TestNotificationsStorage::enqueue(const NotificationIndicator& notificationIndicator) {
                m_notificationQueue.push(notificationIndicator);
                return true;
            }
            bool TestNotificationsStorage::dequeue() {
                if (m_notificationQueue.empty()) return false;
                m_notificationQueue.pop();
                return true;
            }
            bool TestNotificationsStorage::peek(NotificationIndicator* notificationIndicator) {
                if (m_notificationQueue.empty() || !notificationIndicator) return false;
                *notificationIndicator = m_notificationQueue.front();
                return true;
            }
            bool TestNotificationsStorage::setIndicatorState(IndicatorState state) {
                m_indicatorState = state;
                return true;
            }
            bool TestNotificationsStorage::getIndicatorState(IndicatorState* state) {
                if (!state) return false;
                *state = m_indicatorState;
                return true;
            }
            bool TestNotificationsStorage::checkForEmptyQueue(bool* empty) {
                *empty = m_notificationQueue.empty();
                return true;
            }
            bool TestNotificationsStorage::clearNotificationIndicators() {
                m_notificationQueue = queue<NotificationIndicator>();
                return true;
            }
            bool TestNotificationsStorage::getQueueSize(int* size) {
                if (!size) return false;
                *size = m_notificationQueue.size();
                return true;
            }
            bool TestNotificationsStorage::waitForQueueSizeToBe(size_t size, milliseconds timeout) {
                unique_lock<mutex> lock(m_mutex);
                return m_conditionVariable.wait_for(lock, timeout, [this, size] { return m_notificationQueue.size() == size; });
            }
            class MockNotificationRenderer : public acsdkNotificationsInterfaces::NotificationRendererInterface {
            public:
                ~MockNotificationRenderer();
                MockNotificationRenderer();
                static shared_ptr<NiceMock<MockNotificationRenderer>> create() {
                    auto renderer = make_shared<NiceMock<MockNotificationRenderer>>();
                    /*ON_CALL(*renderer.get(), renderNotificationShim(_, _)).WillByDefault(Invoke(renderer.get(),
                            &MockNotificationRenderer::mockRender));*/
                    ON_CALL(*renderer.get(), cancelNotificationRenderingShim()).WillByDefault(InvokeWithoutArgs(renderer.get(),
                            &MockNotificationRenderer::mockCancel));
                    return renderer;
                }
                void addObserver(shared_ptr<NotificationRendererObserverInterface> observer) override;
                void removeObserver(shared_ptr<NotificationRendererObserverInterface> observer) override;
                bool renderNotification(function<pair<unique_ptr<istream>, const MediaType>()> audioFactory, const string& url) override;
                bool cancelNotificationRendering() override;
                //MOCK_METHOD2(renderNotificationShim, bool(function<pair<unique_ptr<istream>, const MediaType>()> audioFactory, const string& url));
                MOCK_METHOD0(cancelNotificationRenderingShim, bool());
                bool mockRender(function<pair<unique_ptr<istream>, const MediaType>()> audioFactory, const string& url);
                bool mockCancel();
                bool waitForRenderCall();
                bool waitForRenderCallDone();
                bool waitUntilRenderingStarted(milliseconds timeout = MY_WAIT_TIMEOUT);
                bool waitUntilRenderingFinished(milliseconds timeout = MY_WAIT_TIMEOUT);
            private:
                shared_ptr<NotificationRendererObserverInterface> m_observer;
                thread m_renderStartedThread;
                thread m_renderFinishedThread;
                condition_variable m_renderTrigger;
                bool m_startedRendering;
                bool m_finishedRendering;
                bool m_cancelling;
                promise<void> m_renderStartedPromise;
                future<void> m_renderStartedFuture;
                promise<void> m_renderFinishedPromise;
                future<void> m_renderFinishedFuture;
                mutex m_mutex;
            };
            MockNotificationRenderer::~MockNotificationRenderer() {
                if (m_renderStartedThread.joinable()) m_renderStartedThread.join();
                if (m_renderFinishedThread.joinable()) m_renderFinishedThread.join();
            }
            MockNotificationRenderer::MockNotificationRenderer() :
                    m_observer{nullptr},
                    m_startedRendering{false},
                    m_finishedRendering{false},
                    m_cancelling{false},
                    m_renderStartedPromise{},
                    m_renderStartedFuture{m_renderStartedPromise.get_future()},
                    m_renderFinishedPromise{},
                    m_renderFinishedFuture{m_renderFinishedPromise.get_future()} {
            }
            void MockNotificationRenderer::addObserver(shared_ptr<NotificationRendererObserverInterface> observer) {
                if (observer) m_observer = observer;
            }
            void MockNotificationRenderer::removeObserver(shared_ptr<NotificationRendererObserverInterface> observer) {
                m_observer = nullptr;
            }
            bool MockNotificationRenderer::renderNotification(function<pair<unique_ptr<istream>, const MediaType>()> audioFactory,
                                                              const string& url) {
                //return renderNotificationShim(audioFactory, url);
                return false;
            }
            bool MockNotificationRenderer::cancelNotificationRendering() {
                return cancelNotificationRenderingShim();
            }
            bool MockNotificationRenderer::mockRender(function<pair<unique_ptr<istream>, const MediaType>()> audioFactory, const string& url) {
                lock_guard<mutex> lock(m_mutex);
                if (m_renderStartedThread.joinable() && m_renderFinishedThread.joinable()) {
                    m_renderStartedThread.join();
                    m_renderFinishedThread.join();
                }
                m_renderStartedThread = thread(&MockNotificationRenderer::waitForRenderCall, this);
                m_renderFinishedThread = thread(&MockNotificationRenderer::waitForRenderCallDone, this);
                m_startedRendering = true;
                m_renderTrigger.notify_all();
                this_thread::sleep_for(RENDER_TIME);
                m_finishedRendering = true;
                m_renderTrigger.notify_all();
                return true;
            }
            bool MockNotificationRenderer::mockCancel() {
                m_cancelling = true;
                m_renderTrigger.notify_all();
                return true;
            }
            bool MockNotificationRenderer::waitForRenderCall() {
                unique_lock<mutex> lock(m_mutex);
                m_renderTrigger.wait_for(lock, MY_WAIT_TIMEOUT, [this]() { return m_startedRendering; });
                m_renderStartedPromise.set_value();
                return true;
            }
            bool MockNotificationRenderer::waitForRenderCallDone() {
                unique_lock<mutex> lock(m_mutex);
                m_renderTrigger.wait_for(lock, MY_WAIT_TIMEOUT, [this]() { return m_cancelling || m_finishedRendering; });
                m_renderFinishedPromise.set_value();
                return true;
            }
            bool MockNotificationRenderer::waitUntilRenderingStarted(milliseconds timeout) {
                if (m_renderStartedFuture.wait_for(timeout) == future_status::ready) {
                    m_startedRendering = false;
                    m_renderStartedPromise = promise<void>();
                    m_renderStartedFuture = m_renderStartedPromise.get_future();
                    return true;
                }
                return false;
            }
            bool MockNotificationRenderer::waitUntilRenderingFinished(milliseconds timeout) {
                if (m_renderFinishedFuture.wait_for(timeout) == future_status::ready) {
                    m_finishedRendering = false;
                    m_renderFinishedPromise = promise<void>();
                    m_renderFinishedFuture = m_renderFinishedPromise.get_future();
                    m_observer->onNotificationRenderingFinished();
                    return true;
                }
                return false;
            }
            class NotificationsCapabilityAgentTest : public Test {
            public:
                void SetUp() override;
                void TearDown() override;
                void initializeCapabilityAgent();
                void sendSetIndicatorDirective(const string& payload, const string& messageId);
                void sendClearIndicatorDirective(const string& messageId);
                const string generatePayload(bool persistVisualIndicator, bool playAudioIndicator, const string& assetId = ASSET_ID1,
                                             const string& assetUrl = ASSET_URL1);
                shared_ptr<MetricRecorderInterface> m_metricRecorder;
                shared_ptr<TestNotificationsObserver> m_testNotificationsObserver;
                shared_ptr<NotificationsCapabilityAgent> m_notificationsCapabilityAgent;
                shared_ptr<TestNotificationsStorage> m_notificationsStorage;
                shared_ptr<MockNotificationRenderer> m_renderer;
                shared_ptr<MockContextManager> m_mockContextManager;
                unique_ptr<MockDirectiveHandlerResult> m_mockDirectiveHandlerResult;
                shared_ptr<MockExceptionEncounteredSender> m_mockExceptionSender;
                shared_ptr<TestNotificationsAudioFactory> m_testNotificationsAudioFactory;
                mutex m_mutex;
                condition_variable m_setIndicatorTrigger;
                shared_ptr<registrationManager::CustomerDataManager> m_dataManager;
                unsigned int m_numSetIndicatorsProcessed;
            };
            void NotificationsCapabilityAgentTest::initializeCapabilityAgent() {
                m_notificationsCapabilityAgent = NotificationsCapabilityAgent::create(m_notificationsStorage,m_renderer,
                                                                         m_mockContextManager, m_mockExceptionSender,
                                                                  m_testNotificationsAudioFactory,m_dataManager,
                                                                         m_metricRecorder);
                ASSERT_TRUE(m_notificationsCapabilityAgent);
                m_notificationsCapabilityAgent->addObserver(m_testNotificationsObserver);
                m_renderer->addObserver(m_notificationsCapabilityAgent);
            }
            void NotificationsCapabilityAgentTest::SetUp() {
                auto inString = shared_ptr<istringstream>(new istringstream(NOTIFICATIONS_CONFIG_JSON));
                ASSERT_TRUE(AlexaClientSDKInit::initialize({inString}));
                m_metricRecorder = make_shared<NiceMock<MockMetricRecorder>>();
                m_notificationsStorage = std::make_shared<TestNotificationsStorage>();
                m_renderer = MockNotificationRenderer::create();
                m_mockContextManager = make_shared<NiceMock<MockContextManager>>();
                m_mockExceptionSender = make_shared<NiceMock<MockExceptionEncounteredSender>>();
                m_testNotificationsAudioFactory = make_shared<TestNotificationsAudioFactory>();
                m_testNotificationsObserver = make_shared<TestNotificationsObserver>();
                m_mockDirectiveHandlerResult = unique_ptr<MockDirectiveHandlerResult>(new MockDirectiveHandlerResult);
                m_numSetIndicatorsProcessed = 0;
                m_dataManager = make_shared<CustomerDataManager>();
            }
            void NotificationsCapabilityAgentTest::TearDown() {
                if (m_notificationsCapabilityAgent) m_notificationsCapabilityAgent->shutdown();
                AlexaClientSDKInit::uninitialize();
            }
            void NotificationsCapabilityAgentTest::sendSetIndicatorDirective(const string& payload, const string& messageId) {
                auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_NOTIFICATIONS, NAME_SET_INDICATOR, messageId);
                auto mockAttachmentManager = make_shared<MockAttachmentManager>();
                shared_ptr<AVSDirective> setIndicatorDirective = AVSDirective::create("", avsMessageHeader, payload,
                                                                                      mockAttachmentManager, CONTEXT_ID_TEST);
                shared_ptr<DirectiveHandlerInterface> agentAsDirectiveHandler = m_notificationsCapabilityAgent;
                agentAsDirectiveHandler->preHandleDirective(setIndicatorDirective, std::move(m_mockDirectiveHandlerResult));
                agentAsDirectiveHandler->handleDirective(messageId);
                m_numSetIndicatorsProcessed++;
                m_setIndicatorTrigger.notify_all();
            }
            void NotificationsCapabilityAgentTest::sendClearIndicatorDirective(const string& messageId) {
                auto avsMessageHeader = make_shared<AVSMessageHeader>(NAMESPACE_NOTIFICATIONS, NAME_CLEAR_INDICATOR, messageId);
                auto mockAttachmentManager = make_shared<MockAttachmentManager>();
                shared_ptr<AVSDirective> clearIndicatorDirective = AVSDirective::create("", avsMessageHeader, "",
                                                                                        mockAttachmentManager, CONTEXT_ID_TEST);
                shared_ptr<DirectiveHandlerInterface> agentAsDirectiveHandler = m_notificationsCapabilityAgent;
                agentAsDirectiveHandler->preHandleDirective(clearIndicatorDirective, move(m_mockDirectiveHandlerResult));
                agentAsDirectiveHandler->handleDirective(messageId);
            }
            const string NotificationsCapabilityAgentTest::generatePayload(bool persistVisualIndicator, bool playAudioIndicator,
                                                                           const string& assetId, const string& assetUrl) {
                string stringPersistVisualIndicator(persistVisualIndicator ? "true" : "false");
                string stringPlayAudioIndicator(playAudioIndicator ? "true" : "false");
                const string payload = "{\"persistVisualIndicator\":" + stringPersistVisualIndicator + ",\"playAudioIndicator\":" +
                                       stringPlayAudioIndicator + ",\"asset\": {\"assetId\":\"" + assetId + "\",\"url\":\"" + assetUrl + "\"}}";
                return payload;
            }
            TEST_F(NotificationsCapabilityAgentTest, test_create) {
                shared_ptr<NotificationsCapabilityAgent> testNotificationsCapabilityAgent;
                testNotificationsCapabilityAgent = NotificationsCapabilityAgent::create(nullptr,m_renderer,
                                                                           m_mockContextManager,m_mockExceptionSender,
                                                                     m_testNotificationsAudioFactory,m_dataManager,
                                                                            m_metricRecorder);
                EXPECT_EQ(testNotificationsCapabilityAgent, nullptr);
                testNotificationsCapabilityAgent = NotificationsCapabilityAgent::create(m_notificationsStorage,nullptr,
                                                                           m_mockContextManager,m_mockExceptionSender,
                                                                     m_testNotificationsAudioFactory,m_dataManager,
                                                                            m_metricRecorder);
                EXPECT_EQ(testNotificationsCapabilityAgent, nullptr);
                testNotificationsCapabilityAgent = NotificationsCapabilityAgent::create(m_notificationsStorage,m_renderer,
                                                                           nullptr,m_mockExceptionSender,
                                                                     m_testNotificationsAudioFactory,m_dataManager,
                                                                            m_metricRecorder);
                EXPECT_EQ(testNotificationsCapabilityAgent, nullptr);
                testNotificationsCapabilityAgent = NotificationsCapabilityAgent::create(m_notificationsStorage,m_renderer,
                                                                           m_mockContextManager,nullptr,
                                                                    m_testNotificationsAudioFactory,m_dataManager,
                                                                           m_metricRecorder);
                EXPECT_EQ(testNotificationsCapabilityAgent, nullptr);
                testNotificationsCapabilityAgent = NotificationsCapabilityAgent::create(m_notificationsStorage,m_renderer,
                                                                           m_mockContextManager,m_mockExceptionSender,
                                                                    nullptr,m_dataManager,
                                                                           m_metricRecorder);
                EXPECT_EQ(testNotificationsCapabilityAgent, nullptr);
            }
            TEST_F(NotificationsCapabilityAgentTest, test_nonEmptyStartupQueue) {
                NotificationIndicator ni(true, true, ASSET_ID1, ASSET_URL1);
                ASSERT_TRUE(m_notificationsStorage->enqueue(ni));
                //EXPECT_CALL(*(m_renderer.get()), renderNotificationShim(_, ASSET_URL1)).Times(1);
                initializeCapabilityAgent();
                ASSERT_TRUE(m_renderer->waitUntilRenderingFinished());
            }
            TEST_F(NotificationsCapabilityAgentTest, test_sendSetIndicator) {
                //EXPECT_CALL(*(m_renderer.get()), renderNotificationShim(_, _)).Times(0);
                initializeCapabilityAgent();
                ASSERT_TRUE(m_testNotificationsObserver->waitFor(IndicatorState::OFF, MY_WAIT_TIMEOUT));
                sendSetIndicatorDirective(generatePayload(true, false), MESSAGE_ID_TEST);
                ASSERT_TRUE(m_testNotificationsObserver->waitFor(IndicatorState::ON, MY_WAIT_TIMEOUT));
                ASSERT_TRUE(m_notificationsStorage->waitForQueueSizeToBe(0));
            }
            TEST_F(NotificationsCapabilityAgentTest, test_sendSetIndicatorIncreasesCount) {
                initializeCapabilityAgent();
                ASSERT_TRUE(m_testNotificationsObserver->waitFor(0, MY_WAIT_TIMEOUT));
                sendSetIndicatorDirective(generatePayload(true, false), MESSAGE_ID_TEST);
                ASSERT_TRUE(m_testNotificationsObserver->waitFor(1, MY_WAIT_TIMEOUT));
                sendSetIndicatorDirective(generatePayload(true, false), MESSAGE_ID_TEST);
                ASSERT_TRUE(m_testNotificationsObserver->waitFor(2, MY_WAIT_TIMEOUT));
                sendSetIndicatorDirective(generatePayload(true, true), MESSAGE_ID_TEST);
                ASSERT_TRUE(m_testNotificationsObserver->waitFor(3, MY_WAIT_TIMEOUT));
            }
            TEST_F(NotificationsCapabilityAgentTest, test_persistVisualIndicatorPreservedIncreasesCount) {
                initializeCapabilityAgent();
                sendSetIndicatorDirective(generatePayload(true, false, ASSET_ID1), MESSAGE_ID_TEST);
                ASSERT_TRUE(m_testNotificationsObserver->waitFor(1, MY_WAIT_TIMEOUT));
                m_notificationsCapabilityAgent->shutdown();
                initializeCapabilityAgent();
                ASSERT_TRUE(m_testNotificationsObserver->waitFor(1, MY_WAIT_TIMEOUT));
            }
            TEST_F(NotificationsCapabilityAgentTest, test_sendSetIndicatorWithAudio) {
                //XPECT_CALL(*(m_renderer.get()), renderNotificationShim(_, ASSET_URL1));
                initializeCapabilityAgent();
                sendSetIndicatorDirective(generatePayload(false, true, ASSET_ID1, ASSET_URL1), MESSAGE_ID_TEST);
                ASSERT_TRUE(m_testNotificationsObserver->waitFor(IndicatorState::OFF, MY_WAIT_TIMEOUT));
                ASSERT_TRUE(m_renderer->waitUntilRenderingFinished());
            }
            TEST_F(NotificationsCapabilityAgentTest, test_sendSetIndicatorWithVisualIndicator) {
                //EXPECT_CALL(*(m_renderer.get()), renderNotificationShim(_, _)).Times(0);
                initializeCapabilityAgent();
                sendSetIndicatorDirective(generatePayload(true, false), MESSAGE_ID_TEST);
                ASSERT_TRUE(m_testNotificationsObserver->waitFor(IndicatorState::ON, MY_WAIT_TIMEOUT));
            }
            TEST_F(NotificationsCapabilityAgentTest, test_sameAssetId) {
                /*EXPECT_CALL(*(m_renderer.get()), renderNotificationShim(_, ASSET_URL1)).Times(1)
                    .WillOnce(Invoke([this](function<pair<unique_ptr<istream>, const MediaType>()> audioFactory, const string& url) {
                        unsigned int expectedNumSetIndicators = 2;
                        unique_lock<std::mutex> lock(m_mutex);
                        if (!m_setIndicatorTrigger.wait_for(lock, MY_WAIT_TIMEOUT, [this, expectedNumSetIndicators]() {
                                return m_numSetIndicatorsProcessed == expectedNumSetIndicators;
                            })) {
                            return false;
                        }
                        m_renderer->mockRender(audioFactory, url);
                        EXPECT_TRUE(m_renderer->waitUntilRenderingStarted());
                        return true;
                    }));*/
                initializeCapabilityAgent();
                sendSetIndicatorDirective(generatePayload(true, true, ASSET_ID1), MESSAGE_ID_TEST);
                sendSetIndicatorDirective(generatePayload(false, true, ASSET_ID1), MESSAGE_ID_TEST2);
                ASSERT_TRUE(m_testNotificationsObserver->waitFor(IndicatorState::ON, MY_WAIT_TIMEOUT));
            }
            TEST_F(NotificationsCapabilityAgentTest, test_persistVisualIndicatorPreserved) {
                initializeCapabilityAgent();
                sendSetIndicatorDirective(generatePayload(true, false, ASSET_ID1), MESSAGE_ID_TEST);
                ASSERT_TRUE(m_testNotificationsObserver->waitFor(IndicatorState::ON, MY_WAIT_TIMEOUT));
                m_notificationsCapabilityAgent->shutdown();
                initializeCapabilityAgent();
                ASSERT_TRUE(m_testNotificationsObserver->waitFor(IndicatorState::ON, MY_WAIT_TIMEOUT));
                sendSetIndicatorDirective(generatePayload(false, false, ASSET_ID1), MESSAGE_ID_TEST);
                ASSERT_TRUE(m_testNotificationsObserver->waitFor(IndicatorState::OFF, MY_WAIT_TIMEOUT));
                m_notificationsCapabilityAgent->shutdown();
                initializeCapabilityAgent();
                ASSERT_TRUE(m_testNotificationsObserver->waitFor(IndicatorState::OFF, MY_WAIT_TIMEOUT));
            }
            TEST_F(NotificationsCapabilityAgentTest, test_clearIndicatorWithEmptyQueue) {
                initializeCapabilityAgent();
                sendClearIndicatorDirective(MESSAGE_ID_TEST);
                ASSERT_TRUE(m_testNotificationsObserver->waitFor(IndicatorState::OFF, MY_WAIT_TIMEOUT));
            }
            TEST_F(NotificationsCapabilityAgentTest, test_clearIndicatorWithEmptyQueueAndIndicatorOn) {
                //EXPECT_CALL(*(m_renderer.get()), renderNotificationShim(_, ASSET_URL1)).Times(1);
                initializeCapabilityAgent();
                sendSetIndicatorDirective(generatePayload(true, true, ASSET_ID1), MESSAGE_ID_TEST);
                ASSERT_TRUE(m_renderer->waitUntilRenderingFinished());
                ASSERT_TRUE(m_testNotificationsObserver->waitFor(IndicatorState::ON, MY_WAIT_TIMEOUT));
                sendClearIndicatorDirective(MESSAGE_ID_TEST2);
                ASSERT_TRUE(m_testNotificationsObserver->waitFor(IndicatorState::OFF, MY_WAIT_TIMEOUT));
            }
            TEST_F(NotificationsCapabilityAgentTest, testSlow_clearIndicatorAfterMultipleSetIndicators) {
                //EXPECT_CALL(*(m_renderer.get()), renderNotificationShim(_, ASSET_URL1)).Times(1);
                EXPECT_CALL(*(m_renderer.get()), cancelNotificationRenderingShim()).Times(1);
                initializeCapabilityAgent();
                sendSetIndicatorDirective(generatePayload(true, true, "assetId1"),
                                          "firstIndicatorMessageId");
                sendSetIndicatorDirective(generatePayload(true, true, "assetId2"),
                                          "secondIndicatorMessageId");
                sendSetIndicatorDirective(generatePayload(true, true, "assetId3"),
                                          "thirdIndicatorMessageId");
                ASSERT_TRUE(m_renderer->waitUntilRenderingStarted());
                sendClearIndicatorDirective(MESSAGE_ID_TEST);
                ASSERT_TRUE(m_renderer->waitUntilRenderingFinished());
                ASSERT_TRUE(m_notificationsStorage->waitForQueueSizeToBe(0));
            }
            TEST_F(NotificationsCapabilityAgentTest, test_multipleSetIndicators) {
                //EXPECT_CALL(*(m_renderer.get()), renderNotificationShim(_, ASSET_URL1)).Times(3);
                initializeCapabilityAgent();
                sendSetIndicatorDirective(generatePayload(true, true, "id1"),
                                          "firstIndicatorMessageId");
                sendSetIndicatorDirective(generatePayload(true, true, "id2"),
                                          "secondIndicatorMessageId");
                sendSetIndicatorDirective(generatePayload(true, true, "id3"),
                                          "thirdIndicatorMessageId");
                ASSERT_TRUE(m_renderer->waitUntilRenderingStarted());
                ASSERT_TRUE(m_renderer->waitUntilRenderingFinished());
                ASSERT_TRUE(m_renderer->waitUntilRenderingStarted());
                ASSERT_TRUE(m_renderer->waitUntilRenderingFinished());
                ASSERT_TRUE(m_renderer->waitUntilRenderingStarted());
                ASSERT_TRUE(m_renderer->waitUntilRenderingFinished());
            }
            TEST_F(NotificationsCapabilityAgentTest, test_clearData) {
                initializeCapabilityAgent();
                sendSetIndicatorDirective(generatePayload(true, true, "assetId1"), "firstIndicatorMessageId");
                ASSERT_TRUE(m_renderer->waitUntilRenderingStarted());
                IndicatorState state = IndicatorState::UNDEFINED;
                m_notificationsStorage->getIndicatorState(&state);
                ASSERT_EQ(state, IndicatorState::ON);
                int queueSize;
                m_notificationsStorage->getQueueSize(&queueSize);
                ASSERT_GT(queueSize, 0);
                m_notificationsCapabilityAgent->clearData();
                ASSERT_TRUE(m_notificationsStorage->waitForQueueSizeToBe(0));
                m_notificationsStorage->getIndicatorState(&state);
                ASSERT_EQ(state, IndicatorState::OFF);
                ASSERT_TRUE(m_testNotificationsObserver->waitFor(IndicatorState::OFF, MY_WAIT_TIMEOUT));
            }
        }
    }
}
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
#if defined(_WIN32) && !defined(RESOLVED_ACSDK_1367)
    ::testing::GTEST_FLAG(filter) = "-NotificationsCapabilityAgentTest.testSameAssetId";
#endif
    return RUN_ALL_TESTS();
}