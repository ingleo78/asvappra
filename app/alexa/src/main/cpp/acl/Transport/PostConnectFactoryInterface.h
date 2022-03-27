#ifndef ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_POSTCONNECTFACTORYINTERFACE_H_
#define ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_POSTCONNECTFACTORYINTERFACE_H_

#include <memory>
#include "PostConnectInterface.h"

namespace alexaClientSDK {
    namespace acl {
        class PostConnectFactoryInterface {
        public:
            virtual ~PostConnectFactoryInterface() = default;
            virtual std::shared_ptr<PostConnectInterface> createPostConnect() = 0;
        };
    }
}
#endif