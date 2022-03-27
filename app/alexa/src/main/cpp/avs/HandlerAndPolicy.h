#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_HANDLERANDPOLICY_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_HANDLERANDPOLICY_H_

#include <memory>
#include <sdkinterfaces/DirectiveHandlerInterface.h>
#include "BlockingPolicy.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            using namespace std;
            using namespace sdkInterfaces;
            class HandlerAndPolicy {
            public:
                HandlerAndPolicy(){};
                HandlerAndPolicy(shared_ptr<DirectiveHandlerInterface>& handlerIn, BlockingPolicy policyIn);
                operator bool() const;
                shared_ptr<DirectiveHandlerInterface> handler;
                BlockingPolicy policy;
            };
            bool operator==(const HandlerAndPolicy& lhs, const HandlerAndPolicy& rhs);
            bool operator!=(const HandlerAndPolicy& lhs, const HandlerAndPolicy& rhs);
        }
    }
}
#endif