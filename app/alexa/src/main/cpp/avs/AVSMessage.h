#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_AVSMESSAGE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_AVSMESSAGE_H_

#include <memory>
#include <string>
#include <util/Optional.h>
#include "AVSMessageEndpoint.h"
#include "AVSMessageHeader.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            using namespace std;
            using namespace utils;
            class AVSMessage {
            public:
                AVSMessage(shared_ptr<AVSMessageHeader> avsMessageHeader, string payload, const Optional<AVSMessageEndpoint>& endpoint = Optional<AVSMessageEndpoint>());
                virtual ~AVSMessage() = default;
                string getNamespace() const;
                string getName() const;
                string getMessageId() const;
                string getCorrelationToken() const;
                string getEventCorrelationToken() const;
                string getPayloadVersion() const;
                string getInstance() const;
                string getDialogRequestId() const;
                string getPayload() const;
                shared_ptr<const AVSMessageHeader> getHeader() const;
                string getHeaderAsString() const;
                Optional<AVSMessageEndpoint> getEndpoint() const;
            private:
                const shared_ptr<AVSMessageHeader> m_header;
                const string m_payload;
                const Optional<AVSMessageEndpoint> m_endpoint;
            };
        }
    }
}
#endif