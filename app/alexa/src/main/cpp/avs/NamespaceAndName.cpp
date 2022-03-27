#include <tuple>
#include <functional/hash.h>
#include "NamespaceAndName.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            using namespace std;
            NamespaceAndName::NamespaceAndName(const string& nameSpaceIn, const string& nameIn) : CapabilityTag{nameSpaceIn, nameIn, ""} {}
            NamespaceAndName::NamespaceAndName() : CapabilityTag{"", "", ""} {}
            NamespaceAndName::NamespaceAndName(const CapabilityTag& identifier) : CapabilityTag(identifier) {}
        }
    }
}
namespace std {
    using namespace alexaClientSDK::avsCommon::avs;
    using namespace alexaClientSDK::avsCommon::utils;
    using namespace alexaClientSDK::avsCommon::utils::functional;
    size_t hash<NamespaceAndName>::operator()(const NamespaceAndName& in) const {
        size_t seed = 0;
        hashCombine(seed, in.nameSpace);
        hashCombine(seed, in.name);
        return seed;
    };
}