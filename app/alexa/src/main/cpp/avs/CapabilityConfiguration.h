#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_CAPABILITYCONFIGURATION_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_CAPABILITYCONFIGURATION_H_

#include <map>
#include <string>
#include <unordered_map>
#include <vector>
#include <util/Optional.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            using namespace std;
            using namespace avsCommon::utils;
            static const std::string CAPABILITY_INTERFACE_TYPE_KEY = "type";
            static const std::string CAPABILITY_INTERFACE_NAME_KEY = "interface";
            static const std::string CAPABILITY_INTERFACE_VERSION_KEY = "version";
            static const std::string CAPABILITY_INTERFACE_CONFIGURATIONS_KEY = "configurations";
            struct CapabilityConfiguration {
                static constexpr const char* ALEXA_INTERFACE_TYPE = "AlexaInterface";
                using AdditionalConfigurations = map<std::string, std::string>;
                struct Properties {
                    bool isRetrievable;
                    bool isProactivelyReported;
                    vector<std::string> supportedList;
                    Optional<bool> isNonControllable;
                    Properties(bool isRetrievableIn = false, bool isProactivelyReportedIn = false, const vector<std::string>& supportedListIn = vector<std::string>(),
                               const Optional<bool>& isNonControllableIn = Optional<bool>());
                };
                CapabilityConfiguration() = default;
                CapabilityConfiguration(const unordered_map<std::string, std::string>& capabilityConfigurationIn);
                CapabilityConfiguration(const string& typeIn, const string& interfaceNameIn, const string& versionIn,
                                        const Optional<std::string>& instanceNameIn = Optional<std::string>(), const Optional<Properties>& propertiesIn = Optional<Properties>(),
                                        const AdditionalConfigurations& additionalConfigurationsIn = AdditionalConfigurations());
                std::string type;
                std::string interfaceName;
                std::string version;
                Optional<std::string> instanceName;
                Optional<Properties> properties;
                AdditionalConfigurations additionalConfigurations;
            };
            bool operator==(const CapabilityConfiguration::Properties& lhs, const CapabilityConfiguration::Properties& rhs);
            bool operator!=(const CapabilityConfiguration::Properties& lhs, const CapabilityConfiguration::Properties& rhs);
            bool operator==(const CapabilityConfiguration& lhs, const CapabilityConfiguration& rhs);
            bool operator!=(const CapabilityConfiguration& lhs, const CapabilityConfiguration& rhs);
            bool operator==(const shared_ptr<CapabilityConfiguration>& lhs, const shared_ptr<CapabilityConfiguration>& rhs);
            bool operator!=(const shared_ptr<CapabilityConfiguration>& lhs, const shared_ptr<CapabilityConfiguration>& rhs);
        }
    }
}
namespace std {
    using namespace alexaClientSDK::avsCommon::avs;
    template <>struct hash<CapabilityConfiguration> {
        size_t operator()(const CapabilityConfiguration& in) const;
    };
    template <> struct hash<shared_ptr<CapabilityConfiguration>> {
        size_t operator()(const shared_ptr<CapabilityConfiguration>& in) const;
    };
}
#endif