#include <algorithm>
#include "Endpoint.h"
#include "EndpointBuilderInterface.h"
#include "EndpointRegistrationManagerInterface.h"

namespace alexaClientSDK {
    namespace endpoints {
        static const string TAG("Endpoint");
        #define LX(event) LogEntry(TAG, event)
        Endpoint::Endpoint(const EndpointAttributes& attributes) : m_attributes(attributes) {}
        Endpoint::~Endpoint() {
            for (auto& shutdownObj : m_requireShutdownObjects) {
                shutdownObj->shutdown();
            }
        }
        void Endpoint::addRequireShutdownObjects(const list<shared_ptr<RequiresShutdown>>& requireShutdownObjects) {
            m_requireShutdownObjects.insert(requireShutdownObjects.begin(), requireShutdownObjects.end());
        }
        EndpointIdentifier Endpoint::getEndpointId() const {
            return m_attributes.endpointId;
        }
        AVSDiscoveryEndpointAttributes Endpoint::getAttributes() const {
            return m_attributes;
        }
        vector<CapabilityConfiguration> Endpoint::getCapabilityConfigurations() const {
            lock_guard<std::mutex> lock{m_mutex};
            vector<CapabilityConfiguration> retValue;
            for_each(m_capabilities.begin(), m_capabilities.end(), [&retValue](decltype(m_capabilities)::value_type value) {
                         retValue.push_back(value.first);
                     });
            return retValue;
        }
        unordered_map<CapabilityConfiguration, shared_ptr<DirectiveHandlerInterface>> Endpoint::getCapabilities() const {
            lock_guard<mutex> lock{m_mutex};
            return m_capabilities;
        }
        bool Endpoint::addCapability(const CapabilityConfiguration& capabilityConfiguration, shared_ptr<DirectiveHandlerInterface> directiveHandler) {
            if (!directiveHandler) {
                ACSDK_ERROR(LX("addCapabilityAgentFailed").d("reason", "nullHandler"));
                return false;
            }
            lock_guard<mutex> lock{m_mutex};
            if (m_capabilities.find(capabilityConfiguration) != m_capabilities.end()) {
                ACSDK_ERROR(LX("addCapabilityAgentFailed").d("reason", "capabilityAlreadyExists")
                    .d("interface", capabilityConfiguration.interfaceName).d("type", capabilityConfiguration.type)
                    .d("instance", capabilityConfiguration.instanceName.valueOr("")));
                return false;
            }
            m_capabilities.insert(make_pair(capabilityConfiguration, directiveHandler));
            return true;
        }
        bool Endpoint::addCapabilityConfiguration(const CapabilityConfiguration& capabilityConfiguration) {
            lock_guard<mutex> lock{m_mutex};
            if (!m_capabilities.insert(make_pair(capabilityConfiguration, nullptr)).second) {
                ACSDK_ERROR(LX("addCapabilityConfigurationFailed").d("reason", "capabilityAlreadyExists")
                    .d("interface", capabilityConfiguration.interfaceName).d("type", capabilityConfiguration.type)
                    .d("instance", capabilityConfiguration.instanceName.valueOr("")));
                return false;
            }
            return true;
        }
    }
}