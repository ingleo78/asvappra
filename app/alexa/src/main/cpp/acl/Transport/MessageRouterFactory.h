#ifndef ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_MESSAGEROUTERFACTORY_H_
#define ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_MESSAGEROUTERFACTORY_H_

#include <memory>
#include "MessageRouter.h"
#include "MessageRouterFactoryInterface.h"

namespace alexaClientSDK {
    namespace acl {
        using namespace std;
        using namespace alexaClientSDK::acl;
        using namespace avsCommon::sdkInterfaces;
        using namespace avsCommon::avs::attachment;
        class MessageRouterFactory : public MessageRouterFactoryInterface {
        public:
            MessageRouterFactory();
            shared_ptr<MessageRouterInterface> createMessageRouter(shared_ptr<AuthDelegateInterface> authDelegate, shared_ptr<AttachmentManager> attachmentManager,
                                                                   shared_ptr<TransportFactoryInterface> transportFactory) override;
        };
    }
}
#endif