#include <vector>
#include "TransportDefines.h"

namespace alexaClientSDK {
    namespace acl {
        const std::vector<int> TransportDefines::RETRY_TABLE = {250, 1000, 2000, 4000, 8000, 16000, 32000, 64000};
        avsCommon::utils::RetryTimer TransportDefines::RETRY_TIMER(TransportDefines::RETRY_TABLE);
    }
}