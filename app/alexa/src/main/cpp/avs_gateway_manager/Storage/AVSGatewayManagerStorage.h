#ifndef ALEXA_CLIENT_SDK_AVSGATEWAYMANAGER_INCLUDE_AVSGATEWAYMANAGER_STORAGE_AVSGATEWAYMANAGERSTORAGE_H_
#define ALEXA_CLIENT_SDK_AVSGATEWAYMANAGER_INCLUDE_AVSGATEWAYMANAGER_STORAGE_AVSGATEWAYMANAGERSTORAGE_H_

#include <memory>
#include <sdkinterfaces/Storage/MiscStorageInterface.h>
#include <configuration/ConfigurationNode.h>
#include "AVSGatewayManagerStorageInterface.h"

namespace alexaClientSDK {
    namespace avsGatewayManager {
        namespace storage {
            class AVSGatewayManagerStorage : public AVSGatewayManagerStorageInterface {
            public:
                static std::unique_ptr<AVSGatewayManagerStorage> create(std::shared_ptr<avsCommon::sdkInterfaces::storage::MiscStorageInterface> miscStorage);
                bool init() override;
                bool loadState(GatewayVerifyState* state) override;
                bool saveState(const GatewayVerifyState& state) override;
                void clear() override;
            private:
                AVSGatewayManagerStorage(std::shared_ptr<avsCommon::sdkInterfaces::storage::MiscStorageInterface> miscStorage);
                std::shared_ptr<avsCommon::sdkInterfaces::storage::MiscStorageInterface> m_miscStorage;
            };
        }
    }
}
#endif