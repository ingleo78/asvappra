#ifndef ALEXA_CLIENT_SDK_ENDPOINTS_INCLUDE_ENDPOINTS_ENDPOINTATTRIBUTEVALIDATION_H_
#define ALEXA_CLIENT_SDK_ENDPOINTS_INCLUDE_ENDPOINTS_ENDPOINTATTRIBUTEVALIDATION_H_

#include <map>
#include <string>
#include <avs/AVSDiscoveryEndpointAttributes.h>

namespace alexaClientSDK {
    namespace endpoints {
        using namespace std;
        using namespace avsCommon;
        using namespace avs;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace logger;
        using namespace sdkInterfaces::endpoints;
        using AdditionalAttributes = AVSDiscoveryEndpointAttributes::AdditionalAttributes;
        bool isEndpointIdValid(const EndpointIdentifier& identifier);
        bool isFriendlyNameValid(const string& name);
        bool isDescriptionValid(const string& description);
        bool isManufacturerNameValid(const string& manufacturerName);
        bool isAdditionalAttributesValid(const AdditionalAttributes& attributes);
        bool areConnectionsValid(const vector<map<string, string>>& connections);
        bool areCookiesValid(const map<string, string>& cookies);
    }
}
#endif