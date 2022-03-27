#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_RENDERPLAYERINFOCARDSPROVIDERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_RENDERPLAYERINFOCARDSPROVIDERINTERFACE_H_

#include <memory>
#include "RenderPlayerInfoCardsObserverInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class RenderPlayerInfoCardsProviderInterface {
            public:
                virtual ~RenderPlayerInfoCardsProviderInterface() = default;
                virtual void setObserver(std::shared_ptr<avsCommon::sdkInterfaces::RenderPlayerInfoCardsObserverInterface> observer);
            };
        }
    }
}
#endif