#include <logger/Logger.h>
#include "ToggleControllerAttributeBuilder.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace toggleController {
            static const string TAG{"ToggleControllerAttributeBuilder"};
            #define LX(event) LogEntry(TAG, event)
            unique_ptr<ToggleControllerAttributeBuilder> ToggleControllerAttributeBuilder::create() {
                return unique_ptr<ToggleControllerAttributeBuilder>(new ToggleControllerAttributeBuilder());
            }
            ToggleControllerAttributeBuilder::ToggleControllerAttributeBuilder() : m_invalidAttribute{false} {}
            ToggleControllerAttributeBuilder& ToggleControllerAttributeBuilder::withCapabilityResources(const CapabilityResources& capabilityResources) {
                ACSDK_DEBUG5(LX(__func__));
                if (!capabilityResources.isValid()) {
                    ACSDK_ERROR(LX("withCapabilityResourcesFailed").d("reason", "invalidCapabilityResources"));
                    m_invalidAttribute = true;
                    return *this;
                }
                m_capabilityResources = capabilityResources;
                return *this;
            }
            Optional<ToggleControllerAttributes> ToggleControllerAttributeBuilder::build() {
                ACSDK_DEBUG5(LX(__func__));
                if (m_invalidAttribute) {
                    ACSDK_ERROR(LX("buildFailed").d("reason", "invalidAttribute"));
                    return Optional<ToggleControllerAttributes>();
                }
                ACSDK_DEBUG5(LX(__func__).sensitive("capabilityResources", m_capabilityResources.toJson()));
                return Optional<ToggleControllerAttributes>({m_capabilityResources});
            }
        }
    }
}