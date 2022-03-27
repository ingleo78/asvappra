#ifndef ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_POSTCONNECTSEQUENCER_H_
#define ALEXA_CLIENT_SDK_ACL_INCLUDE_ACL_TRANSPORT_POSTCONNECTSEQUENCER_H_

#include <condition_variable>
#include <memory>
#include <mutex>
#include <set>
#include <thread>
#include <sdkinterfaces/PostConnectOperationInterface.h>
#include <sdkinterfaces/MessageRequestObserverInterface.h>
#include <util/RequiresShutdown.h>
#include "PostConnectInterface.h"
#include "TransportObserverInterface.h"
#include "HTTP2Transport.h"

namespace alexaClientSDK {
    namespace acl {
        using namespace std;
        using namespace avsCommon;
        using namespace sdkInterfaces;
        struct PostConnectOperationPriorityCompare {
            bool operator()(
                const shared_ptr<PostConnectOperationInterface>& first,
                const shared_ptr<PostConnectOperationInterface>& second) const {
                return first->getOperationPriority() < second->getOperationPriority();
            }
        };
        class PostConnectSequencer : public PostConnectInterface {
        public:
            using PostConnectOperationsSet = set<shared_ptr<PostConnectOperationInterface>, PostConnectOperationPriorityCompare>;
            static shared_ptr<PostConnectSequencer> create(const PostConnectOperationsSet& postConnectOperations);
            ~PostConnectSequencer() override;
            bool doPostConnect(shared_ptr<MessageSenderInterface> postConnectSender, shared_ptr<PostConnectObserverInterface> postConnectObserver) override;
            void onDisconnect() override;
        private:
            PostConnectSequencer(const PostConnectOperationsSet& postConnectOperations);
            void mainLoop(shared_ptr<MessageSenderInterface> postConnectSender, shared_ptr<PostConnectObserverInterface> postConnectObserver);
            void stop();
            void resetCurrentOperation();
            bool isStopping();
            mutex m_mutex;
            shared_ptr<PostConnectOperationInterface> m_currentPostConnectOperation;
            bool m_isStopping;
            PostConnectOperationsSet m_postConnectOperations;
            mutex m_mainLoopThreadMutex;
            thread m_mainLoopThread;
        };
    }
}
#endif