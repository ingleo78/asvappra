#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_DIRECTIVEROUTINGRULE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_DIRECTIVEROUTINGRULE_H_

#include "CapabilityTag.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            namespace directiveRoutingRule {
                using DirectiveRoutingRule = CapabilityTag;
                using namespace std;
                using namespace utils;
                DirectiveRoutingRule routingRulePerDirective(const string& endpointId, const Optional<string>& instance, const string& nameSpace, const string& name);
                DirectiveRoutingRule routingRulePerNamespace(const string& endpointId,const Optional<string>& instance, const string& nameSpace);
                DirectiveRoutingRule routingRulePerInstance(const string& endpointId, const Optional<string>& instance);
                DirectiveRoutingRule routingRulePerNamespaceAnyInstance(const string& endpointId, const string& nameSpace);
                DirectiveRoutingRule routingRulePerEndpoint(const string& endpointId);
                bool isDirectiveRoutingRuleValid(const DirectiveRoutingRule& rule);
            }
        }
    }
}
#endif