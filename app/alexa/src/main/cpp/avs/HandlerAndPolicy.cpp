#include <tuple>
#include <sdkinterfaces/DirectiveHandlerInterface.h>
#include "HandlerAndPolicy.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            using namespace std;
            using namespace sdkInterfaces;
            HandlerAndPolicy::HandlerAndPolicy(shared_ptr<DirectiveHandlerInterface>& handlerIn, BlockingPolicy policyIn) : handler{handlerIn}, policy{policyIn} {}
            HandlerAndPolicy::operator bool() const {
                return handler && policy.isValid();
            }
            bool operator==(const HandlerAndPolicy& lhs, const HandlerAndPolicy& rhs) {
                return tie(lhs.handler, lhs.policy) == tie(rhs.handler, rhs.policy);
            }
            bool operator!=(const HandlerAndPolicy& lhs, const HandlerAndPolicy& rhs) {
                return !(lhs == rhs);
            }
        }
    }
}