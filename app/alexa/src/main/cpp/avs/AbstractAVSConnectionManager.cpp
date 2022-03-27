#include "AbstractAVSConnectionManager.h"
#include "logger/Logger.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            using namespace avsCommon::sdkInterfaces;
            static const std::string TAG("AbstractAVSConnectionManager");
            #define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)
            AbstractAVSConnectionManager::AbstractAVSConnectionManager(std::unordered_set<std::shared_ptr<ConnectionStatusObserverInterface>> observers) :
                    m_connectionStatus{ConnectionStatusObserverInterface::Status::DISCONNECTED},
                    m_connectionChangedReason{ConnectionStatusObserverInterface::ChangedReason::ACL_CLIENT_REQUEST},
                    m_connectionStatusObservers{observers} {}
            void AbstractAVSConnectionManager::addConnectionStatusObserver(std::shared_ptr<ConnectionStatusObserverInterface> observer) {
                if (!observer) {
                    ACSDK_ERROR(LX("addConnectionStatusObserverFailed").d("reason", "nullObserver"));
                    return;
                }
                std::unique_lock<std::mutex> lock{m_mutex};
                auto localStatus = m_connectionStatus;
                auto localReason = m_connectionChangedReason;
                bool addedOk = m_connectionStatusObservers.insert(observer).second;
                lock.unlock();
                if (addedOk) observer->onConnectionStatusChanged(localStatus, localReason);
            }
            void AbstractAVSConnectionManager::removeConnectionStatusObserver(std::shared_ptr<ConnectionStatusObserverInterface> observer) {
                if (!observer) {
                    ACSDK_ERROR(LX("removeConnectionStatusObserverFailed").d("reason", "nullObserver"));
                    return;
                }
                std::lock_guard<std::mutex> lock{m_mutex};
                m_connectionStatusObservers.erase(observer);
            }
            void AbstractAVSConnectionManager::updateConnectionStatus(ConnectionStatusObserverInterface::Status status, ConnectionStatusObserverInterface::ChangedReason reason) {
                std::unique_lock<std::mutex> lock{m_mutex};
                m_connectionStatus = status;
                m_connectionChangedReason = reason;
                lock.unlock();
                notifyObservers();
            }
            void AbstractAVSConnectionManager::notifyObservers() {
                std::unique_lock<std::mutex> lock{m_mutex};
                auto observers = m_connectionStatusObservers;
                auto localStatus = m_connectionStatus;
                auto localReason = m_connectionChangedReason;
                lock.unlock();
                for (auto observer : observers) {
                    observer->onConnectionStatusChanged(localStatus, localReason);
                }
            }
            void AbstractAVSConnectionManager::clearObservers() {
                std::lock_guard<std::mutex> lock{m_mutex};
                m_connectionStatusObservers.clear();
            }
        }
    }
}
