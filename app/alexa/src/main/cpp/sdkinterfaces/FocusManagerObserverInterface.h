#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_FOCUSMANAGEROBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_FOCUSMANAGEROBSERVERINTERFACE_H_

#include <string>
#include <avs/FocusState.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class FocusManagerObserverInterface {
            public:
                virtual ~FocusManagerObserverInterface() = default;
                virtual void onFocusChanged(const std::string& channelName, avsCommon::avs::FocusState newFocus) = 0;
            };
        }
    }
}
#endif