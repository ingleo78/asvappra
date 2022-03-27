#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_ALEXAEVENTPROCESSEDOBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_ALEXAEVENTPROCESSEDOBSERVERINTERFACE_H_

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class AlexaEventProcessedObserverInterface {
            public:
                virtual ~AlexaEventProcessedObserverInterface() = default;
                virtual void onAlexaEventProcessedReceived(const std::string& eventCorrelationToken) = 0;
            };
        }
    }
}
#endif