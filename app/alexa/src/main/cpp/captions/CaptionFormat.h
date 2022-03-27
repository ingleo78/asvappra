#ifndef ALEXA_CLIENT_SDK_CAPTIONS_INTERFACE_INCLUDE_CAPTIONS_CAPTIONFORMAT_H_
#define ALEXA_CLIENT_SDK_CAPTIONS_INTERFACE_INCLUDE_CAPTIONS_CAPTIONFORMAT_H_

#include <ostream>
#include <logger/LoggerUtils.h>

namespace alexaClientSDK {
    namespace captions {
        using namespace std;
        using namespace avsCommon::utils::logger;
        enum class CaptionFormat {
            WEBVTT,
            UNKNOWN
        };
        inline CaptionFormat avsStringToCaptionFormat(const string& text) {
            if (text == "WEBVTT") return CaptionFormat::WEBVTT;
            acsdkWarn(LogEntry(__func__, "unknownCaptionFormat").d("formatValue", text));
            return CaptionFormat::UNKNOWN;
        }
        inline ostream& operator<<(ostream& stream, const CaptionFormat& format) {
            switch (format) {
                case CaptionFormat::WEBVTT: stream << "WEBVTT"; break;
                case CaptionFormat::UNKNOWN: stream << "UNKNOWN"; break;
            }
            return stream;
        }
    }
}
#endif