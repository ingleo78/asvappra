#ifndef ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_TRANSPORTFACTORYINTERFACE_H_
#define ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_TRANSPORTFACTORYINTERFACE_H_

#include <memory>
#include <string>
#include <sdkinterfaces/AuthDelegateInterface.h>
#include <avs/attachment/AttachmentManager.h>
#include "TransportInterface.h"
#include "MessageConsumerInterface.h"
#include "TransportObserverInterface.h"
#include "SynchronizedMessageRequestQueue.h"

namespace alexaClientSDK {
    namespace acl {
        using namespace std;
        using namespace avsCommon;
        using namespace sdkInterfaces;
        using namespace avs;
        using namespace attachment;
        class TransportFactoryInterface {
        public:
            virtual shared_ptr<TransportInterface> createTransport(shared_ptr<AuthDelegateInterface> authDelegate, shared_ptr<AttachmentManager> attachmentManager,
                                                                   const string& avsGateway, shared_ptr<MessageConsumerInterface> messageConsumerInterface,
                                                                   shared_ptr<TransportObserverInterface> transportObserverInterface,
                                                                   shared_ptr<SynchronizedMessageRequestQueue> sharedMessageRequestQueue) = 0;
            virtual ~TransportFactoryInterface() = default;
        };
    }
}
#endif