#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_ABSTRACTAVSCONNECTIONMANAGER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_ABSTRACTAVSCONNECTIONMANAGER_H_

#include <memory>
#include <mutex>
#include <unordered_set>
#include <sdkinterfaces/AVSConnectionManagerInterface.h>
#include <sdkinterfaces/ConnectionStatusObserverInterface.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            using namespace std;
            using namespace sdkInterfaces;
            class AbstractAVSConnectionManager : public AVSConnectionManagerInterface {
                public:
                    using ConnectionStatusObserverInterface = ConnectionStatusObserverInterface;
                    AbstractAVSConnectionManager(unordered_set<shared_ptr<ConnectionStatusObserverInterface>> observers = unordered_set<shared_ptr<ConnectionStatusObserverInterface>>());
                    virtual ~AbstractAVSConnectionManager() = default;
                    void addConnectionStatusObserver(shared_ptr<ConnectionStatusObserverInterface> observer);
                    void removeConnectionStatusObserver(shared_ptr<ConnectionStatusObserverInterface> observer);
                protected:
                    void updateConnectionStatus(ConnectionStatusObserverInterface::Status status, ConnectionStatusObserverInterface::ChangedReason reason);
                    void notifyObservers();
                    void clearObservers();
                    mutex m_mutex;
                    ConnectionStatusObserverInterface::Status m_connectionStatus;
                    ConnectionStatusObserverInterface::ChangedReason m_connectionChangedReason;
                    std::unordered_set<std::shared_ptr<ConnectionStatusObserverInterface>> m_connectionStatusObservers;
            };
        }
    }
}
#endif