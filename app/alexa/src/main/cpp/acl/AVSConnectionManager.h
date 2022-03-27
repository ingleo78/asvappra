#ifndef ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_AVSCONNECTIONMANAGER_H_
#define ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_AVSCONNECTIONMANAGER_H_

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <avs/AbstractAVSConnectionManager.h>
#include <avs/MessageRequest.h>
#include <sdkinterfaces/AVSGatewayAssignerInterface.h>
#include <sdkinterfaces/ConnectionStatusObserverInterface.h>
#include <sdkinterfaces/InternetConnectionMonitorInterface.h>
#include <sdkinterfaces/MessageObserverInterface.h>
#include <sdkinterfaces/MessageSenderInterface.h>
#include <util/RequiresShutdown.h>
#include "Transport/MessageRouterInterface.h"
#include "Transport/MessageRouterObserverInterface.h"

namespace alexaClientSDK {
    namespace acl {
        using namespace std;
        using namespace avsCommon;
        using namespace avs;
        using namespace sdkInterfaces;
        using namespace utils;
        class AVSConnectionManager : public AbstractAVSConnectionManager, public MessageSenderInterface, public AVSGatewayAssignerInterface,
                                     public MessageRouterObserverInterface, public InternetConnectionObserverInterface, public RequiresShutdown,
                                     public enable_shared_from_this<AVSConnectionManager> {
        public:
            static shared_ptr<AVSConnectionManager> create(shared_ptr<MessageRouterInterface> messageRouter, bool isEnabled = true,
                unordered_set<shared_ptr<ConnectionStatusObserverInterface>> connectionStatusObservers = unordered_set<shared_ptr<ConnectionStatusObserverInterface>>(),
                unordered_set<shared_ptr<MessageObserverInterface>> messageObservers = unordered_set<shared_ptr<MessageObserverInterface>>(),
                shared_ptr<InternetConnectionMonitorInterface> internetConnectionMonitor = nullptr);
            void enable() override;
            void disable() override;
            bool isEnabled() override;
            void reconnect() override;
            bool isConnected() const override;
            void onWakeConnectionRetry() override;
            void addMessageObserver(shared_ptr<MessageObserverInterface> observer) override;
            void removeMessageObserver(shared_ptr<MessageObserverInterface> observer) override;
            void sendMessage(shared_ptr<MessageRequest> request) override;
            void setAVSGateway(const string& avsGateway) override;
            string getAVSGateway();
            void onConnectionStatusChanged(bool connected) override;
        private:
            AVSConnectionManager(shared_ptr<MessageRouterInterface> messageRouter,
                unordered_set<shared_ptr<ConnectionStatusObserverInterface>> connectionStatusObservers = unordered_set<shared_ptr<ConnectionStatusObserverInterface>>(),
                unordered_set<shared_ptr<MessageObserverInterface>> messageObserver = unordered_set<shared_ptr<MessageObserverInterface>>(),
                shared_ptr<InternetConnectionMonitorInterface> internetConnectionMonitor = nullptr);
            void doShutdown() override;
            void onConnectionStatusChanged(const ConnectionStatusObserverInterface::Status status,
                                           const ConnectionStatusObserverInterface::ChangedReason reason) override;
            void receive(const string& contextId, const string& message) override;
            mutex m_isEnabledMutex;
            bool m_isEnabled;
            unordered_set<shared_ptr<MessageObserverInterface>> m_messageObservers;
            mutex m_messageObserverMutex;
            shared_ptr<MessageRouterInterface> m_messageRouter;
            shared_ptr<InternetConnectionMonitorInterface> m_internetConnectionMonitor;
        };
    }
}
#endif