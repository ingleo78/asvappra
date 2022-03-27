#ifndef ALEXA_CLIENT_SDK_ACL_TEST_TRANSPORT_MESSAGEROUTERTEST_H_
#define ALEXA_CLIENT_SDK_ACL_TEST_TRANSPORT_MESSAGEROUTERTEST_H_

#include <gtest/gtest.h>
#include <memory>
#include <sstream>
#include <string>
#include <threading/Executor.h>
#include <memory/Memory.h>
#include "MockMessageRouterObserver.h"
#include "MockAuthDelegate.h"
#include "MockTransport.h"
#include "MessageRouter.h"
#include "MessageConsumerInterface.h"

namespace alexaClientSDK {
    namespace acl {
        namespace test {
            using namespace std;
            using namespace chrono;
            using namespace transport;
            using namespace transport::test;
            using namespace avsCommon;
            using namespace avs;
            using namespace attachment;
            using namespace utils;
            using namespace sdkInterfaces;
            using namespace threading;
            using namespace memory;
            using namespace testing;
            class TestableMessageRouter : public MessageRouter {
            public:
                TestableMessageRouter(shared_ptr<AuthDelegateInterface> authDelegate, shared_ptr<AttachmentManager> attachmentManager,
                                      shared_ptr<TransportFactoryInterface> factory, const string& avsGateway) :
                                      MessageRouter(authDelegate, attachmentManager, factory, avsGateway) {}
                bool isExecutorReady(milliseconds millisecondsToWait) {
                    auto future = m_executor.submit([]() { ; });
                    auto status = future.wait_for(millisecondsToWait);
                    return status == future_status::ready;
                }
            };
            class MockTransportFactory : public TransportFactoryInterface {
            public:
                MockTransportFactory(shared_ptr<MockTransport> transport) : m_mockTransport{transport} {}
                void setMockTransport(shared_ptr<MockTransport> transport) {
                    m_mockTransport = transport;
                }
            private:
                shared_ptr<TransportInterface> createTransport(shared_ptr<AuthDelegateInterface> authDelegate, shared_ptr<AttachmentManager> attachmentManager,
                                                               const string& avsGateway, shared_ptr<MessageConsumerInterface> messageConsumerInterface,
                                                               shared_ptr<TransportObserverInterface> transportObserverInterface,
                                                               shared_ptr<SynchronizedMessageRequestQueue> sharedMessageRequestQueue) override {
                    return m_mockTransport;
                }
                shared_ptr<MockTransport> m_mockTransport;
            };
            class MessageRouterTest : public Test {
            public:
                const string AVS_ENDPOINT = "AVS_ENDPOINT";
                MessageRouterTest() : m_mockMessageRouterObserver{make_shared<MockMessageRouterObserver>()}, m_mockAuthDelegate{make_shared<MockAuthDelegate>()},
                                      m_attachmentManager{make_shared<AttachmentManager>(AttachmentManager::AttachmentType::IN_PROCESS)},
                                      m_mockTransport{make_shared<NiceMock<MockTransport>>()},m_transportFactory{make_shared<MockTransportFactory>(m_mockTransport)},
                                      m_router{make_shared<TestableMessageRouter>(m_mockAuthDelegate, m_attachmentManager, m_transportFactory, AVS_ENDPOINT)} {
                    m_router->setObserver(m_mockMessageRouterObserver);
                }
                void TearDown() {
                    waitOnMessageRouter(SHORT_TIMEOUT_MS);
                }
                shared_ptr<MessageRequest> createMessageRequest() {
                    return make_shared<MessageRequest>(MESSAGE);
                }
                void waitOnMessageRouter(milliseconds millisecondsToWait) {
                    auto status = m_router->isExecutorReady(millisecondsToWait);
                    ASSERT_EQ(true, status);
                }
                void setupStateToPending() {
                    initializeMockTransport(m_mockTransport.get());
                    m_router->enable();
                }
                void setupStateToConnected() {
                    setupStateToPending();
                    m_router->onConnected(m_mockTransport);
                    connectMockTransport(m_mockTransport.get());
                }
                static const string MESSAGE;
                static const int MESSAGE_LENGTH;
                static const milliseconds SHORT_TIMEOUT_MS;
                static const string CONTEXT_ID;
                shared_ptr<MockMessageRouterObserver> m_mockMessageRouterObserver;
                shared_ptr<MockAuthDelegate> m_mockAuthDelegate;
                shared_ptr<AttachmentManager> m_attachmentManager;
                shared_ptr<NiceMock<MockTransport>> m_mockTransport;
                shared_ptr<MockTransportFactory> m_transportFactory;
                shared_ptr<TestableMessageRouter> m_router;
            };
            const string MessageRouterTest::MESSAGE = "123456789";
            const int MessageRouterTest::MESSAGE_LENGTH = 10;
            const milliseconds MessageRouterTest::SHORT_TIMEOUT_MS = milliseconds(1000);
            const string MessageRouterTest::CONTEXT_ID = "contextIdString";
        }
    }
}
#endif