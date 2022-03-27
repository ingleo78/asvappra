#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_INTERACTIONMODELREQUESTPROCESSINGOBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_INTERACTIONMODELREQUESTPROCESSINGOBSERVERINTERFACE_H_

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class InteractionModelRequestProcessingObserverInterface {
            public:
                virtual ~InteractionModelRequestProcessingObserverInterface() = default;
                virtual void onRequestProcessingStarted();
                virtual void onRequestProcessingCompleted();
            };
        }
    }
}
#endif