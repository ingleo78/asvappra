#ifndef ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_MESSAGEROUTER_H_
#define ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_MESSAGEROUTER_H_

#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <threading/Executor.h>
#include <avs/attachment/AttachmentManager.h>
#include <avs/MessageRequest.h>
#include <sdkinterfaces/AuthDelegateInterface.h>
#include "MessageConsumerInterface.h"
#include "MessageRouterInterface.h"
#include "MessageRouterObserverInterface.h"
#include "TransportFactoryInterface.h"
#include "TransportInterface.h"
#include "TransportObserverInterface.h"
#include "SynchronizedMessageRequestQueue.h"

namespace alexaClientSDK {
    namespace acl {
        using namespace std;
        using namespace avsCommon;
        using namespace avs;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace attachment;
        using namespace threading;
        class MessageRouter : public MessageRouterInterface, public TransportObserverInterface, public MessageConsumerInterface,
                              public enable_shared_from_this<MessageRouter> {
        public:
            MessageRouter(shared_ptr<AuthDelegateInterface> authDelegate, shared_ptr<AttachmentManager> attachmentManager,
                          shared_ptr<TransportFactoryInterface> transportFactory, const string& avsGateway = "");
            void enable() override;
            void disable() override;
            ConnectionStatus getConnectionStatus() override;
            void sendMessage(shared_ptr<MessageRequest> request) override;
            void setAVSGateway(const string& avsGateway) override;
            string getAVSGateway() override;
            void onWakeConnectionRetry() override;
            void onWakeVerifyConnectivity() override;
            void setObserver(shared_ptr<MessageRouterObserverInterface> observer) override;
            void onConnected(shared_ptr<TransportInterface> transport) override;
            void onDisconnected(shared_ptr<TransportInterface> transport, ConnectionStatusObserverInterface::ChangedReason reason) override;
            void onServerSideDisconnect(shared_ptr<TransportInterface> transport) override;
            void consumeMessage(const string& contextId, const string& message) override;
            void doShutdown() override;
        private:
            void setConnectionStatusLocked(const ConnectionStatusObserverInterface::Status status, const ConnectionStatusObserverInterface::ChangedReason reason);
            void notifyObserverOnConnectionStatusChanged(const ConnectionStatusObserverInterface::Status status,
                                                         const ConnectionStatusObserverInterface::ChangedReason reason);
            void notifyObserverOnReceive(const string& contextId, const string& message);
            void createActiveTransportLocked();
            void disconnectAllTransportsLocked(unique_lock<mutex>& lock, const ConnectionStatusObserverInterface::ChangedReason reason);
            shared_ptr<MessageRouterObserverInterface> getObserver();
            void safelyResetActiveTransportLocked();
            void safelyReleaseTransport(shared_ptr<TransportInterface> transport);
            shared_ptr<MessageRouterObserverInterface> m_observer;
            string m_avsGateway;
            shared_ptr<AuthDelegateInterface> m_authDelegate;
            mutex m_connectionMutex;
            ConnectionStatusObserverInterface::Status m_connectionStatus;
            ConnectionStatusObserverInterface::ChangedReason m_connectionReason;
            bool m_isEnabled;
            vector<shared_ptr<TransportInterface>> m_transports;
            shared_ptr<TransportInterface> m_activeTransport;
            shared_ptr<AttachmentManager> m_attachmentManager;
            shared_ptr<TransportFactoryInterface> m_transportFactory;
            shared_ptr<SynchronizedMessageRequestQueue> m_requestQueue;
        protected:
            Executor m_executor;
        };
    }
}
#endif