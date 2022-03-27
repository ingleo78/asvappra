#ifndef ALEXA_CLIENT_SDK_APPLICATIONUTILITIES_SDKCOMPONENT_INCLUDE_SDKCOMPONENT_SDKCOMPONENT_H_
#define ALEXA_CLIENT_SDK_APPLICATIONUTILITIES_SDKCOMPONENT_INCLUDE_SDKCOMPONENT_SDKCOMPONENT_H_

#include <memory>
#include <avs/ComponentConfiguration.h>
#include <sdkinterfaces/ComponentReporterInterface.h>

namespace alexaClientSDK {
    namespace applicationUtilities {
        namespace SDKComponent {
            class SDKComponent {
            public:
                static bool registerComponent(std::shared_ptr<avsCommon::sdkInterfaces::ComponentReporterInterface> componentReporter);
            };
        }
    }
}
#endif