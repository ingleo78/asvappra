#ifndef ALEXA_CLIENT_SDK_CAPTIONS_INTERFACE_INCLUDE_CAPTIONS_TEXTSTYLE_H_
#define ALEXA_CLIENT_SDK_CAPTIONS_INTERFACE_INCLUDE_CAPTIONS_TEXTSTYLE_H_

#include <cstdint>
#include <iostream>

namespace alexaClientSDK {
    namespace captions {
        struct Style {
            Style(bool bold = false, bool italic = false, bool underline = false);
            bool operator==(const Style& rhs) const;
            bool operator!=(const Style& rhs) const;
            bool m_bold;
            bool m_italic;
            bool m_underline;
        };
        std::ostream& operator<<(std::ostream& stream, const Style& style);
        struct TextStyle {
            TextStyle(size_t index = 0, const Style& style = Style());
            size_t charIndex;
            Style activeStyle;
            bool operator==(const TextStyle& rhs) const;
            bool operator!=(const TextStyle& rhs) const;
        };
        std::ostream& operator<<(std::ostream& stream, const TextStyle& textStyle);
    }
}
#endif