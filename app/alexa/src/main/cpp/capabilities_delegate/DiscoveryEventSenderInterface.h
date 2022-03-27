#ifndef ALEXA_CLIENT_SDK_CAPABILITIESDELEGATE_INCLUDE_CAPABILITIESDELEGATE_DISCOVERYEVENTSENDERINTERFACE_H_
#define ALEXA_CLIENT_SDK_CAPABILITIESDELEGATE_INCLUDE_CAPABILITIESDELEGATE_DISCOVERYEVENTSENDERINTERFACE_H_

#include <sdkinterfaces/MessageSenderInterface.h>
#include "DiscoveryStatusObserverInterface.h"

namespace alexaClientSDK {
    namespace capabilitiesDelegate {
        using namespace std;
        using namespace avsCommon;
        using namespace sdkInterfaces;
        class DiscoveryEventSenderInterface : public AlexaEventProcessedObserverInterface, public AuthObserverInterface {
        public:
            virtual bool sendDiscoveryEvents(const shared_ptr<MessageSenderInterface>& messageSender) = 0;
            virtual void stop() = 0;
            virtual void addDiscoveryStatusObserver(const shared_ptr<DiscoveryStatusObserverInterface>& observer) = 0;
            virtual void removeDiscoveryStatusObserver(const shared_ptr<DiscoveryStatusObserverInterface>& observer) = 0;
        };
    }
}
#endif