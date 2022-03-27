#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_AVSMESSAGEHEADER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_AVSMESSAGEHEADER_H_

#include <string>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            using namespace std;
            class AVSMessageHeader {
            public:
                AVSMessageHeader(const string& avsNamespace, const string& avsName, const string& avsMessageId, const string& avsDialogRequestId = "",
                                 const string& correlationToken = "", const string& eventCorrelationToken = "", const string& payloadVersion = "",
                                 const string& instance = "");
                static AVSMessageHeader createAVSEventHeader(const string& avsNamespace, const string& avsName, const string& avsDialogRequestId = "",
                                                             const string& correlationToken = "", const string& payloadVersion = "", const string& instance = "");
                string getNamespace() const;
                string getName() const;
                string getMessageId() const;
                string getDialogRequestId() const;
                string getCorrelationToken() const;
                string getEventCorrelationToken() const;
                string getPayloadVersion() const;
                string getInstance() const;
                string getAsString() const;
                string toJson() const;
            private:
                const string m_namespace;
                const string m_name;
                const string m_messageId;
                const string m_dialogRequestId;
                const string m_correlationToken;
                const string m_eventCorrelationToken;
                const string m_payloadVersion;
                const string m_instance;
            };
        }
    }
}
#endif