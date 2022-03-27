#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_DIRECTIVEHANDLERCONFIGURATION_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_DIRECTIVEHANDLERCONFIGURATION_H_

#include <unordered_map>
#include "BlockingPolicy.h"
#include "DirectiveRoutingRule.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            using DirectiveHandlerConfiguration = std::unordered_map<directiveRoutingRule::DirectiveRoutingRule, BlockingPolicy>;
        }
    }
}
#endif