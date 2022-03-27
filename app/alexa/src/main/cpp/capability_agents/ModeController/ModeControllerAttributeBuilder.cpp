#include <logger/Logger.h>
#include "ModeControllerAttributeBuilder.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace modeController {
            using namespace logger;
            static const string TAG{"ModeControllerAttributeBuilder"};
            #define LX(event) LogEntry(TAG, event)
            unique_ptr<ModeControllerAttributeBuilder> ModeControllerAttributeBuilder::create() {
                return unique_ptr<ModeControllerAttributeBuilder>(new ModeControllerAttributeBuilder());
            }
            ModeControllerAttributeBuilder::ModeControllerAttributeBuilder() : m_invalidAttribute{false}, m_ordered{false} {}
            ModeControllerAttributeBuilder& ModeControllerAttributeBuilder::withCapabilityResources(
                const CapabilityResources& capabilityResources) {
                ACSDK_DEBUG5(LX(__func__));
                if (!capabilityResources.isValid()) {
                    ACSDK_ERROR(LX("withCapabilityResourcesFailed").d("reason", "invalidCapabilityResources"));
                    m_invalidAttribute = true;
                    return *this;
                }
                m_capabilityResources = capabilityResources;
                return *this;
            }
            ModeControllerAttributeBuilder& ModeControllerAttributeBuilder::addMode(
                const string& mode,
                const ModeResources& modeResources) {
                ACSDK_DEBUG5(LX(__func__));
                if (mode.empty()) {
                    ACSDK_ERROR(LX("addModeFailed").d("reason", "emptyMode"));
                    m_invalidAttribute = true;
                    return *this;
                }
                if (!modeResources.isValid()) {
                    ACSDK_ERROR(LX("addModeFailed").d("reason", "invalidModeResources"));
                    m_invalidAttribute = true;
                    return *this;
                }
                if (m_modes.find(mode) != m_modes.end()) {
                    ACSDK_ERROR(LX("addModeFailed").d("reason", "modeAlreadyExists").sensitive("mode", mode));
                    m_invalidAttribute = true;
                    return *this;
                }
                ACSDK_DEBUG5(LX(__func__).sensitive("mode", mode).sensitive("modeResources", modeResources.toJson()));
                m_modes[mode] = modeResources;
                return *this;
            }
            ModeControllerAttributeBuilder& ModeControllerAttributeBuilder::setOrdered(bool ordered) {
                ACSDK_DEBUG5(LX(__func__));
                m_ordered = ordered;
                return *this;
            }
            avsCommon::utils::Optional<ModeControllerAttributes> ModeControllerAttributeBuilder::build() {
                ACSDK_DEBUG5(LX(__func__));
                auto controllerAttribute = Optional<ModeControllerAttributes>();
                if (m_invalidAttribute) {
                    ACSDK_ERROR(LX("buildFailed").d("reason", "invalidAttribute"));
                    return controllerAttribute;
                }
                if (m_modes.size() == 0) {
                    ACSDK_ERROR(LX("buildFailed").d("reason", "modesNotProvided"));
                    return controllerAttribute;
                }
                ACSDK_DEBUG5(LX(__func__).sensitive("capabilityResources", m_capabilityResources.toJson()));
                ACSDK_DEBUG5(LX(__func__).d("#modes", m_modes.size()));
                return Optional<ModeControllerAttributes>({m_capabilityResources, m_modes, m_ordered});
            }
        }
    }
}