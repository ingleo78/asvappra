#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_EXTERNALMEDIAPLAYERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_EXTERNALMEDIAPLAYERINTERFACE_H_

#include "ContextManagerInterface.h"
#include "FocusManagerInterface.h"
#include "MessageSenderInterface.h"
#include "PlaybackHandlerInterface.h"
#include "PlaybackRouterInterface.h"
#include "SpeakerInterface.h"
#include "SpeakerManagerInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class ExternalMediaPlayerInterface {
            public:
                virtual ~ExternalMediaPlayerInterface() = default;
                virtual void setPlayerInFocus(const std::string& playerInFocus);
            };
        }
    }
}
#endif