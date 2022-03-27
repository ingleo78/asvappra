#ifndef ALEXA_CLIENT_SDK_SYNCHRONIZESTATESENDER_INCLUDE_SYNCHRONIZESTATESENDER_POSTCONNECTSYNCHRONIZESTATESENDER_H_
#define ALEXA_CLIENT_SDK_SYNCHRONIZESTATESENDER_INCLUDE_SYNCHRONIZESTATESENDER_POSTCONNECTSYNCHRONIZESTATESENDER_H_

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <avs/WaitableMessageRequest.h>
#include <sdkinterfaces/ContextManagerInterface.h>
#include <sdkinterfaces/ContextRequesterInterface.h>
#include <sdkinterfaces/PostConnectOperationInterface.h>

namespace alexaClientSDK {
    namespace synchronizeStateSender {
        using namespace std;
        using namespace avsCommon;
        using namespace avs;
        using namespace sdkInterfaces;
        class PostConnectSynchronizeStateSender : public ContextRequesterInterface, public PostConnectOperationInterface,
                                                  public enable_shared_from_this<PostConnectSynchronizeStateSender> {
        public:
            static shared_ptr<PostConnectSynchronizeStateSender> create(shared_ptr<ContextManagerInterface> contextManager);
            void onContextAvailable(const string& jsonContext) override;
            void onContextFailure(const ContextRequestError error) override;
            unsigned int getOperationPriority() override;
            bool performOperation(const shared_ptr<MessageSenderInterface>& messageSender) override;
            void abortOperation() override;
        private:
            PostConnectSynchronizeStateSender(shared_ptr<ContextManagerInterface> contextManager);
            bool fetchContext();
            bool isStopping();
            shared_ptr<ContextManagerInterface> m_contextManager;
            bool m_isStopping;
            string m_contextString;
            mutex m_mutex;
            shared_ptr<WaitableMessageRequest> m_postConnectRequest;
            condition_variable m_wakeTrigger;
        };
    }
}

#endif  // ALEXA_CLIENT_SDK_SYNCHRONIZESTATESENDER_INCLUDE_SYNCHRONIZESTATESENDER_POSTCONNECTSYNCHRONIZESTATESENDER_H_
