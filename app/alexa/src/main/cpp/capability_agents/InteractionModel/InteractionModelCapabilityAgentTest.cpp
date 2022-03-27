#include <condition_variable>
#include <iterator>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sdkinterfaces/MockDirectiveSequencer.h>
#include <sdkinterfaces/MockExceptionEncounteredSender.h>
#include <sdkinterfaces/InteractionModelRequestProcessingObserverInterface.h>
#include "InteractionModelCapabilityAgent.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace interactionModel {
            namespace test {
                using namespace testing;
                using namespace sdkInterfaces::test;
                const string TEST_DIALOG_REQUEST_AVS = "2";
                static const string CORRECT_NEW_DIALOG_REQUEST_DIRECTIVE_JSON_STRING = R"delim({"directive": {"header": {"namespace": "InteractionModel",
                                                                                       "name": "NewDialogRequest","messageId": "12345"},
                                                                                       "payload": {"dialogRequestId": "2"}}})delim";
                static const string INCORRECT_NEW_DIALOG_REQUEST_DIRECTIVE_JSON_STRING_1 = R"delim({"directive": {"header": {"namespace": "InteractionModel",
                                                                                           "name": "NewDialogRequest1","messageId": "12345"},
                                                                                           "payload": {"dialogRequestId": "2"}}})delim";
                static const string INCORRECT_NEW_DIALOG_REQUEST_DIRECTIVE_JSON_STRING_2 = R"delim({"directive": {"header": {"namespace": "InteractionModel",
                                                                                           "name": "NewDialogRequest","messageId": "12345"
                                                                                           },"payload": {}}})delim";
                static const string INCORRECT_NEW_DIALOG_REQUEST_DIRECTIVE_JSON_STRING_3 = R"delim({"directive": {"header": {"namespace": "InteractionModel",
                                                                                           "name": "NewDialogRequest","messageId": "12345"},
                                                                                           "payload": {"dialogRequestId": 2}}})delim";
                static const string INCORRECT_NEW_DIALOG_REQUEST_DIRECTIVE_JSON_STRING_4 = R"delim({"directive": {"header": {"namespace": "InteractionModel",
                                                                                           "name": "NewDialogRequest","messageId": "12345"},
                                                                                           "payload": {"dialogRequestId": ""}}})delim";
                static const string RPS_DIRECTIVE_JSON_STRING = R"delim({"directive": {"header": {"namespace": "InteractionModel",
                                                                "name": "RequestProcessingStarted","messageId": "12345"},"payload": {}}})delim";
                static const string RPC_DIRECTIVE_JSON_STRING = R"delim({"directive": {"header": {"namespace": "InteractionModel",
                                                                "name": "RequestProcessingCompleted","messageId": "12345"},"payload": {}}})delim";
                milliseconds TIMEOUT{500};
                class InteractionModelCapabilityAgentTest : public Test {
                public:
                    void SetUp() override;
                protected:
                    shared_ptr<InteractionModelCapabilityAgent> m_interactionModelCA;
                    shared_ptr<MockDirectiveSequencer> m_mockDirectiveSequencer;
                    shared_ptr<MockExceptionEncounteredSender> m_mockExceptionEncounteredSender;
                };
                class MockObserver : public InteractionModelRequestProcessingObserverInterface {
                public:
                    void onRequestProcessingStarted() override;
                    void onRequestProcessingCompleted() override;
                    bool waitOnRPS();
                    bool waitOnRPC();
                protected:
                    mutex m_mutex;
                    condition_variable m_cond;
                    bool m_rpcCalled;
                    bool m_rpsCalled;
                };
                void MockObserver::onRequestProcessingStarted() {
                    lock_guard<mutex> lock(m_mutex);
                    m_rpsCalled = true;
                    m_cond.notify_all();
                }
                void MockObserver::onRequestProcessingCompleted() {
                    lock_guard<mutex> lock(m_mutex);
                    m_rpcCalled = true;
                    m_cond.notify_all();
                }
                bool MockObserver::waitOnRPS() {
                    unique_lock<mutex> lock(m_mutex);
                    return m_cond.wait_for(lock, TIMEOUT, [this] { return m_rpsCalled; });
                }
                bool MockObserver::waitOnRPC() {
                    unique_lock<mutex> lock(m_mutex);
                    return m_cond.wait_for(lock, TIMEOUT, [this] { return m_rpcCalled; });
                }
                void InteractionModelCapabilityAgentTest::SetUp() {
                    m_mockDirectiveSequencer = make_shared<MockDirectiveSequencer>();
                    m_mockExceptionEncounteredSender =make_shared<MockExceptionEncounteredSender>();
                    m_interactionModelCA = InteractionModelCapabilityAgent::create(m_mockDirectiveSequencer, m_mockExceptionEncounteredSender);
                    ASSERT_NE(m_interactionModelCA, nullptr);
                }
                TEST_F(InteractionModelCapabilityAgentTest, test_createNoDirectiveSequencer) {
                    m_interactionModelCA = InteractionModelCapabilityAgent::create(nullptr, m_mockExceptionEncounteredSender);
                    ASSERT_EQ(m_interactionModelCA, nullptr);
                }
                TEST_F(InteractionModelCapabilityAgentTest, test_createNoExceptionHanlder) {
                    m_interactionModelCA = InteractionModelCapabilityAgent::create(m_mockDirectiveSequencer, nullptr);
                    ASSERT_EQ(m_interactionModelCA, nullptr);
                }
                TEST_F(InteractionModelCapabilityAgentTest, test_processNewDialogRequestID) {
                    auto directivePair = AVSDirective::create(CORRECT_NEW_DIALOG_REQUEST_DIRECTIVE_JSON_STRING, nullptr, "");
                    shared_ptr<AVSDirective> directive = std::move(directivePair.first);
                    m_interactionModelCA->handleDirectiveImmediately(directive);
                    ASSERT_EQ(TEST_DIALOG_REQUEST_AVS, m_mockDirectiveSequencer->getDialogRequestId());
                }
                TEST_F(InteractionModelCapabilityAgentTest, test_processNullDirective) {
                    m_interactionModelCA->handleDirectiveImmediately(nullptr);
                    ASSERT_EQ("", m_mockDirectiveSequencer->getDialogRequestId());
                }
                TEST_F(InteractionModelCapabilityAgentTest, test_processInvalidDirective) {
                    shared_ptr<AVSDirective> directive1 = AVSDirective::create(INCORRECT_NEW_DIALOG_REQUEST_DIRECTIVE_JSON_STRING_1, nullptr, "").first;
                    shared_ptr<AVSDirective> directive2 = AVSDirective::create(INCORRECT_NEW_DIALOG_REQUEST_DIRECTIVE_JSON_STRING_2, nullptr, "").first;
                    shared_ptr<AVSDirective> directive3 = AVSDirective::create(INCORRECT_NEW_DIALOG_REQUEST_DIRECTIVE_JSON_STRING_3, nullptr, "").first;
                    shared_ptr<AVSDirective> directive4 = AVSDirective::create(INCORRECT_NEW_DIALOG_REQUEST_DIRECTIVE_JSON_STRING_4, nullptr, "").first;
                    EXPECT_CALL(*m_mockExceptionEncounteredSender, sendExceptionEncountered(_, _, _)).Times(4);
                    m_interactionModelCA->handleDirectiveImmediately(directive1);
                    m_interactionModelCA->handleDirectiveImmediately(directive2);
                    m_interactionModelCA->handleDirectiveImmediately(directive3);
                    m_interactionModelCA->handleDirectiveImmediately(directive4);
                    ASSERT_EQ("", m_mockDirectiveSequencer->getDialogRequestId());
                }
                TEST_F(InteractionModelCapabilityAgentTest, test_addObserverRPS) {
                    shared_ptr<MockObserver> observer = make_shared<MockObserver>();
                    m_interactionModelCA->addObserver(observer);
                    auto directivePair = AVSDirective::create(RPS_DIRECTIVE_JSON_STRING, nullptr, "");
                    shared_ptr<AVSDirective> directive = move(directivePair.first);
                    m_interactionModelCA->handleDirectiveImmediately(directive);
                    EXPECT_TRUE(observer->waitOnRPS());
                }
                TEST_F(InteractionModelCapabilityAgentTest, test_addObserverRPC) {
                    shared_ptr<MockObserver> observer = make_shared<MockObserver>();
                    m_interactionModelCA->addObserver(observer);
                    auto directivePair = AVSDirective::create(RPC_DIRECTIVE_JSON_STRING, nullptr, "");
                    shared_ptr<AVSDirective> directive = move(directivePair.first);
                    m_interactionModelCA->handleDirectiveImmediately(directive);
                    EXPECT_TRUE(observer->waitOnRPC());
                }
                TEST_F(InteractionModelCapabilityAgentTest, test_addObserverNullptr_NoErrors) {
                    m_interactionModelCA->addObserver(nullptr);
                }
                TEST_F(InteractionModelCapabilityAgentTest, test_removeObserver) {
                    shared_ptr<MockObserver> observer = make_shared<MockObserver>();
                    m_interactionModelCA->addObserver(observer);
                    m_interactionModelCA->removeObserver(observer);
                    auto directivePair = AVSDirective::create(RPC_DIRECTIVE_JSON_STRING, nullptr, "");
                    shared_ptr<AVSDirective> directive = move(directivePair.first);
                    m_interactionModelCA->handleDirectiveImmediately(directive);
                    EXPECT_FALSE(observer->waitOnRPC());
                }
                TEST_F(InteractionModelCapabilityAgentTest, test_removeObserverNullptr_NoErrors) {
                    m_interactionModelCA->removeObserver(nullptr);
                }
            }
        }
    }
}