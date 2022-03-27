#include <algorithm>
#include <curl/curl.h>
#include <logger/Logger.h>
#include <memory/Memory.h>
#include <threading/Executor.h>
#include "MessageRouter.h"

namespace alexaClientSDK {
    namespace acl {
        using namespace std;

        using namespace avsCommon;
        using namespace avs;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace attachment;
        using namespace logger;
        static const std::string TAG("MessageRouter");
        static constexpr const char* KEY_SIZEOF_TRANSPORTS = "sizeOf m_transports";
        #define LX(event) LogEntry(TAG, event)
        MessageRouter::MessageRouter(shared_ptr<AuthDelegateInterface> authDelegate, shared_ptr<AttachmentManager> attachmentManager,
                                     shared_ptr<TransportFactoryInterface> transportFactory, const string& avsGateway) :
                                     MessageRouterInterface{"MessageRouter"}, m_avsGateway{avsGateway}, m_authDelegate{authDelegate},
                                     m_connectionStatus{ConnectionStatusObserverInterface::Status::DISCONNECTED},
                                     m_connectionReason{ConnectionStatusObserverInterface::ChangedReason::ACL_CLIENT_REQUEST},
                                     m_isEnabled{false}, m_attachmentManager{attachmentManager}, m_transportFactory{transportFactory},
                                     m_requestQueue{make_shared<SynchronizedMessageRequestQueue>()} {}
        MessageRouterInterface::ConnectionStatus MessageRouter::getConnectionStatus() {
            lock_guard<mutex> lock{m_connectionMutex};
            return make_pair(m_connectionStatus, m_connectionReason);
        }
        void MessageRouter::enable() {
            ACSDK_INFO(LX(__func__));
            lock_guard<mutex> lock{m_connectionMutex};
            if (m_isEnabled) {
                ACSDK_INFO(LX(__func__).m("already enabled"));
                return;
            }
            if (m_activeTransport != nullptr) {
                ACSDK_ERROR(LX("enableFailed").d("reason", "activeTransportNotNull"));
                return;
            }
            m_isEnabled = true;
            setConnectionStatusLocked(ConnectionStatusObserverInterface::Status::PENDING,
                                      ConnectionStatusObserverInterface::ChangedReason::ACL_CLIENT_REQUEST);
            createActiveTransportLocked();
        }
        void MessageRouter::doShutdown() {
            disable();
            unique_lock<std::mutex> lock{m_connectionMutex};
            if (!m_requestQueue->empty()) {
                auto request = m_requestQueue->dequeueOldestRequest();
                if (request != nullptr) request->sendCompleted(MessageRequestObserverInterface::Status::NOT_CONNECTED);
                m_requestQueue->clear();
            }
            lock.unlock();
            m_executor.shutdown();
        }
        void MessageRouter::disable() {
            ACSDK_INFO(LX(__func__));
            unique_lock<std::mutex> lock{m_connectionMutex};
            m_isEnabled = false;
            disconnectAllTransportsLocked(lock, ConnectionStatusObserverInterface::ChangedReason::ACL_CLIENT_REQUEST);
        }
        void MessageRouter::sendMessage(shared_ptr<MessageRequest> request) {
            if (!request) {
                ACSDK_ERROR(LX("sendFailed").d("reason", "nullRequest"));
                return;
            }
            unique_lock<std::mutex> lock{m_connectionMutex};
            if (m_activeTransport) {
                m_requestQueue->enqueueRequest(request);
                m_activeTransport->onRequestEnqueued();
            } else {
                ACSDK_ERROR(LX("sendFailed").d("reason", "noActiveTransport"));
                request->sendCompleted(MessageRequestObserverInterface::Status::NOT_CONNECTED);
            }
        }
        void MessageRouter::setAVSGateway(const string& avsGateway) {
            ACSDK_INFO(LX(__func__).d("avsGateway", avsGateway));
            unique_lock<mutex> lock{m_connectionMutex};
            if (avsGateway != m_avsGateway) {
                m_avsGateway = avsGateway;
                if (m_isEnabled) { disconnectAllTransportsLocked(lock, ConnectionStatusObserverInterface::ChangedReason::SERVER_ENDPOINT_CHANGED); }
                if (m_isEnabled) createActiveTransportLocked();
            }
        }
        string MessageRouter::getAVSGateway() {
            lock_guard<std::mutex> lock{m_connectionMutex};
            return m_avsGateway;
        }
        void MessageRouter::onWakeConnectionRetry() {
            ACSDK_INFO(LX(__func__));
            lock_guard<mutex> lock{m_connectionMutex};
            if (m_isEnabled && m_activeTransport) {
                ACSDK_INFO(LX(__func__).p("m_activeTransport", m_activeTransport));
                m_activeTransport->onWakeConnectionRetry();
            }
        }
        void MessageRouter::onWakeVerifyConnectivity() {
            ACSDK_INFO(LX(__func__));
            lock_guard<mutex> lock{m_connectionMutex};
            if (m_isEnabled && m_activeTransport) {
                ACSDK_INFO(LX(__func__).p("m_activeTransport", m_activeTransport));
                m_activeTransport->onWakeVerifyConnectivity();
            }
        }
        void MessageRouter::onConnected(shared_ptr<TransportInterface> transport) {
            unique_lock<mutex> lock{m_connectionMutex};
            ACSDK_INFO(LX(__func__).p("transport", transport).p("m_activeTransport", m_activeTransport));
            if (!m_isEnabled) {
                ACSDK_DEBUG0(LX("onConnectedWhenDisabled"));
                return;
            }
            if (transport != m_activeTransport) {
                ACSDK_DEBUG0(LX("onInactiveTransportConnected"));
                return;
            }
            setConnectionStatusLocked(ConnectionStatusObserverInterface::Status::CONNECTED,
                               ConnectionStatusObserverInterface::ChangedReason::ACL_CLIENT_REQUEST);
        }
        void MessageRouter::onDisconnected(shared_ptr<TransportInterface> transport, ConnectionStatusObserverInterface::ChangedReason reason) {
            lock_guard<mutex> lock{m_connectionMutex};
            ACSDK_INFO(LX(__func__).p("transport", transport).p("m_activeTransport", m_activeTransport)
                           .d(KEY_SIZEOF_TRANSPORTS, m_transports.size()).d("reason", reason));
            for (auto it = m_transports.begin(); it != m_transports.end(); it++) {
                if (*it == transport) {
                    m_transports.erase(it);
                    ACSDK_INFO(LX("releaseTransport").p("transport", transport).d(KEY_SIZEOF_TRANSPORTS, m_transports.size()));
                    safelyReleaseTransport(transport);
                    break;
                }
            }
            if (transport == m_activeTransport) {
                m_activeTransport.reset();
                switch(m_connectionStatus) {
                    case ConnectionStatusObserverInterface::Status::PENDING: case ConnectionStatusObserverInterface::Status::CONNECTED:
                        if (m_isEnabled && reason != ConnectionStatusObserverInterface::ChangedReason::UNRECOVERABLE_ERROR) {
                            setConnectionStatusLocked(ConnectionStatusObserverInterface::Status::PENDING, reason);
                            createActiveTransportLocked();
                        } else if (m_transports.empty()) setConnectionStatusLocked(ConnectionStatusObserverInterface::Status::DISCONNECTED, reason);
                        return;
                    case ConnectionStatusObserverInterface::Status::DISCONNECTED: return;
                }
                ACSDK_ERROR(LX("unhandledConnectionStatus").d("connectionStatus", static_cast<int>(m_connectionStatus)));
            }
        }
        void MessageRouter::onServerSideDisconnect(shared_ptr<TransportInterface> transport) {
            unique_lock<mutex> lock{m_connectionMutex};
            ACSDK_INFO(LX("server-side disconnect received").d("Message router is enabled", m_isEnabled).p("transport", transport)
                           .p("m_activeTransport", m_activeTransport));
            if (m_isEnabled && transport == m_activeTransport) {
                setConnectionStatusLocked(ConnectionStatusObserverInterface::Status::PENDING,
                                   ConnectionStatusObserverInterface::ChangedReason::SERVER_SIDE_DISCONNECT);
                createActiveTransportLocked();
            }
        }
        void MessageRouter::consumeMessage(const string& contextId, const string& message) {
            notifyObserverOnReceive(contextId, message);
        }
        void MessageRouter::setObserver(shared_ptr<MessageRouterObserverInterface> observer) {
            lock_guard<mutex> lock{m_connectionMutex};
            m_observer = observer;
        }
        void MessageRouter::setConnectionStatusLocked(
            const ConnectionStatusObserverInterface::Status status,
            const ConnectionStatusObserverInterface::ChangedReason reason) {
            ACSDK_INFO(LX(__func__).d("status", status).d("reason", reason));
            if (status != m_connectionStatus) {
                m_connectionStatus = status;
                m_connectionReason = reason;
                ACSDK_DEBUG(LX("connectionStatusChanged").d("reason", reason).d("newStatus", m_connectionStatus));
                notifyObserverOnConnectionStatusChanged(m_connectionStatus, reason);
            }
        }
        void MessageRouter::notifyObserverOnConnectionStatusChanged(
            const ConnectionStatusObserverInterface::Status status,
            const ConnectionStatusObserverInterface::ChangedReason reason) {
            auto task = [this, status, reason]() {
                auto observer = getObserver();
                if (observer) observer->onConnectionStatusChanged(status, reason);
            };
            m_executor.submit(task);
        }
        void MessageRouter::notifyObserverOnReceive(const std::string& contextId, const std::string& message) {
            auto task = [this, contextId, message]() {
                auto temp = getObserver();
                if (temp) temp->receive(contextId, message);
            };
            m_executor.submit(task);
        }
        void MessageRouter::createActiveTransportLocked() {
            auto transport = m_transportFactory->createTransport(
                m_authDelegate, m_attachmentManager, m_avsGateway, shared_from_this(), shared_from_this(), m_requestQueue);
            ACSDK_INFO(LX(__func__).p("transport", transport).d(KEY_SIZEOF_TRANSPORTS, m_transports.size()));
            if (transport && transport->connect()) {
                m_transports.push_back(transport);
                m_activeTransport = transport;
                ACSDK_INFO(LX("setAsActiveTransport").p("transport", transport).d(KEY_SIZEOF_TRANSPORTS, m_transports.size()));
                if (m_transports.size() > 2) ACSDK_WARN(LX("tooManyTransports").d(KEY_SIZEOF_TRANSPORTS, m_transports.size()));
            } else {
                safelyResetActiveTransportLocked();
                setConnectionStatusLocked(ConnectionStatusObserverInterface::Status::DISCONNECTED,
                                   ConnectionStatusObserverInterface::ChangedReason::INTERNAL_ERROR);
                ACSDK_ERROR(LX("createActiveTransportLockedFailed").d("reason", transport ? "internalError" : "createTransportFailed"));
            }
        }
        void MessageRouter::disconnectAllTransportsLocked(unique_lock<std::mutex>& lock, const ConnectionStatusObserverInterface::ChangedReason reason) {
            ACSDK_INFO(LX(__func__).d("reason", reason).d(KEY_SIZEOF_TRANSPORTS, m_transports.size()).p("m_activeTransport", m_activeTransport));
            safelyResetActiveTransportLocked();
            auto movedTransports = std::move(m_transports);
            m_transports.clear();
            setConnectionStatusLocked(ConnectionStatusObserverInterface::Status::DISCONNECTED, reason);
            lock.unlock();
            for (auto transport : movedTransports) {
                ACSDK_INFO(LX(__func__).p("transport", transport));
                transport->shutdown();
            }
            lock.lock();
        }
        shared_ptr<MessageRouterObserverInterface> MessageRouter::getObserver() {
            lock_guard<std::mutex> lock{m_connectionMutex};
            return m_observer;
        }
        void MessageRouter::safelyResetActiveTransportLocked() {
            if (m_activeTransport) {
                if (find(m_transports.begin(), m_transports.end(), m_activeTransport) == m_transports.end()) {
                    ACSDK_ERROR(LX("safelyResetActiveTransportLockedError").d("reason", "activeTransportNotInTransports)"));
                    safelyReleaseTransport(m_activeTransport);
                }
                ACSDK_INFO(LX("clearActiveTransport").p("m_activeTransport", m_activeTransport));
                m_activeTransport.reset();
            }
        }
        void MessageRouter::safelyReleaseTransport(shared_ptr<TransportInterface> transport) {
            if (transport) {
                auto task = [transport]() { transport->shutdown(); };
                m_executor.submit(task);
            }
        }
    }
}