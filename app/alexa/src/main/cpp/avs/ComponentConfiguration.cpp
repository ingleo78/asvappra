#include <functional/hash.h>
#include <logger/Logger.h>
#include "ComponentConfiguration.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            using namespace std;
            using namespace utils;
            using namespace logger;
            static const string TAG("ComponentConfiguration");
            #define LX(event) LogEntry(TAG, event)
            static bool isValidConfiguration(string name, string version) {
                for (auto it = version.begin(); it != version.end(); ++it) {
                    const unsigned char& c = *it;
                    if (!isalnum(c) && c != '.') {
                        ACSDK_ERROR(LX(__func__).m("invalid component version").d("name", name).d("version", version));
                        return false;
                    }
                    if (c == '.' && it + 1 != version.end() && *(it + 1) == '.') {
                        ACSDK_ERROR(LX(__func__).m("invalid component version").d("name", name).d("version", version));
                        return false;
                    }
                }
                if (name.length() == 0 || version.length() == 0) {
                    ACSDK_ERROR(LX(__func__).m("component can not be empty").d("name", name).d("version", version));
                    return false;
                }
                return true;
            }
            ComponentConfiguration::ComponentConfiguration(string name, string version) : name{name}, version{version} {}
            shared_ptr<ComponentConfiguration> ComponentConfiguration::createComponentConfiguration(string name, string version) {
                if (isValidConfiguration(name, version)) return make_shared<ComponentConfiguration>(ComponentConfiguration({name, version}));
                return nullptr;
            }
            bool operator==(const ComponentConfiguration& lhs, const ComponentConfiguration& rhs) {
                return lhs.name == rhs.name;
            }
            bool operator!=(const ComponentConfiguration& lhs, const ComponentConfiguration& rhs) {
                return !(lhs == rhs);
            }
            bool operator==(const shared_ptr<ComponentConfiguration> lhs, const shared_ptr<ComponentConfiguration> rhs) {
                if (!lhs && !rhs) return true;
                if ((!lhs && rhs) || (lhs && !rhs)) return false;
                return *lhs == *rhs;
            }
            bool operator!=(const shared_ptr<ComponentConfiguration> lhs, const shared_ptr<ComponentConfiguration> rhs) {
                return !(lhs == rhs);
            }
        }
    }
}
namespace std {
    size_t hash<alexaClientSDK::avsCommon::avs::ComponentConfiguration>::operator()(
        const alexaClientSDK::avsCommon::avs::ComponentConfiguration& in) const {
        std::size_t seed = 0;
        alexaClientSDK::avsCommon::utils::functional::hashCombine(seed, in.name);
        return seed;
    };
    size_t hash<std::shared_ptr<alexaClientSDK::avsCommon::avs::ComponentConfiguration>>::operator()(
        const std::shared_ptr<alexaClientSDK::avsCommon::avs::ComponentConfiguration> in) const {
        std::size_t seed = 0;
        alexaClientSDK::avsCommon::utils::functional::hashCombine(seed, in->name);
        return seed;
    };
}