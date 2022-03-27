#include <capability_agents/Alexa/AlexaInterfaceCapabilityAgent.h>
#include <logger/Logger.h>
#include <memory/Memory.h>
#ifdef POWER_CONTROLLER
#include <PowerController/PowerControllerCapabilityAgent.h>
#endif
#ifdef TOGGLE_CONTROLLER
#include <ToggleController/ToggleControllerCapabilityAgent.h>
#endif
#ifdef MODE_CONTROLLER
#include <ModeController/ModeControllerCapabilityAgent.h>
#endif
#ifdef RANGE_CONTROLLER
#include <RangeController/RangeControllerCapabilityAgent.h>
#endif
#include "Endpoint.h"
#include "EndpointAttributeValidation.h"
#include "EndpointBuilder.h"

namespace alexaClientSDK {
    namespace endpoints {
        using namespace memory;
        static const string TAG("EndpointBuilder");
        #define LX(event) LogEntry(TAG, event)
        const string ENDPOINT_ID_CONCAT = "::";
        static constexpr size_t MAX_SUFFIX_LENGTH = 10;
        const string ALEXA_DISPLAY_CATEGORY = "ALEXA_VOICE_ENABLED";
        unique_ptr<EndpointBuilder> EndpointBuilder::create(const DeviceInfo& deviceInfo, shared_ptr<ContextManagerInterface> contextManager,
                                                            shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
                                                            shared_ptr<AlexaInterfaceMessageSenderInternalInterface> alexaMessageSender) {
            if (!contextManager || !alexaMessageSender || !exceptionSender) {
                ACSDK_ERROR(LX("createFailed").d("reason", "nullParameter").d("isContextManagerNull", !contextManager)
                    .d("isExceptionSenderNull", !exceptionSender).d("isAlexaMessageSenderNull", !alexaMessageSender));
                return nullptr;
            }
            return unique_ptr<EndpointBuilder>(new EndpointBuilder(deviceInfo, contextManager, exceptionSender, alexaMessageSender));
        }
        EndpointBuilder::EndpointBuilder(const DeviceInfo& deviceInfo, shared_ptr<ContextManagerInterface> contextManager,
                                         shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
                                         shared_ptr<AlexaInterfaceMessageSenderInternalInterface> alexaMessageSender) :
                                         m_isDefaultEndpoint{false}, m_hasBeenBuilt{false}, m_invalidConfiguration{false},
                                         m_deviceInfo{deviceInfo}, m_contextManager{contextManager}, m_exceptionSender{exceptionSender},
                                         m_alexaMessageSender{alexaMessageSender} {}
        EndpointBuilder::~EndpointBuilder() {
            for (auto& shutdownObj : m_requireShutdownObjects) {
                shutdownObj->shutdown();
            }
        }
        EndpointBuilder& EndpointBuilder::withDerivedEndpointId(const std::string& suffix) {
            if (m_isDefaultEndpoint) {
                ACSDK_ERROR(LX(string(__func__) + "Failed").d("reason", "operationNotAllowed"));
                return *this;
            }
            if (suffix.length() > MAX_SUFFIX_LENGTH) {
                ACSDK_ERROR(LX(string(__func__) + "Failed").d("reason", "suffixMaxLengthExceeded").d("suffix", suffix));
                return *this;
            }
            m_attributes.endpointId = m_deviceInfo.getDefaultEndpointId() + ENDPOINT_ID_CONCAT + suffix;
            return *this;
        }
        EndpointBuilder& EndpointBuilder::withEndpointId(const EndpointIdentifier& endpointId) {
            if (m_isDefaultEndpoint) {
                ACSDK_ERROR(LX(string(__func__) + "Failed").d("reason", "operationNotAllowed"));
                return *this;
            }
            if (!isEndpointIdValid(endpointId)) {
                ACSDK_ERROR(LX(__func__).d("reason", "invalidEndpointId"));
                m_invalidConfiguration = true;
                return *this;
            }
            m_attributes.endpointId = endpointId;
            return *this;
        }
        bool EndpointBuilder::finishDefaultEndpointConfiguration() {
            m_attributes.registration.set(EndpointAttributes::Registration(m_deviceInfo.getProductId(),m_deviceInfo.getDeviceSerialNumber(),
                                          m_deviceInfo.getRegistrationKey(),m_deviceInfo.getProductIdKey()));
            m_attributes.endpointId = m_deviceInfo.getDefaultEndpointId();
            m_attributes.displayCategories = {ALEXA_DISPLAY_CATEGORY};
            m_attributes.manufacturerName = m_deviceInfo.getManufacturerName();
            m_attributes.description = m_deviceInfo.getDeviceDescription();
            if (!m_deviceInfo.getFriendlyName().empty()) m_attributes.friendlyName = m_deviceInfo.getFriendlyName();
            m_isDefaultEndpoint = true;
            auto attributes = m_attributes;
            return !m_invalidConfiguration;
        }
        EndpointBuilder& EndpointBuilder::withFriendlyName(const string& friendlyName) {
            if (!isFriendlyNameValid(friendlyName)) {
                ACSDK_ERROR(LX(__func__).d("reason", "invalidFriendlyName"));
                return *this;
            }
            m_attributes.friendlyName = friendlyName;
            return *this;
        }
        EndpointBuilder& EndpointBuilder::withDescription(const string& description) {
            if (!isDescriptionValid(description)) {
                ACSDK_ERROR(LX(__func__).d("reason", "invalidDescription"));
                m_invalidConfiguration = true;
                return *this;
            }
            m_attributes.description = description;
            return *this;
        }
        EndpointBuilder& EndpointBuilder::withManufacturerName(const string& manufacturerName) {
            if (!isManufacturerNameValid(manufacturerName)) {
                ACSDK_ERROR(LX(__func__).d("reason", "invalidManufacturerName"));
                m_invalidConfiguration = true;
                return *this;
            }
            m_attributes.manufacturerName = manufacturerName;
            return *this;
        }
        EndpointBuilder& EndpointBuilder::withDisplayCategory(const vector<string>& displayCategories) {
            if (displayCategories.empty()) {
                ACSDK_DEBUG5(LX(__func__).d("reason", "invalidDisplayCategories"));
                m_invalidConfiguration = true;
                return *this;
            }
            m_attributes.displayCategories = displayCategories;
            return *this;
        }
        EndpointBuilder& EndpointBuilder::withAdditionalAttributes(const string& manufacturer, const string& model, const string& serialNumber,
                                                                   const string& firmwareVersion, const string& softwareVersion,
                                                                   const string& customIdentifier) {
            EndpointAttributes::AdditionalAttributes additionalAttributes;
            additionalAttributes.manufacturer = manufacturer;
            additionalAttributes.model = model;
            additionalAttributes.serialNumber = serialNumber;
            additionalAttributes.firmwareVersion = firmwareVersion;
            additionalAttributes.softwareVersion = softwareVersion;
            additionalAttributes.customIdentifier = customIdentifier;
            if (!isAdditionalAttributesValid(additionalAttributes)) {
                ACSDK_ERROR(LX(__func__).d("reason", "invalidAdditionalAttributes"));
                m_invalidConfiguration = true;
                return *this;
            }
            m_attributes.additionalAttributes.set(additionalAttributes);
            return *this;
        }
        EndpointBuilder& EndpointBuilder::withConnections(const vector<map<string, string>>& connections) {
            if (!areConnectionsValid(connections)) {
                ACSDK_ERROR(LX(__func__).d("reason", "invalidConnections"));
                m_invalidConfiguration = true;
                return *this;
            }
            m_attributes.connections = connections;
            return *this;
        }
        EndpointBuilder& EndpointBuilder::withCookies(const map<string, string>& cookies) {
            if (!areCookiesValid(cookies)) {
                ACSDK_ERROR(LX(__func__).d("reason", "invalidCookies"));
                m_invalidConfiguration = true;
                return *this;
            }
            m_attributes.cookies = cookies;
            return *this;
        }
        EndpointBuilder& EndpointBuilder::withPowerController(shared_ptr<PowerControllerInterface> powerController, bool isProactivelyReported,
                                                              bool isRetrievable) {
        #ifdef POWER_CONTROLLER
            m_capabilitiesBuilders.push_back([this, powerController, isProactivelyReported, isRetrievable] {
                auto powerControllerCA = PowerControllerCapabilityAgent::create(this->m_attributes.endpointId, powerController,
                                                                                m_contextManager, m_alexaMessageSender, m_exceptionSender,
                                                                                isProactivelyReported, isRetrievable);
                m_requireShutdownObjects.push_back(powerControllerCA);
                return CapabilityBuilder::result_type(powerControllerCA->getCapabilityConfiguration(), powerControllerCA);
            });
        #else
            ACSDK_ERROR(LX("withPowerController").d("reason", "capabilityNotEnabled"));
            m_invalidConfiguration = true;
        #endif
            return *this;
        }
        EndpointBuilder& EndpointBuilder::withToggleController(shared_ptr<ToggleControllerInterface> toggleController, const string& instance,
                                                               const ToggleControllerAttributes& toggleControllerAttributes,
                                                               bool isProactivelyReported, bool isRetrievable, bool isNonControllable) {
        #ifdef TOGGLE_CONTROLLER
            m_capabilitiesBuilders.push_back([this, toggleController, instance, toggleControllerAttributes, isProactivelyReported,
                                             isRetrievable, isNonControllable] {
                auto toggleControllerCA = ToggleControllerCapabilityAgent::create(this->m_attributes.endpointId, instance, toggleControllerAttributes,
                                                                                  toggleController, m_contextManager, m_alexaMessageSender,
                                                                                  m_exceptionSender, isProactivelyReported, isRetrievable,
                                                                                  isNonControllable);
                m_requireShutdownObjects.push_back(toggleControllerCA);
                return CapabilityBuilder::result_type(toggleControllerCA->getCapabilityConfiguration(), toggleControllerCA);
            });
        #else
            ACSDK_ERROR(LX("withToggleController").d("reason", "capabilityNotEnabled"));
            m_invalidConfiguration = true;
        #endif
            return *this;
        }
        EndpointBuilder& EndpointBuilder::withModeController(shared_ptr<ModeControllerInterface> modeController, const string& instance,
                                                             const ModeControllerAttributes& modeControllerAttributes, bool isProactivelyReported,
                                                             bool isRetrievable, bool isNonControllable) {
        #ifdef MODE_CONTROLLER
            m_capabilitiesBuilders.push_back([this, modeController, instance, modeControllerAttributes, isProactivelyReported, isRetrievable,
                                             isNonControllable] {
                auto modeControllerCA = ModeControllerCapabilityAgent::create(this->m_attributes.endpointId, instance, modeControllerAttributes,
                                                                              modeController, m_contextManager, m_alexaMessageSender,
                                                                              m_exceptionSender, isProactivelyReported, isRetrievable,
                                                                              isNonControllable);
                m_requireShutdownObjects.push_back(modeControllerCA);
                return CapabilityBuilder::result_type(modeControllerCA->getCapabilityConfiguration(), modeControllerCA);
            });
        #else
            ACSDK_ERROR(LX("withModeController").d("reason", "capabilityNotEnabled"));
            m_invalidConfiguration = true;
        #endif
            return *this;
        }
        EndpointBuilder& EndpointBuilder::withRangeController(shared_ptr<RangeControllerInterface> rangeController, const string& instance,
                                                              const RangeControllerAttributes& rangeControllerAttributes, bool isProactivelyReported,
                                                              bool isRetrievable, bool isNonControllable) {
        #ifdef RANGE_CONTROLLER
            m_capabilitiesBuilders.push_back([this, rangeController, instance, rangeControllerAttributes, isProactivelyReported, isRetrievable,
                                             isNonControllable] {
                auto rangeControllerCA = RangeControllerCapabilityAgent::create(this->m_attributes.endpointId, instance, rangeControllerAttributes,
                                                                                rangeController, m_contextManager, m_alexaMessageSender,
                                                                                m_exceptionSender, isProactivelyReported, isRetrievable,
                                                                                isNonControllable);
                m_requireShutdownObjects.push_back(rangeControllerCA);
                return CapabilityBuilder::result_type(rangeControllerCA->getCapabilityConfiguration(), rangeControllerCA);
            });
        #else
            ACSDK_ERROR(LX("withRangeController").d("reason", "capabilityNotEnabled"));
            m_invalidConfiguration = true;
        #endif
            return *this;
        }
        EndpointBuilder& EndpointBuilder::withCapability(const CapabilityConfiguration& configuration, shared_ptr<DirectiveHandlerInterface> directiveHandler) {
            m_capabilitiesBuilders.push_back([configuration, directiveHandler] { return make_pair(configuration, directiveHandler); });
            return *this;
        }
        EndpointBuilder& EndpointBuilder::withCapability(const shared_ptr<CapabilityConfigurationInterface>& configurationInterface,
                                                         shared_ptr<DirectiveHandlerInterface> directiveHandler) {
            if (!configurationInterface || !directiveHandler) {
                ACSDK_ERROR(LX("withCapabilityFailed").d("reason", "nullParameter").d("nullConfigurations", !configurationInterface)
                    .d("nullHandler", !directiveHandler));
                m_invalidConfiguration = true;
                return *this;
            }
            auto configurations = configurationInterface->getCapabilityConfigurations();
            if (configurations.empty() || configurations.find(nullptr) != configurations.end()) {
                ACSDK_ERROR(LX("withCapabilityFailed").d("reason", "invalidConfiguration").d("size", configurations.size()));
                m_invalidConfiguration = true;
                return *this;
            }
            for (auto& configurationPtr : configurations) {
                auto& configuration = *configurationPtr;
                m_capabilitiesBuilders.push_back([configuration, directiveHandler] { return make_pair(configuration, directiveHandler); });
            }
            return *this;
        }
        EndpointBuilder& EndpointBuilder::withCapabilityConfiguration(const shared_ptr<CapabilityConfigurationInterface>& configurationInterface) {
            if (!configurationInterface) {
                ACSDK_ERROR(LX("withCapabilityFailed").d("reason", "nullConfiguration"));
                m_invalidConfiguration = true;
                return *this;
            }
            auto configurations = configurationInterface->getCapabilityConfigurations();
            if (configurations.find(nullptr) != configurations.end()) {
                ACSDK_ERROR(LX("withCapabilityFailed").d("reason", "nullConfiguration"));
                m_invalidConfiguration = true;
                return *this;
            }
            for (auto& configurationPtr : configurations) {
                auto& configuration = *configurationPtr;
                m_capabilitiesBuilders.push_back([configuration]() { return std::make_pair(configuration, nullptr); });
            }
            return *this;
        }
        unique_ptr<EndpointInterface> EndpointBuilder::build() {
            if (m_isDefaultEndpoint) {
                ACSDK_ERROR(LX("buildFailed").d("reason", "buildDefaultEndpointNotAllowed"));
            }
            return buildImplementation();
        }
        unique_ptr<EndpointInterface> EndpointBuilder::buildDefaultEndpoint() {
            if (!m_isDefaultEndpoint) {
                ACSDK_ERROR(LX("buildDefaultEndpointFailed").d("reason", "notDefaultEndpoint"));
                return nullptr;
            }
            return buildImplementation();
        }
        unique_ptr<EndpointInterface> EndpointBuilder::buildImplementation() {
            if (m_hasBeenBuilt) {
                ACSDK_ERROR(LX("buildImplementationFailed").d("reason", "endpointAlreadyBuilt"));
                return nullptr;
            }
            if (m_invalidConfiguration) {
                ACSDK_ERROR(LX("buildImplementationFailed").d("reason", "invalidConfiguration"));
                return nullptr;
            }
            if (!m_isDefaultEndpoint && !isFriendlyNameValid(m_attributes.friendlyName.data())) {
                ACSDK_ERROR(
                    LX("buildFailed").d("reason", "friendlyNameInvalid").sensitive("friendlyName", m_attributes.friendlyName));
                return nullptr;
            }
            auto endpoint = make_unique<Endpoint>(m_attributes);
            if (!m_isDefaultEndpoint) {
                auto alexaCapabilityAgent = AlexaInterfaceCapabilityAgent::create(m_deviceInfo, m_attributes.endpointId, m_exceptionSender, m_alexaMessageSender);
                if (!alexaCapabilityAgent) {
                    ACSDK_ERROR(LX("buildImplementationFailed").d("reason", "unableToCreateAlexaCapabilityAgent"));
                    return nullptr;
                }
                endpoint->addCapability(alexaCapabilityAgent->getCapabilityConfiguration(), alexaCapabilityAgent);
            }
            for (auto& capabilityBuilder : m_capabilitiesBuilders) {
                auto capability = capabilityBuilder();
                if (!capability.second) {
                    if (!m_isDefaultEndpoint) {
                        ACSDK_ERROR(LX("buildImplementationFailed").d("reason", "buildCapabilityFailed"));
                        return nullptr;
                    }
                    endpoint->addCapabilityConfiguration(capability.first);
                } else endpoint->addCapability(capability.first, capability.second);
            }
            endpoint->addRequireShutdownObjects(m_requireShutdownObjects);
            m_requireShutdownObjects.clear();
            ACSDK_DEBUG2(LX(__func__).d("isDefault", m_isDefaultEndpoint).d("#capabilities", m_capabilitiesBuilders.size())
                .sensitive("endpointId", m_attributes.endpointId).sensitive("friendlyName", m_attributes.friendlyName));
            return move(endpoint);
        }
    }
}