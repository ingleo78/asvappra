#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_DIRECTIVEHANDLERRESULTINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_DIRECTIVEHANDLERRESULTINTERFACE_H_

#include <memory>
#include <avs/AVSDirective.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class DirectiveHandlerResultInterface {
            public:
                virtual ~DirectiveHandlerResultInterface() = default;
                virtual void setCompleted();
                virtual void setFailed(const std::string& description);
            };
        }
    }
}
#endif