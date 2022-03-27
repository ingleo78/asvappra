#include "TextStyle.h"

namespace alexaClientSDK {
    namespace captions {
        Style::Style(bool bold, bool italic, bool underline) : m_bold{bold}, m_italic{italic}, m_underline{underline} {}
        bool Style::operator==(const Style& rhs) const {
            return (m_bold == rhs.m_bold) && (m_italic == rhs.m_italic) && (m_underline == rhs.m_underline);
        }
        bool Style::operator!=(const Style& rhs) const {
            return !(rhs == *this);
        }
        std::ostream& operator<<(std::ostream& stream, const Style& style) {
            return stream << "Style(bold=" << style.m_bold << ", underline=" << style.m_underline << ", italic=" << style.m_italic << ")";
        }
        TextStyle::TextStyle(size_t index, const Style& style) : charIndex{index}, activeStyle{style} {}
        bool TextStyle::operator==(const TextStyle& rhs) const {
            return (charIndex == rhs.charIndex) && (activeStyle == rhs.activeStyle);
        }
        bool TextStyle::operator!=(const TextStyle& rhs) const {
            return !(rhs == *this);
        }
        std::ostream& operator<<(std::ostream& stream, const TextStyle& textStyle) {
            return stream << "TextStyle(charIndex:" << textStyle.charIndex << ", activeStyle:" << textStyle.activeStyle << ")";
        }
    }
}