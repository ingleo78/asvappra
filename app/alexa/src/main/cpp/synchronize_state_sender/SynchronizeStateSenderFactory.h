#ifndef ALEXA_CLIENT_SDK_SYNCHRONIZESTATESENDER_INCLUDE_SYNCHRONIZESTATESENDER_SYNCHRONIZESTATESENDERFACTORY_H_
#define ALEXA_CLIENT_SDK_SYNCHRONIZESTATESENDER_INCLUDE_SYNCHRONIZESTATESENDER_SYNCHRONIZESTATESENDERFACTORY_H_

#include <memory>
#include <sdkinterfaces/ContextManagerInterface.h>
#include <sdkinterfaces/PostConnectOperationProviderInterface.h>

namespace alexaClientSDK {
    namespace synchronizeStateSender {
        using namespace std;
        using namespace avsCommon;
        using namespace avs;
        using namespace sdkInterfaces;
        class SynchronizeStateSenderFactory : public PostConnectOperationProviderInterface {
        public:
            static shared_ptr<SynchronizeStateSenderFactory> create(shared_ptr<ContextManagerInterface> contextManager);
            shared_ptr<PostConnectOperationInterface> createPostConnectOperation() override;
        private:
            SynchronizeStateSenderFactory(shared_ptr<ContextManagerInterface> contextManager);
            shared_ptr<ContextManagerInterface> m_contextManager;
        };
    }
}
#endif
