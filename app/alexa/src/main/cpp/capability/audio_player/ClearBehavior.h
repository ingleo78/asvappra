#ifndef ACSDKAUDIOPLAYER_CLEARBEHAVIOR_H_
#define ACSDKAUDIOPLAYER_CLEARBEHAVIOR_H_

#include <ostream>
#include <sdkinterfaces/DialogUXStateObserverInterface.h>
#include <json/JSONUtils.h>

namespace alexaClientSDK {
    namespace acsdkAudioPlayer {
        enum class ClearBehavior {
            CLEAR_ENQUEUED,
            CLEAR_ALL
        };
        inline std::string clearBehaviorToString(ClearBehavior clearBehavior) {
            switch(clearBehavior) {
                case ClearBehavior::CLEAR_ENQUEUED: return "CLEAR_ENQUEUED";
                case ClearBehavior::CLEAR_ALL: return "CLEAR_ALL";
            }
            return "unknown ClearBehavior";
        }
        inline bool stringToClearBehavior(const std::string& text, ClearBehavior* clearBehavior) {
            if (nullptr == clearBehavior) return false;
            else if (clearBehaviorToString(ClearBehavior::CLEAR_ENQUEUED) == text) {
                *clearBehavior = ClearBehavior::CLEAR_ENQUEUED;
                return true;
            } else if (clearBehaviorToString(ClearBehavior::CLEAR_ALL) == text) {
                *clearBehavior = ClearBehavior::CLEAR_ALL;
                return true;
            }
            return false;
        }
        inline std::ostream& operator<<(std::ostream& stream, const ClearBehavior& clearBehavior) {
            return stream << clearBehaviorToString(clearBehavior);
        }
        inline bool convertToValue(const rapidjson::Value& documentNode, ClearBehavior* clearBehavior) {
            std::string text;
            if (!avsCommon::utils::json::jsonUtils::convertToValue(documentNode, &text)) return false;
            return stringToClearBehavior(text, clearBehavior);
        }
    }
}
#endif