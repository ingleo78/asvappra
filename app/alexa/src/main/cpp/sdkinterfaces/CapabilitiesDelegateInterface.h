#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_CAPABILITIESDELEGATEINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_CAPABILITIESDELEGATEINTERFACE_H_

#include <memory>
#include <vector>
#include <avs/AVSDiscoveryEndpointAttributes.h>
#include <avs/CapabilityConfiguration.h>
#include "AlexaEventProcessedObserverInterface.h"
#include "AVSGatewayObserverInterface.h"
#include "CapabilityConfigurationInterface.h"
#include "CapabilitiesObserverInterface.h"
#include "ConnectionStatusObserverInterface.h"
#include "PostConnectOperationInterface.h"
#include "MessageSenderInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            using namespace avs;
            using namespace std;
            class CapabilitiesDelegateInterface : public AlexaEventProcessedObserverInterface, public AVSGatewayObserverInterface,
                                                  public ConnectionStatusObserverInterface {
            public:
                virtual ~CapabilitiesDelegateInterface() = default;
                virtual bool addOrUpdateEndpoint(const AVSDiscoveryEndpointAttributes& endpointAttributes, const vector<CapabilityConfiguration>& capabilities) = 0;
                virtual bool deleteEndpoint(const AVSDiscoveryEndpointAttributes& endpointAttributes, const vector<CapabilityConfiguration>& capabilities) = 0;
                virtual void addCapabilitiesObserver(shared_ptr<CapabilitiesObserverInterface> observer) = 0;
                virtual void removeCapabilitiesObserver(shared_ptr<CapabilitiesObserverInterface> observer) = 0;
                virtual void invalidateCapabilities() = 0;
                virtual void setMessageSender(const shared_ptr<MessageSenderInterface>& messageSender) = 0;
            };
        }
    }
}
#endif