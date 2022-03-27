#include <string>
#include <vector>
#include <gtest/gtest.h>
#include <avs/AVSDiscoveryEndpointAttributes.h>
#include <avs/CapabilityConfiguration.h>
#include <util/Optional.h>
#include <json/JSONUtils.h>
#include "DiscoveryUtils.h"

namespace alexaClientSDK {
    namespace capabilitiesDelegate {
        namespace utils {
            namespace test {
                using namespace testing;
                using namespace json;
                using namespace jsonUtils;
                using namespace sdkInterfaces::endpoints;
                using namespace rapidjson;
                using Properties = CapabilityConfiguration::Properties;
                static const string TEST_TYPE = "TEST_TYPE";
                static const string TEST_INTERFACE_NAME = "TEST_INTERFACE_NAME";
                static const string TEST_VERSION = "TEST_VERSION";
                static const Optional<string> TEST_INSTANCE_NAME("TEST_INSTANCE_NAME");
                static const vector<string> TEST_SUPPORTED_LIST{"TEST_PROPERTY"};
                static const Optional<Properties> TEST_PROPERTIES = Properties(false, false, TEST_SUPPORTED_LIST);
                static const string EXPECTED_TEST_PROPERTIES_STRING = R"({"supported":[{"name":"TEST_PROPERTY"}],"proactivelyReported":false,"retrievable":false})";
                static const string TEST_VALID_CONFIG_JSON = R"({"key":{"key1":"value1"}})";
                static const CapabilityConfiguration::AdditionalConfigurations TEST_ADDITIONAL_CONFIGURATIONS = {{"configuration", TEST_VALID_CONFIG_JSON}};
                static const string TEST_ENDPOINT_ID = "TEST_ENDPOINT_ID";
                static const string TEST_FRIENDLY_NAME = "TEST_FRIENDLY_NAME";
                static const string TEST_DESCRIPTION = "TEST_DESCRIPTION";
                static const string TEST_MANUFACTURER_NAME = "TEST_MANUFACTURER_NAME";
                static const vector<string> TEST_DISPLAY_CATEGORIES = {"TEST_DISPLAY_CATEGORY"};
                static const string TEST_CUSTOMER_IDENTIFIER = "TEST_CUSTOMER_IDENTIFIER";
                static const string TEST_SOFTWARE_VERSION = "TEST_SOFTWARE_VERSION";
                static const string TEST_FIRMWARE_VERSION = "TEST_FIRMWARE_VERSION";
                static const string TEST_SERIAL_NUMBER = "TEST_SERIAL_NUMBER";
                static const string TEST_MODEL = "TEST_MODEL";
                static const string TEST_MANUFACTURER = "TEST_MANUFACTURER";
                static const string TEST_PRODUCT_ID = "TEST_PRODUCT_ID";
                static const string TEST_REGISTRATION_KEY = "TEST_REGISTRATION_KEY";
                static const string TEST_PRODUCT_ID_KEY = "TEST_PRODUCT_ID_KEY";
                static const string TEST_ENDPOINT_CONFIG = R"({"endpointId":")" + TEST_ENDPOINT_ID + R"("})";
                static const string TEST_AUTH_TOKEN = "TEST_AUTH_TOKEN";
                static const vector<map<string, string>> TEST_CONNECTIONS_DATA = {{{"CON_1", "DATA_1"}}, {{"CON_2", "DATA_2"}}};
                static const map<string, string> TEST_COOKIE_DATA = {{"KEY1", "VALUE1"}, {"KEY2", "VALUE2"}};
                static const string ADD_OR_UPDATE_REPORT_EVENT_NAME = "AddOrUpdateReport";
                static const string DELETE_REPORT_EVENT_NAME = "DeleteReport";
                AVSDiscoveryEndpointAttributes getTestEndpointAttributes() {
                    AVSDiscoveryEndpointAttributes endpointAttributes;
                    std::vector<CapabilityConfiguration> capabilities;
                    endpointAttributes.endpointId = TEST_ENDPOINT_ID;
                    endpointAttributes.friendlyName = TEST_FRIENDLY_NAME;
                    endpointAttributes.description = TEST_DESCRIPTION;
                    endpointAttributes.manufacturerName = TEST_MANUFACTURER_NAME;
                    endpointAttributes.displayCategories = TEST_DISPLAY_CATEGORIES;
                    AVSDiscoveryEndpointAttributes::Registration testRegistration(TEST_PRODUCT_ID, TEST_SERIAL_NUMBER, TEST_REGISTRATION_KEY, TEST_PRODUCT_ID_KEY);
                    endpointAttributes.registration = testRegistration;
                    AVSDiscoveryEndpointAttributes::AdditionalAttributes additionalAttributes;
                    additionalAttributes.customIdentifier = TEST_CUSTOMER_IDENTIFIER;
                    additionalAttributes.softwareVersion = TEST_SOFTWARE_VERSION;
                    additionalAttributes.firmwareVersion = TEST_FIRMWARE_VERSION;
                    additionalAttributes.serialNumber = TEST_SERIAL_NUMBER;
                    additionalAttributes.model = TEST_MODEL;
                    additionalAttributes.manufacturer = TEST_MANUFACTURER;
                    endpointAttributes.additionalAttributes = additionalAttributes;
                    vector<map<string, string>> connections = TEST_CONNECTIONS_DATA;
                    endpointAttributes.connections = connections;
                    map<string, string> cookie = TEST_COOKIE_DATA;
                    endpointAttributes.cookies = cookie;
                    CapabilityConfiguration capability(TEST_TYPE, TEST_INTERFACE_NAME, TEST_VERSION, TEST_INSTANCE_NAME, TEST_PROPERTIES, TEST_ADDITIONAL_CONFIGURATIONS);
                    capabilities.push_back(capability);
                    return endpointAttributes;
                }
                vector<CapabilityConfiguration> getTestCapabilities() {
                    vector<CapabilityConfiguration> capabilities;
                    CapabilityConfiguration capability(TEST_TYPE, TEST_INTERFACE_NAME, TEST_VERSION, TEST_INSTANCE_NAME, TEST_PROPERTIES, TEST_ADDITIONAL_CONFIGURATIONS);
                    capabilities.push_back(capability);
                    return capabilities;
                }
                AVSDiscoveryEndpointAttributes createEndpointAttributes(const string& endpointId, const string& friendlyName, const string& description,
                                                                        const string& manufacturerName, const vector<string>& displayCategories) {
                    AVSDiscoveryEndpointAttributes endpointAttributes;
                    endpointAttributes.endpointId = endpointId;
                    endpointAttributes.friendlyName = friendlyName;
                    endpointAttributes.description = description;
                    endpointAttributes.manufacturerName = manufacturerName;
                    endpointAttributes.displayCategories = displayCategories;
                    return endpointAttributes;
                }
                void validateEndpointConfigJson(const string& endpointConfigJson) {
                    string value;
                    ASSERT_TRUE(retrieveValue(endpointConfigJson, "endpointId", &value));
                    ASSERT_EQ(value, TEST_ENDPOINT_ID);
                    ASSERT_TRUE(retrieveValue(endpointConfigJson, "friendlyName", &value));
                    ASSERT_EQ(value, TEST_FRIENDLY_NAME);
                    ASSERT_TRUE(retrieveValue(endpointConfigJson, "manufacturerName", &value));
                    ASSERT_EQ(value, TEST_MANUFACTURER_NAME);
                    ASSERT_TRUE(retrieveValue(endpointConfigJson, "description", &value));
                    ASSERT_EQ(value, TEST_DESCRIPTION);
                    auto displayCategoriesFromJson = retrieveStringArray<std::vector<std::string>>(endpointConfigJson, "displayCategories");
                    ASSERT_EQ(displayCategoriesFromJson, TEST_DISPLAY_CATEGORIES);
                    string additionalAttributesJson;
                    ASSERT_TRUE(retrieveValue(endpointConfigJson, "additionalAttributes", &additionalAttributesJson));
                    ASSERT_TRUE(retrieveValue(additionalAttributesJson, "manufacturer", &value));
                    ASSERT_EQ(value, TEST_MANUFACTURER);
                    ASSERT_TRUE(retrieveValue(additionalAttributesJson, "model", &value));
                    ASSERT_EQ(value, TEST_MODEL);
                    ASSERT_TRUE(retrieveValue(additionalAttributesJson, "serialNumber", &value));
                    ASSERT_EQ(value, TEST_SERIAL_NUMBER);
                    ASSERT_TRUE(retrieveValue(additionalAttributesJson, "firmwareVersion", &value));
                    ASSERT_EQ(value, TEST_FIRMWARE_VERSION);
                    ASSERT_TRUE(retrieveValue(additionalAttributesJson, "softwareVersion", &value));
                    ASSERT_EQ(value, TEST_SOFTWARE_VERSION);
                    ASSERT_TRUE(retrieveValue(additionalAttributesJson, "customIdentifier", &value));
                    ASSERT_EQ(value, TEST_CUSTOMER_IDENTIFIER);
                    string registrationJson;
                    ASSERT_TRUE(retrieveValue(endpointConfigJson, TEST_REGISTRATION_KEY, &registrationJson));
                    string productId;
                    ASSERT_TRUE(retrieveValue(registrationJson, TEST_PRODUCT_ID_KEY, &productId));
                    ASSERT_EQ(productId, TEST_PRODUCT_ID);
                    string deviceSerialNumber;
                    ASSERT_TRUE(retrieveValue(registrationJson, "deviceSerialNumber", &deviceSerialNumber));
                    ASSERT_EQ(deviceSerialNumber, TEST_SERIAL_NUMBER);
                    string connections;
                    Document document;
                    ASSERT_TRUE(parseJSON(endpointConfigJson, &document));
                    Value::ConstMemberIterator connectionsIt;
                    Value _document{document.GetString(), strlen(document.GetString())};
                    ASSERT_TRUE(jsonUtils::findNode(_document, "connections", &connectionsIt));
                    ASSERT_TRUE(connectionsIt->value.IsArray());
                    vector<map<string, string>> connectionMaps;
                    for (auto it = connectionsIt->value.Begin(); it != connectionsIt->value.End(); ++it) {
                        map<string, string> connectionMap;
                        for (auto iter = it->MemberBegin(); iter != it->MemberEnd(); ++iter) {
                            if (iter->name.IsString() && iter->value.IsString()) {
                                connectionMap[iter->name.GetString()] = iter->value.GetString();
                            }
                        }
                        connectionMaps.push_back(connectionMap);
                    }
                    ASSERT_EQ(connectionMaps.size(), 2U);
                    ASSERT_EQ(connectionMaps[0].size(), 1U);
                    ASSERT_EQ(connectionMaps[1].size(), 1U);
                    map<string, string> cookieMap = jsonUtils::retrieveStringMap(_document, "cookie");
                    ASSERT_EQ(cookieMap.size(), 2U);
                    Value::ConstMemberIterator capabilitiesIt;
                    ASSERT_TRUE(jsonUtils::findNode(_document, "capabilities", &capabilitiesIt));
                    ASSERT_TRUE(capabilitiesIt->value.IsArray());
                    vector<map<string, string>> capabilitiesMaps;
                    for (auto it = capabilitiesIt->value.Begin(); it != capabilitiesIt->value.End(); ++it) {
                        map<string, string> capabilitiesMap;
                        for (auto iter = it->MemberBegin(); iter != it->MemberEnd(); ++iter) {
                            if (iter->name.IsString() && iter->value.IsString()) {
                                capabilitiesMap[iter->name.GetString()] = iter->value.GetString();
                            } else if (
                                iter->name.IsString() && !strcmp(iter->name.GetString(), "properties") && iter->value.IsObject()) {
                                string propertiesString;
                                ASSERT_TRUE(convertToValue(iter->value, &propertiesString));
                                capabilitiesMap[iter->name.GetString()] = propertiesString;

                            } else if (
                                iter->name.IsString() && !strcmp(iter->name.GetString(), "configuration") && iter->value.IsObject()) {
                                string configurationString;
                                ASSERT_TRUE(convertToValue(iter->value, &configurationString));
                                capabilitiesMap[iter->name.GetString()] = configurationString;
                            }
                        }
                        capabilitiesMaps.push_back(capabilitiesMap);
                    }
                    ASSERT_EQ(capabilitiesMaps.size(), 1U);
                    ASSERT_EQ(capabilitiesMaps[0]["interface"], TEST_INTERFACE_NAME);
                    ASSERT_EQ(capabilitiesMaps[0]["version"], TEST_VERSION);
                    ASSERT_EQ(capabilitiesMaps[0]["type"], TEST_TYPE);
                    ASSERT_EQ(capabilitiesMaps[0]["instance"], TEST_INSTANCE_NAME.value());
                    ASSERT_EQ(capabilitiesMaps[0]["properties"], EXPECTED_TEST_PROPERTIES_STRING);
                    ASSERT_EQ(capabilitiesMaps[0]["configuration"], TEST_VALID_CONFIG_JSON);
                }
                void validateDiscoveryEvent(
                    const string& eventJson,
                    const string& expectedName,
                    const string& expectedAuthToken,
                    const vector<std::string>& expectedEndpointIds,
                    const string& expectedEventCorrelationToken = "") {
                    Document document;
                    ASSERT_TRUE(jsonUtils::parseJSON(eventJson, &document));
                    Value::ConstMemberIterator eventIt;
                    Value _document{document.GetString(), strlen(document.GetString())};
                    ASSERT_TRUE(jsonUtils::findNode(_document, "event", &eventIt));
                    Value::ConstMemberIterator headerIt;
                    ASSERT_TRUE(findNode(eventIt->value, "header", &headerIt));
                    string avsNamespace;
                    ASSERT_TRUE(retrieveValue(headerIt->value, "namespace", &avsNamespace));
                    ASSERT_EQ(avsNamespace, "Alexa.Discovery");
                    string avsName;
                    ASSERT_TRUE(retrieveValue(headerIt->value, "name", &avsName));
                    ASSERT_EQ(avsName, expectedName);
                    string payloadVersion;
                    ASSERT_TRUE(retrieveValue(headerIt->value, "payloadVersion", &payloadVersion));
                    ASSERT_EQ(payloadVersion, "3");
                    if (!expectedEventCorrelationToken.empty()) {
                        string eventCorrelationToken;
                        ASSERT_TRUE(retrieveValue(headerIt->value, "eventCorrelationToken", &eventCorrelationToken));
                        ASSERT_EQ(eventCorrelationToken, expectedEventCorrelationToken);
                    }
                    Value::ConstMemberIterator payloadIt;
                    ASSERT_TRUE(findNode(eventIt->value, "payload", &payloadIt));
                    Value::ConstMemberIterator scopeIt;
                    ASSERT_TRUE(findNode(payloadIt->value, "scope", &scopeIt));
                    string type;
                    ASSERT_TRUE(retrieveValue(scopeIt->value, "type", &type));
                    ASSERT_EQ(type, "BearerToken");
                    string authToken;
                    ASSERT_TRUE(retrieveValue(scopeIt->value, "token", &authToken));
                    ASSERT_EQ(authToken, expectedAuthToken);
                    Value::ConstMemberIterator endpointsIt;
                    ASSERT_TRUE(findNode(payloadIt->value, "endpoints", &endpointsIt));
                    ASSERT_TRUE(endpointsIt->value.IsArray());
                    vector<string> endpointIds;
                    string endpointId;
                    for (auto it = endpointsIt->value.Begin(); it != endpointsIt->value.End(); ++it) {
                        ASSERT_TRUE(retrieveValue(*it, "endpointId", &endpointId));
                        endpointIds.push_back(endpointId);
                    }
                    ASSERT_EQ(endpointIds, expectedEndpointIds);
                }
                class DiscoveryUtilsTest : public Test {};
                TEST_F(DiscoveryUtilsTest, test_validateCapabilityConfiguration) {
                    ASSERT_FALSE(validateCapabilityConfiguration(CapabilityConfiguration("", TEST_INTERFACE_NAME, TEST_VERSION)));
                    ASSERT_FALSE(validateCapabilityConfiguration(CapabilityConfiguration(TEST_TYPE, "", TEST_VERSION)));
                    ASSERT_FALSE(validateCapabilityConfiguration(CapabilityConfiguration(TEST_TYPE, TEST_INTERFACE_NAME, "")));
                    Optional<string> invalidInstanceName("");
                    ASSERT_FALSE(validateCapabilityConfiguration(CapabilityConfiguration(TEST_TYPE, TEST_INTERFACE_NAME, TEST_VERSION, invalidInstanceName)));
                    vector<string> supportedList;
                    Properties invalidSupportedListProperties(false, false, supportedList);
                    ASSERT_FALSE(validateCapabilityConfiguration(CapabilityConfiguration(TEST_TYPE, TEST_INTERFACE_NAME, TEST_VERSION, TEST_INSTANCE_NAME,
                                 Optional<Properties>(invalidSupportedListProperties))));
                    CapabilityConfiguration::AdditionalConfigurations additionalConfigurations;
                    additionalConfigurations.insert({"TEST", "abc:"});
                    ASSERT_FALSE(validateCapabilityConfiguration(CapabilityConfiguration(TEST_TYPE, TEST_INTERFACE_NAME, TEST_VERSION, TEST_INSTANCE_NAME, TEST_PROPERTIES, additionalConfigurations)));
                    ASSERT_TRUE(validateCapabilityConfiguration(CapabilityConfiguration(TEST_TYPE, TEST_INTERFACE_NAME, TEST_VERSION, TEST_INSTANCE_NAME,
                                TEST_PROPERTIES, TEST_ADDITIONAL_CONFIGURATIONS)));
                }
                TEST_F(DiscoveryUtilsTest, test_validateAVSDiscoveryEndpointAttributes) {
                    ASSERT_FALSE(validateEndpointAttributes(createEndpointAttributes("", TEST_FRIENDLY_NAME, TEST_DESCRIPTION, TEST_MANUFACTURER_NAME, TEST_DISPLAY_CATEGORIES)));
                    ASSERT_FALSE(validateEndpointAttributes(createEndpointAttributes(TEST_ENDPOINT_ID, TEST_FRIENDLY_NAME, "", TEST_MANUFACTURER_NAME, TEST_DISPLAY_CATEGORIES)));
                    ASSERT_FALSE(validateEndpointAttributes(createEndpointAttributes(TEST_ENDPOINT_ID, TEST_FRIENDLY_NAME, TEST_DESCRIPTION, "", TEST_DISPLAY_CATEGORIES)));
                    ASSERT_FALSE(validateEndpointAttributes(createEndpointAttributes(TEST_ENDPOINT_ID, TEST_FRIENDLY_NAME, TEST_DESCRIPTION, TEST_MANUFACTURER_NAME, {})));
                    ASSERT_TRUE(validateEndpointAttributes(createEndpointAttributes(TEST_ENDPOINT_ID, TEST_FRIENDLY_NAME, TEST_DESCRIPTION, TEST_MANUFACTURER_NAME, TEST_DISPLAY_CATEGORIES)));
                }
                TEST_F(DiscoveryUtilsTest, test_formatEndpointConfigJson) {
                    validateEndpointConfigJson(getEndpointConfigJson(getTestEndpointAttributes(), getTestCapabilities()));
                }
                TEST_F(DiscoveryUtilsTest, test_getDeleteReportEndpointConfigJson) {
                    EXPECT_EQ(TEST_ENDPOINT_CONFIG, getDeleteReportEndpointConfigJson(TEST_ENDPOINT_ID));
                }
                TEST_F(DiscoveryUtilsTest, test_discoveryAddOrUpdateReportEvent) {
                    vector<string> testEndpointConfigs = {TEST_ENDPOINT_CONFIG};
                    auto pair = getAddOrUpdateReportEventJson(testEndpointConfigs, TEST_AUTH_TOKEN);
                    validateDiscoveryEvent(pair.first, ADD_OR_UPDATE_REPORT_EVENT_NAME, TEST_AUTH_TOKEN, {TEST_ENDPOINT_ID});
                }
                TEST_F(DiscoveryUtilsTest, test_deleteReportEvent) {
                    vector<string> testEndpointConfigs = {TEST_ENDPOINT_CONFIG};
                    auto event = getDeleteReportEventJson(testEndpointConfigs, TEST_AUTH_TOKEN);
                    validateDiscoveryEvent(event, DELETE_REPORT_EVENT_NAME, TEST_AUTH_TOKEN, {TEST_ENDPOINT_ID});
                }
            }
        }
    }
}