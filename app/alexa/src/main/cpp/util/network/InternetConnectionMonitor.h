#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_NETWORK_INTERNETCONNECTIONMONITOR_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_NETWORK_INTERNETCONNECTIONMONITOR_H_

#include <chrono>
#include <memory>
#include <mutex>
#include <unordered_set>
#include <avs/attachment/InProcessAttachment.h>
#include <avs/attachment/InProcessAttachmentReader.h>
#include <sdkinterfaces/HTTPContentFetcherInterfaceFactoryInterface.h>
#include <sdkinterfaces/InternetConnectionMonitorInterface.h>
#include <sdkinterfaces/InternetConnectionObserverInterface.h>
#include <timing/Timer.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace network {
                using namespace std;
                using namespace chrono;
                using namespace timing;
                using namespace sdkInterfaces;
                class InternetConnectionMonitor : public InternetConnectionMonitorInterface {
                public:
                    static unique_ptr<InternetConnectionMonitor> create(shared_ptr<HTTPContentFetcherInterfaceFactoryInterface> contentFetcherFactory);
                    virtual ~InternetConnectionMonitor();
                    void addInternetConnectionObserver(shared_ptr<InternetConnectionObserverInterface> observer) override;
                    void removeInternetConnectionObserver(shared_ptr<InternetConnectionObserverInterface> observer) override;
                private:
                    InternetConnectionMonitor(shared_ptr<HTTPContentFetcherInterfaceFactoryInterface> contentFetcherFactory);
                    void startMonitoring();
                    void stopMonitoring();
                    void testConnection();
                    void updateConnectionStatus(bool connected);
                    void notifyObserversLocked();
                    unordered_set<shared_ptr<InternetConnectionObserverInterface>> m_observers;
                    bool m_connected;
                    seconds m_period;
                    Timer m_connectionTestTimer;
                    atomic<bool> m_isShuttingDown;
                    shared_ptr<HTTPContentFetcherInterfaceFactoryInterface> m_contentFetcherFactory;
                    mutex m_mutex;
                };
            }
        }
    }
}
#endif
