#ifndef ALEXA_CLIENT_SDK_CAPABILITYAGENTS_RANGECONTROLLER_INCLUDE_RANGECONTROLLER_RANGECONTROLLERATTRIBUTEBUILDER_H_
#define ALEXA_CLIENT_SDK_CAPABILITYAGENTS_RANGECONTROLLER_INCLUDE_RANGECONTROLLER_RANGECONTROLLERATTRIBUTEBUILDER_H_

#include <sdkinterfaces/RangeController/RangeControllerAttributeBuilderInterface.h>
#include <util/Optional.h>

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace rangeController {
            using namespace std;
            using namespace chrono;
            using namespace avsCommon;
            using namespace avs;
            using namespace resources;
            using namespace sdkInterfaces;
            using namespace utils;
            using namespace logger;
            using namespace sdkInterfaces::rangeController;
            class RangeControllerAttributeBuilder : public RangeControllerAttributeBuilderInterface {
            public:
                virtual ~RangeControllerAttributeBuilder() = default;
                static unique_ptr<RangeControllerAttributeBuilder> create();
                RangeControllerAttributeBuilder& withCapabilityResources(const CapabilityResources& capabilityResources) override;
                RangeControllerAttributeBuilder& withUnitOfMeasure(const AlexaUnitOfMeasure& unitOfMeasure) override;
                RangeControllerAttributeBuilder& addPreset(const pair<double, PresetResources>& preset) override;
                Optional<RangeControllerAttributes> build() override;
            private:
                RangeControllerAttributeBuilder();
                bool m_invalidAttribute;
                CapabilityResources m_capabilityResources;
                Optional<AlexaUnitOfMeasure> m_unitOfMeasure;
                vector<pair<double, PresetResources>> m_presets;
            };
        }
    }
}
#endif