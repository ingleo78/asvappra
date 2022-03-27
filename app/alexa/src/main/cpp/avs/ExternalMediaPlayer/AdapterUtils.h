#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_EXTERNALMEDIAPLAYER_ADAPTERUTILS_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_EXTERNALMEDIAPLAYER_ADAPTERUTILS_H_

#include "../../../../../../../../AndroidSDK/ndk/21.3.6528147/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/c++/v1/vector"
#include "../../../../../../../../AndroidSDK/ndk/21.3.6528147/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/c++/v1/set"
#include "../../json/document.h"
#include "../../json/en.h"
#include "../../json/stringbuffer.h"
#include "../../json/writer.h"
#include "../../sdkinterfaces/ExternalMediaAdapterInterface.h"
#include "../../util/RetryTimer.h"
#include "../NamespaceAndName.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            namespace externalMediaPlayer {
                using namespace std;
                using namespace avs;
                using namespace sdkInterfaces;
                using namespace sdkInterfaces::externalMediaPlayer;
                using namespace utils;
                using namespace rapidjson;
                enum class AdapterEvent {
                    CHANGE_REPORT,
                    REQUEST_TOKEN,
                    LOGIN,
                    LOGOUT,
                    PLAYER_EVENT,
                    PLAYER_ERROR_EVENT
                };
                extern const vector<int> SESSION_RETRY_TABLE;
                extern RetryTimer SESSION_RETRY_TIMER;
                extern const NamespaceAndName CHANGE_REPORT;
                extern const NamespaceAndName REQUEST_TOKEN;
                extern const NamespaceAndName LOGIN;
                extern const NamespaceAndName LOGOUT;
                extern const NamespaceAndName PLAYER_EVENT;
                extern const NamespaceAndName PLAYER_ERROR_EVENT;
                Value buildSupportedOperations(const set<SupportedPlaybackOperation>& supportedOperations, Document::AllocatorType& allocator);
                Value buildPlaybackState(const AdapterPlaybackState& playbackState, Document::AllocatorType& allocator);
                Value buildSessionState(const AdapterSessionState& sessionState, Document::AllocatorType& allocator);
                bool buildDefaultPlayerState(Value* document, Document::AllocatorType& allocator);
            }
        }
    }
}
#endif