#ifndef ALEXA_CLIENT_SDK_AVSGATEWAYMANAGER_INCLUDE_AVSGATEWAYMANAGER_AVSGATEWAYMANAGER_H_
#define ALEXA_CLIENT_SDK_AVSGATEWAYMANAGER_INCLUDE_AVSGATEWAYMANAGER_AVSGATEWAYMANAGER_H_

#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <sdkinterfaces/AVSGatewayAssignerInterface.h>
#include <sdkinterfaces/AVSGatewayManagerInterface.h>
#include <sdkinterfaces/PostConnectOperationProviderInterface.h>
#include <configuration/ConfigurationNode.h>
#include <registration_manager/CustomerDataHandler.h>
#include "Storage/AVSGatewayManagerStorageInterface.h"

namespace alexaClientSDK {
    namespace avsGatewayManager {
        using namespace std;
        using namespace avsCommon;
        using namespace storage;
        using namespace sdkInterfaces;
        using namespace registrationManager;
        using namespace utils;
        using namespace configuration;
        class AVSGatewayManager : public AVSGatewayManagerInterface, public PostConnectOperationProviderInterface, public CustomerDataHandler {
        public:
            static shared_ptr<AVSGatewayManager> create(shared_ptr<AVSGatewayManagerStorageInterface> avsGatewayManagerStorage,
                                                        shared_ptr<CustomerDataManager> customerDataManager, const ConfigurationNode& configurationRoot);
            bool setAVSGatewayAssigner(shared_ptr<AVSGatewayAssignerInterface> avsGatewayAssigner) override;
            string getGatewayURL() const override;
            bool setGatewayURL(const string& avsGatewayURL) override;
            void addObserver(shared_ptr<AVSGatewayObserverInterface> observer) override;
            void removeObserver(shared_ptr<AVSGatewayObserverInterface> observer) override;
            shared_ptr<PostConnectOperationInterface> createPostConnectOperation() override;
            void clearData() override;
            void onGatewayVerified(const shared_ptr<PostConnectOperationInterface>& verifyGatewaySender);
            ~AVSGatewayManager();
        private:
            AVSGatewayManager(shared_ptr<AVSGatewayManagerStorageInterface> avsGatewayManagerStorage, shared_ptr<CustomerDataManager> customerDataManager,
                              const string& defaultGateway);
            bool init();
            bool saveStateLocked();
            shared_ptr<AVSGatewayManagerStorageInterface> m_avsGatewayStorage;
            shared_ptr<AVSGatewayAssignerInterface> m_avsGatewayAssigner;
            mutable mutex m_mutex;
            shared_ptr<PostConnectOperationInterface> m_currentVerifyGatewaySender;
            GatewayVerifyState m_currentState;
            unordered_set<shared_ptr<AVSGatewayObserverInterface>> m_observers;
        };
    }
}
#endif