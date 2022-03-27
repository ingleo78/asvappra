#ifndef ACSDKAUDIOPLAYER_STREAMFORMAT_H_
#define ACSDKAUDIOPLAYER_STREAMFORMAT_H_

#include <ostream>
#include <sdkinterfaces/DialogUXStateObserverInterface.h>
#include <json/JSONUtils.h>

namespace alexaClientSDK {
    namespace acsdkAudioPlayer {
        enum class StreamFormat {
            AUDIO_MPEG,
            UNKNOWN
        };
        inline std::string streamFormatToString(StreamFormat streamFormat) {
            switch(streamFormat) {
                case StreamFormat::AUDIO_MPEG: return "AUDIO_MPEG";
                case StreamFormat::UNKNOWN: break;
            }
            return "unknown StreamFormat";
        }
        inline bool stringToStreamFormat(const std::string& text, StreamFormat* streamFormat) {
            if (nullptr == streamFormat) return false;
            else if (text == streamFormatToString(StreamFormat::AUDIO_MPEG)) {
                *streamFormat = StreamFormat::AUDIO_MPEG;
                return true;
            }
            return false;
        }
        inline std::ostream& operator<<(std::ostream& stream, const StreamFormat& streamFormat) {
            return stream << streamFormatToString(streamFormat);
        }
        inline bool convertToValue(const rapidjson::Value& documentNode, StreamFormat* streamFormat) {
            std::string text;
            if (!avsCommon::utils::json::jsonUtils::convertToValue(documentNode, &text)) return false;
            return stringToStreamFormat(text, streamFormat);
        }
    }
}
#endif