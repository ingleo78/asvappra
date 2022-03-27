#include <vector>
#include <gtest/gtest.h>
#include <acl/Transport/HTTP2TransportFactory.h>
#include <acl/Transport/PostConnectSequencerFactory.h>
#include <util/DeviceInfo.h>
#include <lib_curl_utils/LibcurlHTTP2ConnectionFactory.h>
#include <autorization/CBLAuthDelegate/CBLAuthDelegate.h>
#include <synchronize_state_sender/SynchronizeStateSenderFactory.h>
#include "ACLTestContext.h"

namespace alexaClientSDK {
    namespace integration {
        namespace test {
            using namespace std;
            using namespace acl;
            using namespace avs;
            using namespace attachment;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace configuration;
            using namespace libcurlUtils;
            using namespace authorization;
            using namespace cblAuthDelegate;
            using namespace contextManager;
            using namespace registrationManager;
            using namespace synchronizeStateSender;
            unique_ptr<ACLTestContext> ACLTestContext::create(const string& filePath, const string& overlay) {
                unique_ptr<ACLTestContext> context(new ACLTestContext(filePath, overlay));
                if (context->m_authDelegateTestContext && context->m_attachmentManager && context->m_messageRouter &&
                    context->m_connectionStatusObserver && context->m_contextManager) {
                    return context;
                }
                return nullptr;
            }
            ACLTestContext::~ACLTestContext() {
                m_attachmentManager.reset();
                if (m_messageRouter) m_messageRouter->shutdown();
                m_connectionStatusObserver.reset();
                m_contextManager.reset();
                m_authDelegateTestContext.reset();
            }
            shared_ptr<AuthDelegateInterface> ACLTestContext::getAuthDelegate() const {
                return m_authDelegateTestContext->getAuthDelegate();
            }
            shared_ptr<CustomerDataManager> ACLTestContext::getCustomerDataManager() const {
                return m_authDelegateTestContext->getCustomerDataManager();
            }
            shared_ptr<AttachmentManager> ACLTestContext::getAttachmentManager() const {
                return m_attachmentManager;
            }
            shared_ptr<MessageRouter> ACLTestContext::getMessageRouter() const {
                return m_messageRouter;
            }
            shared_ptr<ConnectionStatusObserver> ACLTestContext::getConnectionStatusObserver() const {
                return m_connectionStatusObserver;
            }
            shared_ptr<ContextManagerInterface> ACLTestContext::getContextManager() const {
                return m_contextManager;
            }
            void ACLTestContext::waitForConnected() {
                ASSERT_TRUE(m_connectionStatusObserver->waitFor(ConnectionStatusObserverInterface::Status::CONNECTED))
                    << "Connecting timed out";
            }
            void ACLTestContext::waitForDisconnected() {
                ASSERT_TRUE(m_connectionStatusObserver->waitFor(ConnectionStatusObserverInterface::Status::DISCONNECTED))
                    << "Disconnecting timed out";
            }
            static const string DEFAULT_AVS_GATEWAY = "https://alexa.na.gateway.devices.a2z.com";
            ACLTestContext::ACLTestContext(const string& filePath, const string& overlay) {
                m_authDelegateTestContext = AuthDelegateTestContext::create(filePath, overlay);
                EXPECT_TRUE(m_authDelegateTestContext);
                if (!m_authDelegateTestContext) return;
                auto config = ConfigurationNode::getRoot();
                EXPECT_TRUE(config);
                if (!config) return;
                auto deviceInfo = DeviceInfo::createFromConfiguration(make_shared<ConfigurationNode>(config));
                EXPECT_TRUE(deviceInfo);
                if (!deviceInfo) return;
                m_contextManager = ContextManager::createContextManagerInterface(move(deviceInfo));
                EXPECT_TRUE(m_contextManager);
                if (!m_contextManager) return;
                auto synchronizeStateSenderFactory = SynchronizeStateSenderFactory::create(m_contextManager);
                vector<shared_ptr<PostConnectOperationProviderInterface>> providers;
                providers.push_back(synchronizeStateSenderFactory);
                auto postConnectFactory = PostConnectSequencerFactory::create(providers);
                auto http2ConnectionFactory = make_shared<LibcurlHTTP2ConnectionFactory>();
                auto transportFactory = make_shared<HTTP2TransportFactory>(http2ConnectionFactory, postConnectFactory);
                m_attachmentManager = make_shared<AttachmentManager>(AttachmentManager::AttachmentType::IN_PROCESS);
                m_messageRouter = make_shared<MessageRouter>(getAuthDelegate(), m_attachmentManager, transportFactory, DEFAULT_AVS_GATEWAY);
                m_connectionStatusObserver = make_shared<ConnectionStatusObserver>();
            }
        }
    }
}