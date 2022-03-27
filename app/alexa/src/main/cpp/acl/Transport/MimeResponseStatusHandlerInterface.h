#ifndef ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_MIMERESPONSESTATUSHANDLERINTERFACE_H_
#define ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_MIMERESPONSESTATUSHANDLERINTERFACE_H_

#include <string>
#include <util/HTTP2/HTTP2ResponseFinishedStatus.h>

namespace alexaClientSDK {
    namespace acl {
        using namespace std;
        using namespace avsCommon::utils::http2;
        class MimeResponseStatusHandlerInterface {
        public:
            virtual ~MimeResponseStatusHandlerInterface() = default;
            virtual void onActivity() = 0;
            virtual bool onReceiveResponseCode(long responseCode) = 0;
            virtual void onResponseFinished(HTTP2ResponseFinishedStatus status, const string& nonMimeBody) = 0;
        };
    }
}
#endif