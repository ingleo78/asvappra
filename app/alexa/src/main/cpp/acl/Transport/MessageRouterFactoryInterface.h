#ifndef ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_MESSAGEROUTERFACTORYINTERFACE_H_
#define ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_MESSAGEROUTERFACTORYINTERFACE_H_

#include "MessageRouterInterface.h"
#include "TransportFactoryInterface.h"

namespace alexaClientSDK {
    namespace acl {
        using namespace alexaClientSDK::acl;
        using namespace avsCommon::sdkInterfaces;
        using namespace avsCommon::avs::attachment;
        class MessageRouterFactoryInterface {
        public:
            virtual ~MessageRouterFactoryInterface() = default;
            virtual shared_ptr<MessageRouterInterface> createMessageRouter(shared_ptr<AuthDelegateInterface> authDelegate,
                                                                           shared_ptr<AttachmentManager> attachmentManager,
                                                                           shared_ptr<TransportFactoryInterface> transportFactory) = 0;
        };
    }
}
#endif