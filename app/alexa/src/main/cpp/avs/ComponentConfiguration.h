#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_COMPONENTCONFIGURATION_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_COMPONENTCONFIGURATION_H_

#include <functional>
#include <memory>
#include <string>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            struct ComponentConfiguration {
                static std::shared_ptr<ComponentConfiguration> createComponentConfiguration(std::string name, std::string version);
                std::string name;
                std::string version;
            private:
                ComponentConfiguration(std::string name, std::string version);
            };
            bool operator==(const ComponentConfiguration& lhs, const ComponentConfiguration& rhs);
            bool operator!=(const ComponentConfiguration& lhs, const ComponentConfiguration& rhs);
            bool operator==(const std::shared_ptr<ComponentConfiguration> lhs, const std::shared_ptr<ComponentConfiguration> rhs);
            bool operator!=(const std::shared_ptr<ComponentConfiguration> lhs, const std::shared_ptr<ComponentConfiguration> rhs);
        }
    }
}
namespace std {
    template <> struct hash<alexaClientSDK::avsCommon::avs::ComponentConfiguration> {
        size_t operator()(const alexaClientSDK::avsCommon::avs::ComponentConfiguration& in) const;
    };
    template <> struct hash<std::shared_ptr<alexaClientSDK::avsCommon::avs::ComponentConfiguration>> {
        size_t operator()(const std::shared_ptr<alexaClientSDK::avsCommon::avs::ComponentConfiguration> in) const;
    };
}
#endif