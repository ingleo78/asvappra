#include <algorithm>
#include <utility>
#include <vector>
#include "CapabilityConfiguration.h"
#include "functional/hash.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            using namespace std;
            using namespace utils;
            CapabilityConfiguration::Properties::Properties(bool isRetrievableIn, bool isProactivelyReportedIn, const vector<std::string>& supportedListIn,
                                                            const Optional<bool>& isNonControllableIn) : isRetrievable{isRetrievableIn},
                                                            isProactivelyReported{isProactivelyReportedIn}, supportedList{supportedListIn},
                                                            isNonControllable{isNonControllableIn} {}
            CapabilityConfiguration::CapabilityConfiguration(const unordered_map<std::string, std::string>& capabilityConfigurationIn) {
                auto it = capabilityConfigurationIn.find(CAPABILITY_INTERFACE_TYPE_KEY);
                if (it != capabilityConfigurationIn.end()) type = it->second;
                it = capabilityConfigurationIn.find(CAPABILITY_INTERFACE_VERSION_KEY);
                if (it != capabilityConfigurationIn.end()) version = it->second;
                it = capabilityConfigurationIn.find(CAPABILITY_INTERFACE_NAME_KEY);
                if (it != capabilityConfigurationIn.end()) interfaceName = it->second;
                it = capabilityConfigurationIn.find(CAPABILITY_INTERFACE_CONFIGURATIONS_KEY);
                if (it != capabilityConfigurationIn.end()) additionalConfigurations.insert({it->first, it->second});
            }
            CapabilityConfiguration::CapabilityConfiguration(const std::string& typeIn, const std::string& interfaceNameIn, const std::string& versionIn,
                                                             const Optional<std::string>& instanceNameIn, const Optional<Properties>& propertiesIn,
                                                             const CapabilityConfiguration::AdditionalConfigurations& additionalConfigurationsIn) : type{typeIn},
                                                             interfaceName{interfaceNameIn}, version{versionIn}, instanceName{instanceNameIn}, properties{propertiesIn},
                                                             additionalConfigurations{additionalConfigurationsIn} {}
            bool operator==(const CapabilityConfiguration::Properties& lhs, const CapabilityConfiguration::Properties& rhs) {
                if ((lhs.isRetrievable != rhs.isRetrievable) || (lhs.isProactivelyReported != rhs.isProactivelyReported) ||
                    (lhs.isNonControllable != rhs.isNonControllable)) {
                    return false;
                }
                if (lhs.supportedList.size() != rhs.supportedList.size()) return false;
                for (size_t i = 0; i < lhs.supportedList.size(); ++i) {
                    if (lhs.supportedList[i] != rhs.supportedList[i]) return false;
                }
                return true;
            }
            bool operator!=(const CapabilityConfiguration::Properties& lhs, const CapabilityConfiguration::Properties& rhs) {
                return !(lhs == rhs);
            }
            bool operator==(const CapabilityConfiguration& lhs, const CapabilityConfiguration& rhs) {
                if ((lhs.interfaceName != rhs.interfaceName) || (lhs.version != rhs.version) || (lhs.type != rhs.type) ||
                    (lhs.instanceName != rhs.instanceName) || (lhs.properties != rhs.properties)) {
                    return false;
                }
                if (lhs.additionalConfigurations.size() != rhs.additionalConfigurations.size()) return false;
                for (const auto& lhsIterator : lhs.additionalConfigurations) {
                    string lhsKey = lhsIterator.first;
                    string lhsValue = lhsIterator.second;
                    auto rhsIterator = rhs.additionalConfigurations.find(lhsKey);
                    if (rhsIterator == rhs.additionalConfigurations.end()) return false;
                    string rhsValue = rhsIterator->second;
                    if (lhsValue.compare(rhsValue) != 0) return false;
                }
                return true;
            }
            bool operator!=(const CapabilityConfiguration& lhs, const CapabilityConfiguration& rhs) { return !(lhs == rhs); }
            bool operator==(const shared_ptr<CapabilityConfiguration>& lhs, const shared_ptr<CapabilityConfiguration>& rhs) {
                if (!lhs && !rhs) return true;
                if ((!lhs && rhs) || (lhs && !rhs)) return false;
                return (*lhs == *rhs);
            }
            bool operator!=(const shared_ptr<CapabilityConfiguration>& lhs, const shared_ptr<CapabilityConfiguration>& rhs) {
                return !(lhs == rhs);
            }
        }
    }
}
namespace std {
    using namespace std;
    using namespace alexaClientSDK::avsCommon::avs;
    using namespace alexaClientSDK::avsCommon::utils::functional;
    size_t hash<CapabilityConfiguration>::operator()(
        const CapabilityConfiguration& in) const {
        size_t seed = 0;
        hashCombine(seed, in.interfaceName);
        hashCombine(seed, in.type);
        hashCombine(seed, in.instanceName.valueOr(""));
        return seed;
    };
    size_t hash<shared_ptr<CapabilityConfiguration>>::operator()(
        const shared_ptr<CapabilityConfiguration>& in) const {
        size_t seed = 0;
        if (in) {
            hashCombine(seed, in->interfaceName);
            hashCombine(seed, in->type);
            hashCombine(seed, in->instanceName.valueOr(""));
        }
        return seed;
    };
}