#include <avs/AVSMessageEndpoint.h>
#include <avs/AVSMessageHeader.h>
#include <avs/EventBuilder.h>
#include <json/JSONGenerator.h>
#include <logger/Logger.h>
#include <util/Optional.h>
#include <endpoints/EndpointAttributeValidation.h>
#include "DiscoveryUtils.h"

static const std::string TAG("DiscoveryUtils");
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)
namespace alexaClientSDK {
    namespace capabilitiesDelegate {
        namespace utils {
            using namespace alexaClientSDK::endpoints;
            using namespace json;
            using namespace rapidjson;
            static const string CAPABILITY_INTERFACE_INSTANCE_NAME_KEY = "instance";
            static const string CAPABILITY_INTERFACE_PROPERTIES_KEY = "properties";
            static const string CAPABILITY_INTERFACE_PROPERTIES_NAME_KEY = "name";
            static const string CAPABILITY_INTERFACE_PROPERTIES_SUPPORTED_KEY = "supported";
            static const string CAPABILITY_INTERFACE_PROPERTIES_PROACTIVELY_REPORTED_KEY = "proactivelyReported";
            static const string CAPABILITY_INTERFACE_PROPERTIES_RETRIEVABLE_KEY = "retrievable";
            static const string CAPABILITY_INTERFACE_PROPERTIES_NON_CONTROLLABLE_KEY = "nonControllable";
            const static string ENDPOINT_ID_KEY = "endpointId";
            const static string FRIENDLY_NAME_KEY = "friendlyName";
            const static string DESCRIPTION_KEY = "description";
            const static string MANUFACTURER_NAME_KEY = "manufacturerName";
            const static string DISPLAY_CATEGORIES_KEY = "displayCategories";
            const static string ADDITIONAL_ATTRIBUTES_KEY = "additionalAttributes";
            const static string DEVICE_SERIAL_NUMBER_KEY = "deviceSerialNumber";
            const static string CONNECTIONS_KEY = "connections";
            const static string COOKIE_KEY = "cookie";
            static const string ADDITIONAL_MANUFACTURER_KEY = "manufacturer";
            static const string ADDITIONAL_MODEL_KEY = "model";
            static const string ADDITIONAL_SERIAL_NUMBER_KEY = "serialNumber";
            static const string ADDITIONAL_FIRMWARE_VERSION_KEY = "firmwareVersion";
            static const string ADDITIONAL_SOFTWARE_VERSION_KEY = "softwareVersion";
            static const string ADDITIONAL_CUSTOM_IDENTIFIER_KEY = "customIdentifier";
            static const string CAPABILITIES_KEY = "capabilities";
            static const string DISCOVERY_NAMESPACE = "Alexa.Discovery";
            static const string PAYLOAD_VERSION = "3";
            static const string ADD_OR_UPDATE_REPORT_NAME = "AddOrUpdateReport";
            static const string DELETE_REPORT_NAME = "DeleteReport";
            static const string ENDPOINTS_KEY = "endpoints";
            static const string SCOPE_KEY = "scope";
            static const string SCOPE_TYPE_KEY = "type";
            static const string SCOPE_TYPE_BEARER_TOKEN = "BearerToken";
            static const string SCOPE_TOKEN_KEY = "token";
            struct JsonObjectScope {
                JsonObjectScope(JsonGenerator* generator, string key) {
                    m_generator = generator;
                    m_generator->startObject(key);
                }
                ~JsonObjectScope() {
                    m_generator->finishObject();
                }
                JsonGenerator* m_generator;
            };
            static string getScopeJson(const string& authToken) {
                JsonGenerator scopeGenerator;
                scopeGenerator.addMember(SCOPE_TYPE_KEY, SCOPE_TYPE_BEARER_TOKEN);
                scopeGenerator.addMember(SCOPE_TOKEN_KEY, authToken);
                return scopeGenerator.toString();
            }
            static string getCapabilityConfigJson(const CapabilityConfiguration& capabilityConfig) {
                JsonGenerator generator;
                generator.addMember(CAPABILITY_INTERFACE_TYPE_KEY, capabilityConfig.type);
                generator.addMember(CAPABILITY_INTERFACE_NAME_KEY, capabilityConfig.interfaceName);
                generator.addMember(CAPABILITY_INTERFACE_VERSION_KEY, capabilityConfig.version);
                if (capabilityConfig.instanceName.hasValue()) {
                    generator.addMember(CAPABILITY_INTERFACE_INSTANCE_NAME_KEY, capabilityConfig.instanceName.value());
                }
                if (capabilityConfig.properties.hasValue()) {
                    JsonObjectScope scope(&generator, CAPABILITY_INTERFACE_PROPERTIES_KEY);
                    std::vector<std::string> supportedJson;
                    for (auto supported : capabilityConfig.properties.value().supportedList) {
                        JsonGenerator generator2;
                        generator2.addMember(CAPABILITY_INTERFACE_PROPERTIES_NAME_KEY, supported);
                        supportedJson.push_back(generator2.toString());
                    }
                    generator.addMembersArray(CAPABILITY_INTERFACE_PROPERTIES_SUPPORTED_KEY, supportedJson);
                    generator.addMember(
                        CAPABILITY_INTERFACE_PROPERTIES_PROACTIVELY_REPORTED_KEY,
                        capabilityConfig.properties.value().isProactivelyReported);
                    generator.addMember(
                        CAPABILITY_INTERFACE_PROPERTIES_RETRIEVABLE_KEY, capabilityConfig.properties.value().isRetrievable);
                    if (capabilityConfig.properties.value().isNonControllable.hasValue()) {
                        generator.addMember(
                            CAPABILITY_INTERFACE_PROPERTIES_NON_CONTROLLABLE_KEY,
                            capabilityConfig.properties.value().isNonControllable.value());
                    }
                }
                for (const auto& customConfigPair : capabilityConfig.additionalConfigurations) {
                    generator.addRawJsonMember(customConfigPair.first, customConfigPair.second);
                }
                return generator.toString();
            }
            bool validateCapabilityConfiguration(const CapabilityConfiguration& capabilityConfig) {
                if (capabilityConfig.type.empty() || capabilityConfig.interfaceName.empty() || capabilityConfig.version.empty()) {
                    ACSDK_ERROR(LX("validateCapabilityConfigurationFailed").d("reason", "requiredFieldEmpty")
                        .d("missingType", capabilityConfig.type.empty()).d("capabilityConfig", capabilityConfig.interfaceName.empty())
                        .d("version", capabilityConfig.version.empty()));
                    return false;
                }
                if (capabilityConfig.instanceName.hasValue() && capabilityConfig.instanceName.value().empty()) {
                    ACSDK_ERROR(LX("validateCapabilityConfigurationFailed").d("reason", "emptyInstanceName"));
                    return false;
                }
                if (capabilityConfig.properties.hasValue() && capabilityConfig.properties.value().supportedList.empty()) {
                    ACSDK_ERROR(LX("validateCapabilityConfigurationFailed").d("reason", "emptySupportedList"));
                    return false;
                }
                for (auto it = capabilityConfig.additionalConfigurations.begin(); it != capabilityConfig.additionalConfigurations.end(); ++it) {
                    Document configJson;
                    if (configJson.Parse(it->second.data()).HasParseError()) {
                        ACSDK_ERROR(LX("validateCapabilityConfigurationFailed").d("reason", "invalidCustomConfiguration"));
                        return false;
                    }
                }
                return true;
            }

