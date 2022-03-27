#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_CHANNELOBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_CHANNELOBSERVERINTERFACE_H_

#include <avs/FocusState.h>
#include <avs/MixingBehavior.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class ChannelObserverInterface {
            public:
                virtual ~ChannelObserverInterface() = default;
                virtual void onFocusChanged(avs::FocusState newFocus, avs::MixingBehavior behavior);
            };
            inline void ChannelObserverInterface::onFocusChanged(avs::FocusState newFocus, avs::MixingBehavior behavior) {
                std::string focus = focusStateToString(newFocus);
            }
        }
    }
}
#endif