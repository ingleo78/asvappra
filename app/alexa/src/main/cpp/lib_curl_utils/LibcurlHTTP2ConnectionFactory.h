#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LIBCURLUTILS_LIBCURLHTTP2CONNECTIONFACTORY_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LIBCURLUTILS_LIBCURLHTTP2CONNECTIONFACTORY_H_

#include <util/HTTP2/HTTP2ConnectionFactoryInterface.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace libcurlUtils {
                class LibcurlHTTP2ConnectionFactory : public avsCommon::utils::http2::HTTP2ConnectionFactoryInterface {
                public:
                    std::shared_ptr<avsCommon::utils::http2::HTTP2ConnectionInterface> createHTTP2Connection() override;
                };
            }
        }
    }
}
#endif