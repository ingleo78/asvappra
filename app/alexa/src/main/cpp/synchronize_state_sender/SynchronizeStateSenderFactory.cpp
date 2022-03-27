#include <logger/Logger.h>
#include "PostConnectSynchronizeStateSender.h"
#include "SynchronizeStateSenderFactory.h"

namespace alexaClientSDK {
    namespace synchronizeStateSender {
        using namespace std;
        using namespace avsCommon;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace logger;
        static const string TAG("SynchronizeStateSenderFactory");
        #define LX(event) LogEntry(TAG, event)
        shared_ptr<SynchronizeStateSenderFactory> SynchronizeStateSenderFactory::create(shared_ptr<ContextManagerInterface> contextManager) {
            ACSDK_DEBUG5(LX(__func__));
            if (!contextManager) {
                ACSDK_ERROR(LX("createFailed").d("reason", "nullContextManager"));
            } else return shared_ptr<SynchronizeStateSenderFactory>(new SynchronizeStateSenderFactory(contextManager));
            return nullptr;
        }
        SynchronizeStateSenderFactory::SynchronizeStateSenderFactory(shared_ptr<ContextManagerInterface> contextManager) : m_contextManager{contextManager} {}
        std::shared_ptr<PostConnectOperationInterface> SynchronizeStateSenderFactory::createPostConnectOperation() {
            ACSDK_DEBUG5(LX(__func__));
            return PostConnectSynchronizeStateSender::create(m_contextManager);
        }
    }
}
