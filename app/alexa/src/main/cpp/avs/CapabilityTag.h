#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_CAPABILITYTAG_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_CAPABILITYTAG_H_

#include <ostream>
#include <string>
#include <util/Optional.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            using namespace std;
            using namespace utils;
            struct CapabilityTag {
                CapabilityTag(const string& nameSpace, const string& name, const string& endpointId, const Optional<string>& instanceId = Optional<std::string>());
                CapabilityTag(const CapabilityTag& other) = default;
                const string nameSpace;
                const string name;
                const string endpointId;
                const Optional<std::string> instance;
                bool operator<(const CapabilityTag& rhs) const;
                bool operator==(const CapabilityTag& rhs) const;
                bool operator!=(const CapabilityTag& rhs) const;
            };
            inline ostream& operator<<(ostream& stream, const CapabilityTag& identifier) {
                stream << "{endpointId:" << identifier.endpointId << ",namespace:" << identifier.nameSpace << ",name:" << identifier.name;
                if (identifier.instance.hasValue()) stream << ",instance:" << identifier.instance.value();
                stream << "}";
                return stream;
            }
        }
    }
}
namespace std {
    template <> struct hash<alexaClientSDK::avsCommon::avs::CapabilityTag> {
        size_t operator()(const alexaClientSDK::avsCommon::avs::CapabilityTag& in) const;
    };
}
#endif