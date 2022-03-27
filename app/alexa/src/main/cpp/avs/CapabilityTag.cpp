#include <tuple>
#include <functional/hash.h>
#include "CapabilityTag.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            using namespace std;
            using namespace utils;
            CapabilityTag::CapabilityTag(const string& nameSpaceIn, const string& nameIn, const string& endpointIdIn, const Optional<std::string>& instanceIdIn) :
                                         nameSpace{nameSpaceIn}, name{nameIn}, endpointId{endpointIdIn}, instance{instanceIdIn} {}
            bool CapabilityTag::operator<(const CapabilityTag& rhs) const {
                return tie(nameSpace, name, endpointId, instance) < tie(rhs.nameSpace, rhs.name, rhs.endpointId, rhs.instance);
            }
            bool CapabilityTag::operator==(const CapabilityTag& rhs) const {
                return tie(nameSpace, name, endpointId, instance) == tie(rhs.nameSpace, rhs.name, rhs.endpointId, rhs.instance);
            }
            bool CapabilityTag::operator!=(const CapabilityTag& rhs) const {
                return !(*this == rhs);
            }
        }
    }
}
namespace std {
    using namespace alexaClientSDK::avsCommon::avs;
    using namespace alexaClientSDK::avsCommon::utils::functional;
    size_t hash<CapabilityTag>::operator()(const CapabilityTag& in) const {
        size_t seed = 0;
        hashCombine(seed, in.nameSpace);
        hashCombine(seed, in.name);
        hashCombine(seed, in.endpointId);
        hashCombine(seed, in.instance.valueOr(""));
        return seed;
    };
}