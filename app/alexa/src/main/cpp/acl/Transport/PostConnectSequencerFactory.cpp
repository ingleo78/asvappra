#include <set>
#include <logger/Logger.h>
#include "PostConnectSequencer.h"
#include "PostConnectSequencerFactory.h"

namespace alexaClientSDK {
    namespace acl {
        using namespace std;
        using namespace avsCommon;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace logger;
        static const string TAG("PostConnectSequencerFactory");
        #define LX(event) LogEntry(TAG, event)
        shared_ptr<PostConnectSequencerFactory> PostConnectSequencerFactory::create(const vector<shared_ptr<PostConnectOperationProviderInterface>>& postConnectOperationProviders) {
            for (auto& provider : postConnectOperationProviders) {
                if (!provider) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "invalidProviderFound"));
                    return nullptr;
                }
            }
            return shared_ptr<PostConnectSequencerFactory>(new PostConnectSequencerFactory(postConnectOperationProviders));
        }
        PostConnectSequencerFactory::PostConnectSequencerFactory(const vector<shared_ptr<PostConnectOperationProviderInterface>>& postConnectOperationProviders) :
                                                                 m_postConnectOperationProviders{postConnectOperationProviders} {}
        shared_ptr<PostConnectInterface> PostConnectSequencerFactory::createPostConnect() {
            PostConnectSequencer::PostConnectOperationsSet postConnectOperationsSet;
            for (auto& provider : m_postConnectOperationProviders) {
                auto postConnectOperation = provider->createPostConnectOperation();
                if (postConnectOperation) postConnectOperationsSet.insert(postConnectOperation);
            }
            return PostConnectSequencer::create(postConnectOperationsSet);
        }
    }
}