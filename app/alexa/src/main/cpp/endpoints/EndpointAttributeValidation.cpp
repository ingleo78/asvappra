#include "EndpointAttributeValidation.h"

namespace alexaClientSDK {
    namespace endpoints {
        constexpr size_t EXTRA_BYTES_PER_COOKIE = 6;
        bool isEndpointIdValid(const EndpointIdentifier& identifier) {
            auto length = identifier.length();
            return (length > 0) && (length <= AVSDiscoveryEndpointAttributes::MAX_ENDPOINT_IDENTIFIER_LENGTH);
        }
        bool isFriendlyNameValid(const string& name) {
            auto length = name.length();
            return (length > 0) && (length <= AVSDiscoveryEndpointAttributes::MAX_FRIENDLY_NAME_LENGTH);
        }
        bool isDescriptionValid(const string& description) {
            auto length = description.length();
            return (length > 0) && (length <= AVSDiscoveryEndpointAttributes::MAX_DESCRIPTION_LENGTH);
        }
        bool isManufacturerNameValid(const string& manufacturerName) {
            auto length = manufacturerName.length();
            return (length > 0) && (length <= AVSDiscoveryEndpointAttributes::MAX_MANUFACTURER_NAME_LENGTH);
        }
        bool isAdditionalAttributesValid(const AVSDiscoveryEndpointAttributes::AdditionalAttributes& attributes) {
            return ((attributes.manufacturer.length() <= AVSDiscoveryEndpointAttributes::MAX_ENDPOINT_ADDITIONAL_ATTRIBUTES_LENGTH) &&
                    (attributes.model.length() <= AVSDiscoveryEndpointAttributes::MAX_ENDPOINT_ADDITIONAL_ATTRIBUTES_LENGTH) &&
                    (attributes.serialNumber.length() <= AVSDiscoveryEndpointAttributes::MAX_ENDPOINT_ADDITIONAL_ATTRIBUTES_LENGTH) &&
                    (attributes.firmwareVersion.length() <= AVSDiscoveryEndpointAttributes::MAX_ENDPOINT_ADDITIONAL_ATTRIBUTES_LENGTH) &&
                    (attributes.softwareVersion.length() <= AVSDiscoveryEndpointAttributes::MAX_ENDPOINT_ADDITIONAL_ATTRIBUTES_LENGTH) &&
                    (attributes.customIdentifier.length() <= AVSDiscoveryEndpointAttributes::MAX_ENDPOINT_ADDITIONAL_ATTRIBUTES_LENGTH));
        }
        bool areConnectionsValid(const vector<map<string, string>>& connections) {
            for (auto& connection : connections) {
                for (auto& keyValuePair : connection) {
                    if (keyValuePair.second.length() == 0 || keyValuePair.second.length() > AVSDiscoveryEndpointAttributes::MAX_CONNECTIONS_VALUE_LENGTH) {
                        return false;
                    }
                }
            }
            return true;
        }
        bool areCookiesValid(const map<string, string>& cookies) {
            size_t totalSizeBytes = 0;
            for (auto& cookie : cookies) {
                totalSizeBytes += cookie.first.size() + cookie.second.size() + EXTRA_BYTES_PER_COOKIE;
            }
            return totalSizeBytes < AVSDiscoveryEndpointAttributes::MAX_COOKIES_SIZE_BYTES;
        }
    }
}