#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_AVSMESSAGEENDPOINT_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_AVSMESSAGEENDPOINT_H_

#include <map>
#include <string>
#include <util/Optional.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            struct AVSMessageEndpoint {
                AVSMessageEndpoint() = default;
                AVSMessageEndpoint(const std::string endpointId);
                const std::string endpointId;
                std::map<std::string, std::string> cookies;
            };
            inline AVSMessageEndpoint::AVSMessageEndpoint(const std::string endpointId) : endpointId(endpointId) { }
        }
    }
}
#endif