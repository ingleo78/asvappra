#ifndef ALEXA_CLIENT_SDK_CAPABILITIESDELEGATE_INCLUDE_CAPABILITIESDELEGATE_DISCOVERYSTATUSOBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_CAPABILITIESDELEGATE_INCLUDE_CAPABILITIESDELEGATE_DISCOVERYSTATUSOBSERVERINTERFACE_H_

#include <string>
#include <unordered_map>
#include <sdkinterfaces/MessageRequestObserverInterface.h>

namespace alexaClientSDK {
    namespace capabilitiesDelegate {
        using namespace std;
        using namespace avsCommon;
        using namespace sdkInterfaces;
        class DiscoveryStatusObserverInterface {
        public:
            virtual ~DiscoveryStatusObserverInterface() = default;
            virtual void onDiscoveryCompleted(
                const unordered_map<string, string>& addOrUpdateReportEndpoints,
                const unordered_map<string, string>& deleteReportEndpoints) = 0;
            virtual void onDiscoveryFailure(MessageRequestObserverInterface::Status status) = 0;
        };
    }
}
#endif