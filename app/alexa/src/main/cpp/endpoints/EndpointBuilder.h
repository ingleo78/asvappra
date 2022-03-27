#ifndef ALEXA_CLIENT_SDK_ENDPOINTS_INCLUDE_ENDPOINTS_ENDPOINTBUILDER_H_
#define ALEXA_CLIENT_SDK_ENDPOINTS_INCLUDE_ENDPOINTS_ENDPOINTBUILDER_H_

#include <list>
#include <memory>
#include <string>
#include <capability_agents/Alexa/AlexaInterfaceMessageSenderInternalInterface.h>
#include <avs/AVSDiscoveryEndpointAttributes.h>
#include <sdkinterfaces/AlexaInterfaceMessageSenderInterface.h>
#include <sdkinterfaces/Endpoints/EndpointBuilderInterface.h>
#include <sdkinterfaces/Endpoints/EndpointInterface.h>
#include <sdkinterfaces/Endpoints/EndpointRegistrationManagerInterface.h>
#include <util/DeviceInfo.h>
#include <util/Optional.h>
#include <util/RequiresShutdown.h>
#include <sdkinterfaces/ContextManagerInterface.h>
#include <sdkinterfaces/CapabilityConfigurationInterface.h>
#include <sdkinterfaces/MessageSenderInterface.h>
#include <sdkinterfaces/ExceptionEncounteredSenderInterface.h>

namespace alexaClientSDK {
    namespace endpoints {
        using namespace std;
        using namespace capabilityAgents;
        using namespace alexa;
        using namespace avsCommon;
        using namespace avs;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace logger;
        using namespace powerController;
        using namespace rangeController;
        using namespace toggleController;
        using namespace sdkInterfaces::endpoints;
        class EndpointBuilder : public EndpointBuilderInterface {
        public:
            using EndpointAttributes = AVSDiscoveryEndpointAttributes;
            static unique_ptr<EndpointBuilder> create(const DeviceInfo& deviceInfo, shared_ptr<ContextManagerInterface> contextManager,
                                                      shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
                                                      shared_ptr<AlexaInterfaceMessageSenderInternalInterface> alexaMessageSender);
            virtual ~EndpointBuilder();
            EndpointBuilder& withDerivedEndpointId(const string& suffix) override;
            EndpointBuilder& withEndpointId(const EndpointIdentifier& endpointId) override;
            EndpointBuilder& withFriendlyName(const string& friendlyName) override;
            EndpointBuilder& withDescription(const string& description) override;
            EndpointBuilder& withManufacturerName(const string& manufacturerName) override;
            EndpointBuilder& withDisplayCategory(const vector<string>& displayCategories) override;
            EndpointBuilder& withAdditionalAttributes(const string& manufacturer, const string& model, const string& serialNumber,
                                                      const string& firmwareVersion, const string& softwareVersion,
                                                      const string& customIdentifier) override;
            EndpointBuilder& withConnections(const vector<map<string, string>>& connections) override;
            EndpointBuilder& withCookies(const map<string, string>& cookies) override;
            EndpointBuilder& withPowerController(shared_ptr<PowerControllerInterface> powerController, bool isProactivelyReported,
                                                 bool isRetrievable) override;
            EndpointBuilder& withToggleController(shared_ptr<ToggleControllerInterface> toggleController, const string& instance,
                                                  const ToggleControllerAttributes& toggleControllerAttributes, bool isProactivelyReported,
                                                  bool isRetrievable, bool isNonControllable = false) override;
            EndpointBuilder& withModeController(shared_ptr<ModeControllerInterface> modeController, const string& instance,
                                                const ModeControllerAttributes& modeControllerAttributes, bool isProactivelyReported,
                                                bool isRetrievable, bool isNonControllable = false) override;
            EndpointBuilder& withRangeController(shared_ptr<RangeControllerInterface> rangeController, const string& instance,
                                                 const RangeControllerAttributes& rangeControllerAttributes, bool isProactivelyReported,
                                                 bool isRetrievable, bool isNonControllable = false) override;
            unique_ptr<EndpointInterface> build() override;
            EndpointBuilder& withCapability(const CapabilityConfiguration& configuration, shared_ptr<DirectiveHandlerInterface> directiveHandler);
            EndpointBuilder& withCapability(const shared_ptr<CapabilityConfigurationInterface>& configurationInterface,
                                            shared_ptr<DirectiveHandlerInterface> directiveHandler);
            EndpointBuilder& withCapabilityConfiguration(const shared_ptr<CapabilityConfigurationInterface>& configurationInterface);
            bool finishDefaultEndpointConfiguration();
            unique_ptr<EndpointInterface> buildDefaultEndpoint();
        private:
            using CapabilityBuilder = function<pair<CapabilityConfiguration, shared_ptr<DirectiveHandlerInterface>>()>;
            EndpointBuilder(const DeviceInfo& deviceInfo, shared_ptr<ContextManagerInterface> contextManager,
                            shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
                            shared_ptr<AlexaInterfaceMessageSenderInternalInterface> alexaMessageSender);
            unique_ptr<EndpointInterface> buildImplementation();
            bool m_isDefaultEndpoint;
            bool m_hasBeenBuilt;
            bool m_invalidConfiguration;
            const DeviceInfo m_deviceInfo;
            EndpointAttributes m_attributes;
            shared_ptr<ContextManagerInterface> m_contextManager;
            shared_ptr<ExceptionEncounteredSenderInterface> m_exceptionSender;
            shared_ptr<AlexaInterfaceMessageSenderInternalInterface> m_alexaMessageSender;
            list<CapabilityBuilder> m_capabilitiesBuilders;
            list<CapabilityConfiguration> m_capabilityConfigurations;
            list<shared_ptr<DirectiveHandlerInterface>> m_directiveHandlers;
            list<shared_ptr<RequiresShutdown>> m_requireShutdownObjects;
        };
    }
}
#endif