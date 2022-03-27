#ifndef ALEXA_CLIENT_SDK_CAPABILITIESDELEGATE_INCLUDE_CAPABILITIESDELEGATE_UTILS_DISCOVERYUTILS_H_
#define ALEXA_CLIENT_SDK_CAPABILITIESDELEGATE_INCLUDE_CAPABILITIESDELEGATE_UTILS_DISCOVERYUTILS_H_

#include <map>
#include <utility>
#include <string>
#include <vector>
#include <avs/AVSDiscoveryEndpointAttributes.h>
#include <avs/CapabilityConfiguration.h>

namespace alexaClientSDK {
    namespace capabilitiesDelegate {
        namespace utils {
            using namespace std;
            using namespace avsCommon;
            using namespace avs;
            using namespace utils;
            using namespace sdkInterfaces;
            bool validateCapabilityConfiguration(const CapabilityConfiguration& capabilityConfig);
            bool validateEndpointAttributes(const AVSDiscoveryEndpointAttributes& endpointAttributes);
            string getEndpointConfigJson(const AVSDiscoveryEndpointAttributes& endpointAttributes, const vector<CapabilityConfiguration>& capabilities);
            pair<string, string> getAddOrUpdateReportEventJson(const vector<string>& endpointConfigurations, const string& authToken);
            string getDeleteReportEndpointConfigJson(const string& endpointId);
            string getDeleteReportEventJson(const vector<string>& endpointConfigurations, const string& authToken);
        }
    }
}
#endif