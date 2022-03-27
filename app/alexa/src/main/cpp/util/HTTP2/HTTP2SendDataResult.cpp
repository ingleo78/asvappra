#include "HTTP2SendDataResult.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace http2 {
                const HTTP2SendDataResult HTTP2SendDataResult::PAUSE{HTTP2SendStatus::PAUSE, 0};
                const HTTP2SendDataResult HTTP2SendDataResult::COMPLETE{HTTP2SendStatus::COMPLETE, 0};
                const HTTP2SendDataResult HTTP2SendDataResult::ABORT{HTTP2SendStatus::ABORT, 0};
            }
        }
    }
}
