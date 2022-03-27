#include "HTTP2GetMimeHeadersResult.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace http2 {
                const HTTP2GetMimeHeadersResult HTTP2GetMimeHeadersResult::PAUSE{HTTP2SendStatus::PAUSE, {}};
                const HTTP2GetMimeHeadersResult HTTP2GetMimeHeadersResult::COMPLETE{HTTP2SendStatus::COMPLETE, {}};
                const HTTP2GetMimeHeadersResult HTTP2GetMimeHeadersResult::ABORT{HTTP2SendStatus::ABORT, {}};
            }
        }
    }
}
