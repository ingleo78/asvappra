#ifndef ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_TRANSPORTDEFINES_H_
#define ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_TRANSPORTDEFINES_H_

#include <util/RetryTimer.h>

namespace alexaClientSDK {
    namespace acl {
        class TransportDefines {
        public:
            const static std::vector<int> RETRY_TABLE;
            static avsCommon::utils::RetryTimer RETRY_TIMER;
            static avsCommon::utils::RetryTimer getRetryTimer() {
                return RETRY_TIMER;
            }
        };
    }
}
#endif