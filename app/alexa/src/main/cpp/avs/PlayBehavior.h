#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_PLAYBEHAVIOR_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_AVS_INCLUDE_AVSCOMMON_AVS_PLAYBEHAVIOR_H_

#include <string>
#include <ostream>
#include <json/JSONUtils.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            enum class PlayBehavior {
                REPLACE_ALL,
                ENQUEUE,
                REPLACE_ENQUEUED
            };
            inline std::string playBehaviorToString(PlayBehavior playBehavior) {
                switch(playBehavior) {
                    case PlayBehavior::REPLACE_ALL: return "REPLACE_ALL";
                    case PlayBehavior::ENQUEUE: return "ENQUEUE";
                    case PlayBehavior::REPLACE_ENQUEUED: return "REPLACE_ENQUEUED";
                }
                return "unknown PlayBehavior";
            }
            inline bool stringToPlayBehavior(const std::string text, PlayBehavior* playBehavior) {
                if (nullptr == playBehavior) return false;
                else if (playBehaviorToString(PlayBehavior::REPLACE_ALL) == text) {
                    *playBehavior = PlayBehavior::REPLACE_ALL;
                    return true;
                } else if (playBehaviorToString(PlayBehavior::ENQUEUE) == text) {
                    *playBehavior = PlayBehavior::ENQUEUE;
                    return true;
                } else if (playBehaviorToString(PlayBehavior::REPLACE_ENQUEUED) == text) {
                    *playBehavior = PlayBehavior::REPLACE_ENQUEUED;
                    return true;
                }
                return false;
            }
            inline std::ostream& operator<<(std::ostream& stream, const PlayBehavior& playBehavior) {
                return stream << playBehaviorToString(playBehavior);
            }
            inline bool convertToValue(const rapidjson::Value& documentNode, PlayBehavior* playBehavior) {
                std::string text;
                if (!avsCommon::utils::json::jsonUtils::convertToValue(documentNode, &text)) return false;
                return stringToPlayBehavior(text, playBehavior);
            }
        }
    }
}
#endif