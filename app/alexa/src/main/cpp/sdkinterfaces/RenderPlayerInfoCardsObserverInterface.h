#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_RENDERPLAYERINFOCARDSOBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_RENDERPLAYERINFOCARDSOBSERVERINTERFACE_H_

#include <chrono>
#include <memory>
#include <string>
#include <avs/PlayerActivity.h>
#include "MediaPropertiesInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class RenderPlayerInfoCardsObserverInterface {
            public:
                virtual ~RenderPlayerInfoCardsObserverInterface() = default;
                struct Context {
                    std::string audioItemId;
                    std::chrono::milliseconds offset;
                    std::shared_ptr<MediaPropertiesInterface> mediaProperties;
                };
                virtual void onRenderPlayerCardsInfoChanged(avsCommon::avs::PlayerActivity state, const Context& context);
            };
        }
    }
}
#endif