#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_PLAYERACTIVITY_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_PLAYERACTIVITY_H_

#include <istream>
#include <ostream>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            enum class PlayerActivity {
                IDLE,
                PLAYING,
                STOPPED,
                PAUSED,
                BUFFER_UNDERRUN,
                FINISHED
            };
            inline std::string playerActivityToString(PlayerActivity playerActivity) {
                switch(playerActivity) {
                    case PlayerActivity::IDLE: return "IDLE";
                    case PlayerActivity::PLAYING: return "PLAYING";
                    case PlayerActivity::STOPPED: return "STOPPED";
                    case PlayerActivity::PAUSED: return "PAUSED";
                    case PlayerActivity::BUFFER_UNDERRUN: return "BUFFER_UNDERRUN";
                    case PlayerActivity::FINISHED: return "FINISHED";
                }
                return "unknown PlayerActivity";
            }
            inline std::ostream& operator<<(std::ostream& stream, const PlayerActivity& playerActivity) {
                return stream << playerActivityToString(playerActivity);
            }
            inline std::istream& operator>>(std::istream& is, PlayerActivity& value) {
                std::string str;
                is >> str;
                if ("IDLE" == str) value = PlayerActivity::IDLE;
                else if ("PLAYING" == str) value = PlayerActivity::PLAYING;
                else if ("STOPPED" == str) value = PlayerActivity::STOPPED;
                else if ("PAUSED" == str) value = PlayerActivity::PAUSED;
                else if ("BUFFER_UNDERRUN" == str) value = PlayerActivity::BUFFER_UNDERRUN;
                else if ("FINISHED" == str) value = PlayerActivity::FINISHED;
                else is.setstate(std::ios_base::failbit);
                return is;
            }
        }
    }
}
#endif