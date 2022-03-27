#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_COMPONENTREPORTERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_COMPONENTREPORTERINTERFACE_H_

#include <memory>
#include <avs/ComponentConfiguration.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class ComponentReporterInterface {
            public:
                virtual ~ComponentReporterInterface() = default;
                virtual bool addConfiguration(std::shared_ptr<avsCommon::avs::ComponentConfiguration> configuration) = 0;
            };
        }
    }
}
#endif