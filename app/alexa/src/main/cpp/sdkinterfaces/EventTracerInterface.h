#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_EVENTTRACERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_EVENTTRACERINTERFACE_H_

#include <string>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class EventTracerInterface {
            public:
                virtual ~EventTracerInterface() = default;
                virtual void traceEvent(const std::string& messageContent);
            };
        }
    }
}
#endif