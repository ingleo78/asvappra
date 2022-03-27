#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_EVENTBUILDER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_EVENTBUILDER_H_

#include <string>
#include <unordered_set>
#include <utility>
#include <util/Optional.h>
#include "AVSContext.h"
#include "AVSMessageEndpoint.h"
#include "AVSMessageHeader.h"
#include "CapabilityConfiguration.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            namespace constants {
                static const string NAMESPACE_KEY_STRING = "namespace";
                static const string NAME_KEY_STRING = "name";
                static const string HEADER_KEY_STRING = "header";
                static const string PAYLOAD_KEY_STRING = "payload";
            }
            const pair<string, string> buildJsonEventString(const string& nameSpace, const string& eventName, const string& dialogRequestIdValue = "",
                                                            const string& jsonPayloadValue = "{}", const string& jsonContext = "");
            std::string buildJsonEventString(const AVSMessageHeader& eventHeader, const Optional<AVSMessageEndpoint>& endpoint = Optional<AVSMessageEndpoint>(),
                                             const string& jsonPayloadValue = "{}",const Optional<AVSContext>& context = Optional<AVSContext>());
        }
    }
}
#endif