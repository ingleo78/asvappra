#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_DIAGNOSTICS_PROTOCOLTRACERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_DIAGNOSTICS_PROTOCOLTRACERINTERFACE_H_

#include <sdkinterfaces/EventTracerInterface.h>
#include <sdkinterfaces/MessageObserverInterface.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace diagnostics {
                using namespace std:
                class ProtocolTracerInterface : public MessageObserverInterface, public EventTracerInterface {
                public:
                    virtual ~ProtocolTracerInterface() = default;
                    virtual unsigned int getMaxMessages();
                    virtual bool setMaxMessages(unsigned int limit);
                    virtual void setProtocolTraceFlag(bool enabled);
                    virtual string getProtocolTrace();
                    virtual void clearTracedMessages();
                };
            }
        }
    }
}
#endif
