#include <logger/Logger.h>
#include "DirectiveRoutingRule.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            namespace directiveRoutingRule {
                using namespace std;
                using namespace utils;
                using namespace logger;
                static const string TAG("DirectiveRoutingRule");
                #define LX(event) LogEntry(TAG, event)
                static const string WILDCARD = "*";
                DirectiveRoutingRule routingRulePerDirective(const string& endpointId,const Optional<string>& instance, const string& nameSpace, const string& name) {
                    return DirectiveRoutingRule(nameSpace, name, endpointId, instance);
                }
                DirectiveRoutingRule routingRulePerNamespace(const string& endpointId, const Optional<string>& instance, const string& nameSpace) {
                    return DirectiveRoutingRule(nameSpace, WILDCARD, endpointId, instance);
                }
                DirectiveRoutingRule routingRulePerInstance(const string& endpointId, const Optional<string>& instance) {
                    return DirectiveRoutingRule(WILDCARD, WILDCARD, endpointId, instance);
                }
                DirectiveRoutingRule routingRulePerNamespaceAnyInstance(const string& endpointId, const string& nameSpace) {
                    return DirectiveRoutingRule(nameSpace, WILDCARD, endpointId, WILDCARD);
                }
                DirectiveRoutingRule routingRulePerEndpoint(const string& endpointId) {
                    return DirectiveRoutingRule(WILDCARD, WILDCARD, endpointId, WILDCARD);
                }
                bool isDirectiveRoutingRuleValid(const DirectiveRoutingRule& rule) {
                    if (rule.endpointId == WILDCARD) {
                        ACSDK_ERROR(LX("isDirectiveRoutingRuleValidFailed").d("result", "invalidEndpointWildcard"));
                        return false;
                    }
                    if (((rule.instance.valueOr("") == WILDCARD) || (rule.nameSpace == WILDCARD)) && (rule.name != WILDCARD)) {
                        ACSDK_ERROR(LX("isDirectiveRoutingRuleValidFailed").d("result", "invalidWildcardUsage").d("instance", rule.instance.valueOr(""))
                                        .d("namespace", rule.nameSpace).d("name", rule.name));
                        return false;
                    }
                    return true;
                }
            }
        }
    }
}