            bool validateEndpointAttributes(const AVSDiscoveryEndpointAttributes& endpointAttributes) {
                if (!isEndpointIdValid(endpointAttributes.endpointId)) {
                    ACSDK_ERROR(LX("validateEndpointAttributesFailed").d("reason", "invalidEndpointId"));
                    return false;
                }
                if (!isDescriptionValid(endpointAttributes.description.data())) {
                    ACSDK_ERROR(LX("validateEndpointAttributesFailed").d("reason", "invalidDescription"));
                    return false;
                }
                if (!isManufacturerNameValid(endpointAttributes.manufacturerName.data())) {
                    ACSDK_ERROR(LX("validateEndpointAttributesFailed").d("reason", "invalidManufacturerName"));
                    return false;
                }
                if (endpointAttributes.displayCategories.empty()) {
                    ACSDK_ERROR(LX("validateEndpointAttributesFailed").d("reason", "invalidDisplayCategories"));
                    return false;
                }
                return true;
            }
            string getEndpointConfigJson(
                const AVSDiscoveryEndpointAttributes& endpointAttributes,
                const std::vector<avsCommon::avs::CapabilityConfiguration>& capabilities) {
                JsonGenerator generator;
                generator.addMember(ENDPOINT_ID_KEY, endpointAttributes.endpointId);
                generator.addMember(FRIENDLY_NAME_KEY, endpointAttributes.friendlyName.data());
                generator.addMember(DESCRIPTION_KEY, endpointAttributes.description.data());
                generator.addMember(MANUFACTURER_NAME_KEY, endpointAttributes.manufacturerName.data());
                generator.addStringArray(DISPLAY_CATEGORIES_KEY, endpointAttributes.displayCategories);
                if (endpointAttributes.additionalAttributes.hasValue()) {
                    JsonObjectScope scope(&generator, ADDITIONAL_ATTRIBUTES_KEY);
                    AVSDiscoveryEndpointAttributes::AdditionalAttributes additionalAttributes =
                        endpointAttributes.additionalAttributes.value();
                    if (!additionalAttributes.manufacturer.empty()) {
                        generator.addMember(ADDITIONAL_MANUFACTURER_KEY, additionalAttributes.manufacturer.data());
                    }
                    if (!additionalAttributes.model.empty()) {
                        generator.addMember(ADDITIONAL_MODEL_KEY, additionalAttributes.model.data());
                    }
                    if (!additionalAttributes.serialNumber.empty()) {
                        generator.addMember(ADDITIONAL_SERIAL_NUMBER_KEY, additionalAttributes.serialNumber.data());
                    }
                    if (!additionalAttributes.firmwareVersion.empty()) {
                        generator.addMember(ADDITIONAL_FIRMWARE_VERSION_KEY, additionalAttributes.firmwareVersion.data());
                    }
                    if (!additionalAttributes.softwareVersion.empty()) {
                        generator.addMember(ADDITIONAL_SOFTWARE_VERSION_KEY, additionalAttributes.softwareVersion.data());
                    }
                    if (!additionalAttributes.customIdentifier.empty()) {
                        generator.addMember(ADDITIONAL_CUSTOM_IDENTIFIER_KEY, additionalAttributes.customIdentifier.data());
                    }
                }
                if (endpointAttributes.registration.hasValue()) {
                    JsonObjectScope registration(&generator, endpointAttributes.registration.value().registrationKey.data());
                    generator.addMember(endpointAttributes.registration.value().productIdKey.data(), endpointAttributes.registration.value().productId.data());
                    generator.addMember(DEVICE_SERIAL_NUMBER_KEY, endpointAttributes.registration.value().serialNumber.data());
                }
                if (!endpointAttributes.connections.empty()) {
                    vector<std::string> connectionsJsons;
                    for (size_t i = 0; i < endpointAttributes.connections.size(); ++i) {
                        JsonGenerator connectionJsonGenerator;
                        for (auto it = endpointAttributes.connections[i].begin(); it != endpointAttributes.connections[i].end(); ++it) {
                            connectionJsonGenerator.addMember(it->first.data(), it->second.data());
                        }
                        connectionsJsons.push_back(connectionJsonGenerator.toString());
                    }
                    generator.addMembersArray(CONNECTIONS_KEY, connectionsJsons);
                }
                if (!endpointAttributes.cookies.empty()) {
                    JsonObjectScope cookies(&generator, COOKIE_KEY);
                    for (auto it = endpointAttributes.cookies.begin(); it != endpointAttributes.cookies.end(); ++it) {
                        generator.addMember(it->first, it->second);
                    }
                }
                vector<string> capabilityConfigJsons;
                for (auto capabilityConfig : capabilities) {
                    if (validateCapabilityConfiguration(capabilityConfig)) {
                        string capabilityConfigJson = getCapabilityConfigJson(capabilityConfig);
                        capabilityConfigJsons.push_back(capabilityConfigJson);
                    }
                }
                generator.addMembersArray(CAPABILITIES_KEY, capabilityConfigJsons);
                return generator.toString();
            }
            string getDeleteReportEndpointConfigJson(const string& endpointId) {
                JsonGenerator deleteReportEndpointConfigGenerator;
                { deleteReportEndpointConfigGenerator.addMember(ENDPOINT_ID_KEY, endpointId); }
                return deleteReportEndpointConfigGenerator.toString();
            }
            pair<string, string> getAddOrUpdateReportEventJson(
                const vector<string>& endpointConfigurations,
                const string& authToken) {
                ACSDK_DEBUG5(LX(__func__));
                auto header = AVSMessageHeader::createAVSEventHeader(DISCOVERY_NAMESPACE, ADD_OR_UPDATE_REPORT_NAME,"","", PAYLOAD_VERSION, "");
                JsonGenerator payloadGenerator;
                {
                    payloadGenerator.addRawJsonMember(SCOPE_KEY, getScopeJson(authToken));
                    payloadGenerator.addMembersArray(ENDPOINTS_KEY, endpointConfigurations);
                }
                string addOrUpdateEvent = buildJsonEventString(header, Optional<AVSMessageEndpoint>(), payloadGenerator.toString());
                return {addOrUpdateEvent, header.getEventCorrelationToken()};
            }
            string getDeleteReportEventJson(const vector<string>& endpointConfigurations, const string& authToken) {
                ACSDK_DEBUG5(LX(__func__));
                auto header = AVSMessageHeader::createAVSEventHeader(DISCOVERY_NAMESPACE, DELETE_REPORT_NAME, "", "", PAYLOAD_VERSION, "");
                JsonGenerator payloadGenerator;
                {
                    payloadGenerator.addRawJsonMember(SCOPE_KEY, getScopeJson(authToken));
                    payloadGenerator.addMembersArray(ENDPOINTS_KEY, endpointConfigurations);
                }
                return buildJsonEventString(header, Optional<AVSMessageEndpoint>(), payloadGenerator.toString());
            }
        }
    }
}