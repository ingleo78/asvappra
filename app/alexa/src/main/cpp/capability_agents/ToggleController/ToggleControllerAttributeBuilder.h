#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_TOGGLECONTROLLER_INCLUDE_TOGGLECONTROLLER_TOGGLECONTROLLERATTRIBUTEBUILDER_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_TOGGLECONTROLLER_INCLUDE_TOGGLECONTROLLER_TOGGLECONTROLLERATTRIBUTEBUILDER_H_

#include <sdkinterfaces/ToggleController/ToggleControllerAttributeBuilderInterface.h>
#include <util/Optional.h>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace toggleController {
            using namespace std;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace logger;
            using namespace sdkInterfaces::toggleController;
            class ToggleControllerAttributeBuilder : public ToggleControllerAttributeBuilderInterface {
            public:
                virtual ~ToggleControllerAttributeBuilder() = default;
                static unique_ptr<ToggleControllerAttributeBuilder> create();
                ToggleControllerAttributeBuilder& withCapabilityResources(const CapabilityResources& capabilityResources) override;
                Optional<ToggleControllerAttributes> build() override;
            private:
                ToggleControllerAttributeBuilder();
                bool m_invalidAttribute;
                CapabilityResources m_capabilityResources;
            };
        }
    }
}
#endif