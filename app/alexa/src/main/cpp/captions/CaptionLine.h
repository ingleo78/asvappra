#ifndef ALEXA_CLIENT_SDK_CAPTIONS_INTERFACE_INCLUDE_CAPTIONS_CAPTIONLINE_H_
#define ALEXA_CLIENT_SDK_CAPTIONS_INTERFACE_INCLUDE_CAPTIONS_CAPTIONLINE_H_

#include <string>
#include <vector>
#include "TextStyle.h"

namespace alexaClientSDK {
    namespace captions {
        using namespace std;
        struct CaptionLine {
            CaptionLine(const string& text = "", const vector<TextStyle>& styles = {});
            vector<CaptionLine> splitAtTextIndex(size_t index);
            static CaptionLine merge(vector<CaptionLine> captionLines);
            bool operator==(const CaptionLine& rhs) const;
            bool operator!=(const CaptionLine& rhs) const;
            string text;
            vector<TextStyle> styles;
        };
        std::ostream& operator<<(ostream& stream, const CaptionLine& line);
    }
}
#endif