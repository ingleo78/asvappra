#include <avs/Initialization/AlexaClientSDKInit.h>
#include <logger/Logger.h>
#include <memory/Memory.h>
#include "AVSConnectionManager.h"
#include "Transport/MessageRouterInterface.h"

namespace alexaClientSDK {
    namespace acl {
        using namespace std;
        using namespace avsCommon;
        using namespace avs;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace logger;
        using namespace initialization;
        static const string TAG("AVSConnectionManager");
        #define LX(event) LogEntry(TAG, event)
        shared_ptr<AVSConnectionManager> AVSConnectionManager::create(shared_ptr<MessageRouterInterface> messageRouter, bool isEnabled,
                                                                      unordered_set<shared_ptr<ConnectionStatusObserverInterface>> connectionStatusObservers,
                                                                      unordered_set<shared_ptr<MessageObserverInterface>> messageObservers,
                                                                      shared_ptr<InternetConnectionMonitorInterface> internetConnectionMonitor) {
            if (!AlexaClientSDKInit::isInitialized()) {
                ACSDK_ERROR(LX("createFailed").d("reason", "uninitialziedAlexaClientSdk").d("return", "nullptr"));
                return nullptr;
            }
            if (!messageRouter) {
                ACSDK_ERROR(LX("createFailed").d("reason", "nullMessageRouter").d("return", "nullptr"));
                return nullptr;
            }
            for (auto observer : connectionStatusObservers) {
                if (!observer) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullConnectionStatusObserver").d("return", "nullptr"));
                    return nullptr;
                }
            }
            for (auto observer : messageObservers) {
                if (!observer) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullMessageObserver").d("return", "nullptr"));
                    return nullptr;
                }
            }
            auto connectionManager = shared_ptr<AVSConnectionManager>(new AVSConnectionManager(messageRouter, connectionStatusObservers, messageObservers,
                                                                      internetConnectionMonitor));
            messageRouter->setObserver(connectionManager);
            if (isEnabled) connectionManager->enable();
            if (internetConnectionMonitor) {
                ACSDK_DEBUG5(LX(__func__).m("Subscribing to InternetConnectionMonitor Callbacks"));
                internetConnectionMonitor->addInternetConnectionObserver(connectionManager);
            }
            return connectionManager;
        }
        AVSConnectionManager::AVSConnectionManager(shared_ptr<MessageRouterInterface> messageRouter,
                                                   unordered_set<shared_ptr<ConnectionStatusObserverInterface>> connectionStatusObservers,
                                                   unordered_set<shared_ptr<MessageObserverInterface>> messageObservers,
                                                   shared_ptr<InternetConnectionMonitorInterface> internetConnectionMonitor) :
                                                   AbstractAVSConnectionManager{connectionStatusObservers}, RequiresShutdown{"AVSConnectionManager"},
                                                   m_isEnabled{false}, m_messageObservers{messageObservers}, m_messageRouter{messageRouter},
                                                   m_internetConnectionMonitor{internetConnectionMonitor} {}
        void AVSConnectionManager::doShutdown() {
            if (m_internetConnectionMonitor) m_internetConnectionMonitor->removeInternetConnectionObserver(shared_from_this());
            disable();
            clearObservers();
            {
                lock_guard<std::mutex> lock{m_messageObserverMutex};
                m_messageObservers.clear();
            }
            m_messageRouter.reset();
        }
        void AVSConnectionManager::enable() {
            lock_guard<std::mutex> lock(m_isEnabledMutex);
            ACSDK_DEBUG5(LX(__func__));
            m_isEnabled = true;
            m_messageRouter->enable();
        }
        void AVSConnectionManager::disable() {
            lock_guard<std::mutex> lock(m_isEnabledMutex);
            ACSDK_DEBUG5(LX(__func__));
            m_isEnabled = false;
            m_messageRouter->disable();
        }
        bool AVSConnectionManager::isEnabled() {
            lock_guard<std::mutex> lock(m_isEnabledMutex);
            return m_isEnabled;
        }
        void AVSConnectionManager::reconnect() {
            lock_guard<std::mutex> lock(m_isEnabledMutex);
            ACSDK_DEBUG5(LX(__func__).d("isEnabled", m_isEnabled));
            if (m_isEnabled) {
                m_messageRouter->disable();
                m_messageRouter->enable();
            }
        }
        void AVSConnectionManager::sendMessage(shared_ptr<MessageRequest> request) {
            m_messageRouter->sendMessage(request);
        }
        bool AVSConnectionManager::isConnected() const {
            return m_messageRouter->getConnectionStatus().first == ConnectionStatusObserverInterface::Status::CONNECTED;
        }
        void AVSConnectionManager::onWakeConnectionRetry() {
            ACSDK_DEBUG9(LX(__func__));
            m_messageRouter->onWakeConnectionRetry();
        }
        void AVSConnectionManager::setAVSGateway(const string& avsGateway) {
            m_messageRouter->setAVSGateway(avsGateway);
        }
        string AVSConnectionManager::getAVSGateway() {
            return m_messageRouter->getAVSGateway();
        }
        void AVSConnectionManager::onConnectionStatusChanged(bool connected) {
            ACSDK_DEBUG5(LX(__func__).d("connected", connected).d("isEnabled", m_isEnabled));
            if (m_isEnabled) {
                if (connected) m_messageRouter->onWakeConnectionRetry();
                else m_messageRouter->onWakeVerifyConnectivity();
            }
        }
        void AVSConnectionManager::addMessageObserver(
            shared_ptr<MessageObserverInterface> observer) {
            if (!observer) {
                ACSDK_ERROR(LX("addObserverFailed").d("reason", "nullObserver"));
                return;
            }
            lock_guard<mutex> lock{m_messageObserverMutex};
            m_messageObservers.insert(observer);
        }
        void AVSConnectionManager::removeMessageObserver(
            shared_ptr<MessageObserverInterface> observer) {
            if (!observer) {
                ACSDK_ERROR(LX("removeObserverFailed").d("reason", "nullObserver"));
                return;
            }
            lock_guard<mutex> lock{m_messageObserverMutex};
            m_messageObservers.erase(observer);
        }
        void AVSConnectionManager::onConnectionStatusChanged(const ConnectionStatusObserverInterface::Status status,
                                                             const ConnectionStatusObserverInterface::ChangedReason reason) {
            updateConnectionStatus(status, reason);
        }
        void AVSConnectionManager::receive(const string& contextId, const string& message) {
            unique_lock<mutex> lock{m_messageObserverMutex};
            unordered_set<shared_ptr<MessageObserverInterface>> observers{m_messageObservers};
            lock.unlock();
            for (auto observer : observers) {
                if (observer) observer->receive(contextId, message);
            }
        }
    }
}