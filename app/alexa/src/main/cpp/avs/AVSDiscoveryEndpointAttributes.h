#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_AVSDISCOVERYENDPOINTATTRIBUTES_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_AVSDISCOVERYENDPOINTATTRIBUTES_H_

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <endpoints/EndpointIdentifier.h>
#include <util/Optional.h>

typedef unsigned int size_t;

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            using namespace std;
            using namespace sdkInterfaces::endpoints;
            using namespace utils;
            struct AVSDiscoveryEndpointAttributes {
                struct Registration {
                    string productId;
                    string serialNumber;
                    string registrationKey;
                    string productIdKey;
                    Registration(const string& productId, const string& serialNumber, const string& registrationKey, const string& productIdKey);
                    Registration() = default;
                };
                struct AdditionalAttributes {
                    string manufacturer;
                    string model;
                    string serialNumber;
                    string firmwareVersion;
                    string softwareVersion;
                    string customIdentifier;
                };
                static constexpr size_t MAX_ENDPOINT_IDENTIFIER_LENGTH = 256;
                static constexpr size_t MAX_FRIENDLY_NAME_LENGTH = 128;
                static constexpr size_t MAX_MANUFACTURER_NAME_LENGTH = 128;
                static constexpr size_t MAX_DESCRIPTION_LENGTH = 128;
                static constexpr size_t MAX_ENDPOINT_ADDITIONAL_ATTRIBUTES_LENGTH = 256;
                static constexpr size_t MAX_CONNECTIONS_VALUE_LENGTH = 256;
                static constexpr size_t MAX_COOKIES_SIZE_BYTES = 5000;
                EndpointIdentifier endpointId;
                string_view friendlyName;
                string_view description;
                string_view manufacturerName;
                vector<string> displayCategories;
                Optional<Registration> registration;
                Optional<AdditionalAttributes> additionalAttributes;
                vector<map<string, string>> connections;
                map<string, string> cookies;
            };
            inline AVSDiscoveryEndpointAttributes::Registration::Registration(const string& productId, const string& serialNumber,
                                                                              const string& registrationKey, const string& productIdKey) :
                                                                              productId(productId), serialNumber(serialNumber), registrationKey(registrationKey),
                                                                              productIdKey(productIdKey) {}
        }
    }
}
#endif