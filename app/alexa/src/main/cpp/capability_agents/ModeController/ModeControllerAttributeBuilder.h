#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_MODECONTROLLER_INCLUDE_MODECONTROLLER_MODECONTROLLERATTRIBUTEBUILDER_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_MODECONTROLLER_INCLUDE_MODECONTROLLER_MODECONTROLLERATTRIBUTEBUILDER_H_

#include <unordered_map>
#include <sdkinterfaces/ModeController/ModeControllerAttributeBuilderInterface.h>
#include <util/Optional.h>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace modeController {
            using namespace std;
            using namespace avsCommon;
            using namespace avs;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace sdkInterfaces::modeController;
            class ModeControllerAttributeBuilder : public ModeControllerAttributeBuilderInterface {
            public:
                virtual ~ModeControllerAttributeBuilder() = default;
                static unique_ptr<ModeControllerAttributeBuilder> create();
                ModeControllerAttributeBuilder& withCapabilityResources(const CapabilityResources& capabilityResources) override;
                ModeControllerAttributeBuilder& addMode(const string& mode, const ModeResources& modeResources) override;
                ModeControllerAttributeBuilder& setOrdered(bool ordered) override;
                Optional<ModeControllerAttributes> build() override;
            private:
                ModeControllerAttributeBuilder();
                bool m_invalidAttribute;
                CapabilityResources m_capabilityResources;
                unordered_map<string, ModeResources> m_modes;
                bool m_ordered;
            };
        }
    }
}
#endif