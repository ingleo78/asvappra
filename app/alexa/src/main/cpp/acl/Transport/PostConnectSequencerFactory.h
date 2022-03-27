#ifndef ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_POSTCONNECTSEQUENCERFACTORY_H_
#define ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_POSTCONNECTSEQUENCERFACTORY_H_

#include <memory>
#include <vector>
#include <sdkinterfaces/PostConnectOperationProviderInterface.h>
#include "PostConnectFactoryInterface.h"

namespace alexaClientSDK {
    namespace acl {
        using namespace std;
        class PostConnectSequencerFactory : public PostConnectFactoryInterface {
        public:
            using PostConnectOperationProviderInterface = avsCommon::sdkInterfaces::PostConnectOperationProviderInterface;
            static shared_ptr<PostConnectSequencerFactory> create(const vector<shared_ptr<PostConnectOperationProviderInterface>>& postConnectOperationProviders);
            shared_ptr<PostConnectInterface> createPostConnect() override;
        private:
            PostConnectSequencerFactory(const vector<std::shared_ptr<PostConnectOperationProviderInterface>>& postConnectOperationProviders);
            vector<shared_ptr<PostConnectOperationProviderInterface>> m_postConnectOperationProviders;
        };
    }
}
#endif