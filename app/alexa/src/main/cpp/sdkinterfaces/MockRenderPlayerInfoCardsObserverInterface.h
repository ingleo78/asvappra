#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKRENDERPLAYERINFOCARDSOBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKRENDERPLAYERINFOCARDSOBSERVERINTERFACE_H_

#include <gmock/gmock.h>
#include "RenderPlayerInfoCardsObserverInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace test {
                class MockRenderPlayerInfoCardsObserver : public RenderPlayerInfoCardsObserverInterface {
                public:
                    //MOCK_METHOD2(onRenderPlayerCardsInfoChanged, void(avsCommon::avs::PlayerActivity state, const Context& context));
                };
            }
        }
    }
}
#endif