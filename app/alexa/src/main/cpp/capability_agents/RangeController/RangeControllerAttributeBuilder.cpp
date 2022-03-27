#include <cmath>
#include <logger/Logger.h>
#include "RangeControllerAttributeBuilder.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace rangeController {
            static const string TAG{"RangeControllerAttributeBuilder"};
            #define LX(event) LogEntry(TAG, event)
            unique_ptr<RangeControllerAttributeBuilder> RangeControllerAttributeBuilder::create() {
                return unique_ptr<RangeControllerAttributeBuilder>(new RangeControllerAttributeBuilder());
            }
            RangeControllerAttributeBuilder::RangeControllerAttributeBuilder() : m_invalidAttribute{false},
                                                                                 m_unitOfMeasure{Optional<resources::AlexaUnitOfMeasure>()} {}
            RangeControllerAttributeBuilder& RangeControllerAttributeBuilder::withCapabilityResources(const CapabilityResources& capabilityResources) {
                ACSDK_DEBUG5(LX(__func__));
                if (!capabilityResources.isValid()) {
                    ACSDK_ERROR(LX("withCapabilityResourcesFailed").d("reason", "invalidCapabilityResources"));
                    m_invalidAttribute = true;
                    return *this;
                }
                m_capabilityResources = capabilityResources;
                return *this;
            }
            RangeControllerAttributeBuilder& RangeControllerAttributeBuilder::withUnitOfMeasure(const AlexaUnitOfMeasure& unitOfMeasure) {
                ACSDK_DEBUG5(LX(__func__));
                if (unitOfMeasure.empty()) {
                    ACSDK_ERROR(LX("withUnitOfMeasureFailed").d("reason", "invalidUnitOfMeasure"));
                    m_invalidAttribute = true;
                    return *this;
                }
                m_unitOfMeasure = Optional<resources::AlexaUnitOfMeasure>(unitOfMeasure);
                return *this;
            }
            RangeControllerAttributeBuilder& RangeControllerAttributeBuilder::addPreset(const pair<double, PresetResources>& preset) {
                ACSDK_DEBUG5(LX(__func__));
                if (!preset.second.isValid()) {
                    ACSDK_ERROR(LX("addPresetFailed").d("reason", "invalidPresetResources"));
                    m_invalidAttribute = true;
                    return *this;
                }
                ACSDK_DEBUG5(LX(__func__).sensitive("preset", preset.first).sensitive("presetResources", preset.second.toJson()));
                m_presets.push_back(preset);
                return *this;
            }
            Optional<RangeControllerAttributes> RangeControllerAttributeBuilder::build() {
                ACSDK_DEBUG5(LX(__func__));
                Optional<RangeControllerAttributes> controllerAttribute;
                if (m_invalidAttribute) {
                    ACSDK_ERROR(LX("buildFailed").d("reason", "invalidAttribute"));
                    return controllerAttribute;
                }
                ACSDK_DEBUG5(LX(__func__).sensitive("capabilityResources", m_capabilityResources.toJson()));
                ACSDK_DEBUG5(LX(__func__).sensitive("unitOfMeasure", m_unitOfMeasure.valueOr("")));
                ACSDK_DEBUG5(LX(__func__).d("#presets", m_presets.size()));
                return Optional<RangeControllerAttributes>({m_capabilityResources, m_unitOfMeasure, m_presets});
            }
        }
    }
}