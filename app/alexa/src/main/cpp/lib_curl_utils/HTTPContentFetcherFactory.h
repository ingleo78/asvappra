#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LIBCURLUTILS_HTTPCONTENTFETCHERFACTORY_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LIBCURLUTILS_HTTPCONTENTFETCHERFACTORY_H_

#include <memory>
#include <string>
#include <sdkinterfaces/HTTPContentFetcherInterface.h>
#include <sdkinterfaces/HTTPContentFetcherInterfaceFactoryInterface.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace libcurlUtils {
                using namespace std;
                using namespace sdkInterfaces;
                class HTTPContentFetcherFactory : public HTTPContentFetcherInterfaceFactoryInterface {
                public:
                    unique_ptr<HTTPContentFetcherInterface> create(const string& url) override;
                };
            }
        }
    }
}
#endif