#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_PLAYBACKBUTTONS_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_PLAYBACKBUTTONS_H_

#include <iostream>
#include <string>
#include <functional>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            enum class PlaybackButton {
                PLAY,
                PAUSE,
                NEXT,
                PREVIOUS,
                SKIP_FORWARD,
                SKIP_BACKWARD
            };
            enum class PlaybackToggle {
                SHUFFLE,
                LOOP,
                REPEAT,
                THUMBS_UP,
                THUMBS_DOWN
            };
            inline std::string PlaybackButtonToString(PlaybackButton button) {
                switch(button) {
                    case PlaybackButton::PLAY: return "Play";
                    case PlaybackButton::PAUSE: return "Pause";
                    case PlaybackButton::PREVIOUS: return "Previous";
                    case PlaybackButton::NEXT: return "Next";
                    case PlaybackButton::SKIP_FORWARD: return "Skip_Forward";
                    case PlaybackButton::SKIP_BACKWARD: return "Skip_Backward";
                }
                return "unknown playbackButton";
            }
            inline std::string PlaybackToggleToString(PlaybackToggle toggle) {
                switch(toggle) {
                    case PlaybackToggle::SHUFFLE: return "Shuffle";
                    case PlaybackToggle::LOOP: return "Loop";
                    case PlaybackToggle::REPEAT: return "Repeat";
                    case PlaybackToggle::THUMBS_UP: return "Thumbs_Up";
                    case PlaybackToggle::THUMBS_DOWN: return "Thumbs_Down";
                }
                return "unknown playbackToggle";
            }
            #define ACSDK_PLAYBACK_BUTTON_TO_STRING(button) \
                case PlaybackButton::button: return stream << #button;
            #define ACSDK_PLAYBACK_TOGGLE_TO_STRING(toggle) \
                case PlaybackToggle::toggle:  return stream << #toggle;
            inline std::ostream& operator<<(std::ostream& stream, const PlaybackButton& button) {
                switch(button) {
                    ACSDK_PLAYBACK_BUTTON_TO_STRING(PLAY);
                    ACSDK_PLAYBACK_BUTTON_TO_STRING(PAUSE);
                    ACSDK_PLAYBACK_BUTTON_TO_STRING(PREVIOUS);
                    ACSDK_PLAYBACK_BUTTON_TO_STRING(NEXT);
                    ACSDK_PLAYBACK_BUTTON_TO_STRING(SKIP_FORWARD);
                    ACSDK_PLAYBACK_BUTTON_TO_STRING(SKIP_BACKWARD);
                }
                return stream;
            }
            #undef ACSDK_PLAYBACK_BUTTON_TO_STRING
            inline std::ostream& operator<<(std::ostream& stream, const PlaybackToggle& toggle) {
                switch(toggle) {
                    ACSDK_PLAYBACK_TOGGLE_TO_STRING(SHUFFLE);
                    ACSDK_PLAYBACK_TOGGLE_TO_STRING(LOOP);
                    ACSDK_PLAYBACK_TOGGLE_TO_STRING(REPEAT);
                    ACSDK_PLAYBACK_TOGGLE_TO_STRING(THUMBS_UP);
                    ACSDK_PLAYBACK_TOGGLE_TO_STRING(THUMBS_DOWN);
                }
                return stream;
            }
            #undef ACSDK_PLAYBACK_TOGGLE_TO_STRING
        }
    }
}
namespace std {
    typedef unsigned int size_t;
    using namespace alexaClientSDK::avsCommon::avs;
    template<> struct hash<std::string> {
        inline size_t operator()(std::string data) const {
            return std::hash<std::string>{}(PlaybackButtonToString(button));
        }
        inline void setData(PlaybackButton& button) { this->button = button;}
        PlaybackButton button;
    };
    template <> struct hash<alexaClientSDK::avsCommon::avs::PlaybackToggle> {
        inline size_t operator()(const alexaClientSDK::avsCommon::avs::PlaybackToggle& toggle) const {
            return std::hash<std::string_view>{}(PlaybackToggleToString(toggle));
        }
    };
}
#endif