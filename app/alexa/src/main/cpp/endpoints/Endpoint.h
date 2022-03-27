#ifndef ALEXA_CLIENT_SDK_ENDPOINTS_INCLUDE_ENDPOINTS_ENDPOINT_H_
#define ALEXA_CLIENT_SDK_ENDPOINTS_INCLUDE_ENDPOINTS_ENDPOINT_H_

#include <list>
#include <mutex>
#include <set>
#include <unordered_map>
#include <avs/AVSDiscoveryEndpointAttributes.h>
#include <avs/CapabilityConfiguration.h>
#include <sdkinterfaces/DirectiveHandlerInterface.h>
#include <sdkinterfaces/Endpoints/EndpointIdentifier.h>
#include <sdkinterfaces/Endpoints/EndpointInterface.h>
#include <util/RequiresShutdown.h>

namespace alexaClientSDK {
    namespace endpoints {
        using namespace std;
        using namespace avsCommon;
        using namespace avs;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace logger;
        using namespace sdkInterfaces::endpoints;
        class Endpoint : public EndpointInterface {
        public:
            using EndpointAttributes = AVSDiscoveryEndpointAttributes;
            Endpoint(const EndpointAttributes& attributes);
            ~Endpoint();
            EndpointIdentifier getEndpointId() const override;
            EndpointAttributes getAttributes() const override;
            vector<CapabilityConfiguration> getCapabilityConfigurations() const override;
            unordered_map<CapabilityConfiguration, shared_ptr<DirectiveHandlerInterface>> getCapabilities() const override;
            bool addCapability(const CapabilityConfiguration& capabilityConfiguration, shared_ptr<DirectiveHandlerInterface> directiveHandler);
            bool addCapabilityConfiguration(const CapabilityConfiguration& capabilityConfiguration);
            void addRequireShutdownObjects(const list<shared_ptr<RequiresShutdown>>& requireShutdownObjects);
        private:
            mutable mutex m_mutex;
            const EndpointAttributes m_attributes;
            unordered_map<CapabilityConfiguration, shared_ptr<DirectiveHandlerInterface>> m_capabilities;
            set<shared_ptr<RequiresShutdown>> m_requireShutdownObjects;
        };
    }
}
#endif