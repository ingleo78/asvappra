#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2REQUESTINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2REQUESTINTERFACE_H_

#include </media/draico/Informacion/AndroidSDK/ndk/21.3.6528147/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/c++/v1/string>
#include "HTTP2RequestSourceInterface.h"
#include "HTTP2ResponseSinkInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace http2 {
                class HTTP2RequestInterface {
                    public:
                        virtual ~HTTP2RequestInterface() = default;
                        virtual bool cancel() = 0;
                        virtual std::string getId() const = 0;
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2REQUESTINTERFACE_H_
