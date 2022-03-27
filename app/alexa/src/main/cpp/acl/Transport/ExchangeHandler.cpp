#include <string>
#include <logger/Logger.h>
#include "HTTP2Transport.h"
#include "ExchangeHandler.h"

namespace alexaClientSDK {
    namespace acl {
        using namespace std;
        using namespace avsCommon::utils::logger;
        static const string AUTHORIZATION_HEADER = "Authorization: Bearer ";
        static const string TAG("ExchangeHandler");
        #define LX(event) LogEntry(TAG, event)
        ExchangeHandler::ExchangeHandler(shared_ptr<ExchangeHandlerContextInterface> context, const string& authToken) : m_context{context}, m_authToken{authToken},
                                         m_authHeader{AUTHORIZATION_HEADER + authToken} {
            ACSDK_DEBUG5(LX(__func__).d("context", context.get()).sensitive("authToken", authToken));
            if (m_authToken.empty()) {
                ACSDK_ERROR(LX(__func__).m("emptyAuthToken"));
            }
        }
    }
}