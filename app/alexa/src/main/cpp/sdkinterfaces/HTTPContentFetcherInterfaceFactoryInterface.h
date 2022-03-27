#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_HTTPCONTENTFETCHERINTERFACEFACTORYINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_HTTPCONTENTFETCHERINTERFACEFACTORYINTERFACE_H_

#include <memory>
#include <string>
#include "HTTPContentFetcherInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class HTTPContentFetcherInterfaceFactoryInterface {
            public:
                virtual ~HTTPContentFetcherInterfaceFactoryInterface() = default;
                virtual std::unique_ptr<HTTPContentFetcherInterface> create(const std::string& url);
            };
        }
    }
}
#endif