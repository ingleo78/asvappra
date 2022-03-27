#ifndef ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_EXCHANGEHANDLER_H_
#define ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_EXCHANGEHANDLER_H_

#include <memory>
#include "ExchangeHandlerContextInterface.h"

namespace alexaClientSDK {
    namespace acl {
        class HTTP2Transport;
        class ExchangeHandler {
        public:
            ExchangeHandler(std::shared_ptr<ExchangeHandlerContextInterface> context, const std::string& authToken);
            virtual ~ExchangeHandler() = default;
        protected:
            std::shared_ptr<ExchangeHandlerContextInterface> m_context;
            const std::string m_authToken;
            const std::string m_authHeader;
        };
    }
}
#endif