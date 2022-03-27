#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_ALEXA_INCLUDE_ALEXA_ALEXAINTERFACEMESSAGESENDERINTERNALINTERFACE_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_ALEXA_INCLUDE_ALEXA_ALEXAINTERFACEMESSAGESENDERINTERNALINTERFACE_H_

#include <sdkinterfaces/AlexaInterfaceMessageSenderInterface.h>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace alexa {
            using namespace std;
            using namespace avsCommon;
            using namespace sdkInterfaces;
            class AlexaInterfaceMessageSenderInternalInterface : public AlexaInterfaceMessageSenderInterface {
            public:
                virtual ~AlexaInterfaceMessageSenderInternalInterface() = default;
                virtual bool sendStateReportEvent(const string& instance, const string& correlationToken, const AVSMessageEndpoint& endpoint);
            };
        }
    }
}
#endif