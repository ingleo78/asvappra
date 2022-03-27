#include "MessageRouterFactory.h"

namespace alexaClientSDK {
    namespace acl {
        using namespace std;
        using namespace avsCommon::sdkInterfaces;
        using namespace avsCommon::avs::attachment;
        MessageRouterFactory::MessageRouterFactory() = default;
        shared_ptr<MessageRouterInterface> MessageRouterFactory::createMessageRouter(shared_ptr<AuthDelegateInterface> authDelegate,
                                                                                     shared_ptr<AttachmentManager> attachmentManager,
                                                                                     shared_ptr<TransportFactoryInterface> transportFactory) {
            return make_shared<MessageRouter>(move(authDelegate), move(attachmentManager), move(transportFactory));
        }
    }
}