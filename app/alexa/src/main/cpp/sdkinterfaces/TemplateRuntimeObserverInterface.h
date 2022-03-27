#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_TEMPLATERUNTIMEOBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_TEMPLATERUNTIMEOBSERVERINTERFACE_H_

#include <chrono>
#include <string>
#include <avs/FocusState.h>
#include <avs/PlayerActivity.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            using namespace std;
            using namespace chrono;
            using namespace avs;
            class TemplateRuntimeObserverInterface {
            public:
                struct AudioPlayerInfo {
                    AudioPlayerInfo() : audioPlayerState{PlayerActivity::IDLE}, offset{milliseconds::zero()} {};
                    PlayerActivity audioPlayerState;
                    milliseconds offset;
                };
                virtual ~TemplateRuntimeObserverInterface() = default;
                virtual void renderTemplateCard(const string& jsonPayload, FocusState focusState);
                virtual void clearTemplateCard();
                virtual void renderPlayerInfoCard(const string& jsonPayload, AudioPlayerInfo audioPlayerInfo, FocusState focusState);
                virtual void clearPlayerInfoCard();
            };
        }
    }
}
#endif