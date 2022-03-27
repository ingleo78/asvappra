#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_NAMESPACEANDNAME_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_NAMESPACEANDNAME_H_

#include <string>
#include "CapabilityTag.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            class NamespaceAndName : public CapabilityTag {
            public:
                NamespaceAndName();
                NamespaceAndName(const std::string& nameSpaceIn, const std::string& nameIn);
                NamespaceAndName(const CapabilityTag& identifier);
            };
        }
    }
}
namespace std {
    template <> struct hash<alexaClientSDK::avsCommon::avs::NamespaceAndName> {
        size_t operator()(const alexaClientSDK::avsCommon::avs::NamespaceAndName& in) const;
    };
}
#endif