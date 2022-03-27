#ifndef ALEXA_CLIENT_SDK_AVSGATEWAYMANAGER_INCLUDE_AVSGATEWAYMANAGER_STORAGE_AVSGATEWAYMANAGERSTORAGEINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSGATEWAYMANAGER_INCLUDE_AVSGATEWAYMANAGER_STORAGE_AVSGATEWAYMANAGERSTORAGEINTERFACE_H_

#include "../GatewayVerifyState.h"

namespace alexaClientSDK {
    namespace avsGatewayManager {
        namespace storage {
            class AVSGatewayManagerStorageInterface {
            public:
                virtual ~AVSGatewayManagerStorageInterface() = default;
                virtual bool init() = 0;
                virtual bool loadState(GatewayVerifyState* state) = 0;
                virtual bool saveState(const GatewayVerifyState& state) = 0;
                virtual void clear() = 0;
            };
        }
    }
}
#endif