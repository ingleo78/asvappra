#include <algorithm>
#include <limits>
#include <utility>
#include <logger/LoggerUtils.h>
#include <util/string/StringUtils.h>
#include "CaptionLine.h"

namespace alexaClientSDK {
    namespace captions {
        using namespace avsCommon;
        using namespace utils::logger;
        static const string TAG("CaptionLine");
        #define LX(event) LogEntry(TAG, event)
        CaptionLine::CaptionLine(const string& text, const vector<TextStyle>& styles) : text{text}, styles{styles} {}
        CaptionLine CaptionLine::merge(vector<CaptionLine> captionLines) {
            CaptionLine result;
            if (captionLines.empty()) return result;
            size_t indexOffset = 0;
            if (captionLines.front().styles.empty() || captionLines.front().styles.front().charIndex != 0) {
                result.styles.emplace_back(TextStyle());
            }
            for (const auto& line : captionLines) {
                result.text.append(line.text);
                for (const auto& style : line.styles) {
                    TextStyle tempStyle = style;
                    tempStyle.charIndex += indexOffset;
                    result.styles.emplace_back(tempStyle);
                }
                indexOffset = line.text.length();
            }
            return result;
        }
        vector<CaptionLine> CaptionLine::splitAtTextIndex(size_t index) {
            vector<CaptionLine> result;
            if (index > text.length()) {
                result.emplace_back(CaptionLine(text, styles));
                return result;
            }
            CaptionLine lineOne, lineTwo;
            lineOne.text = text.substr(0, index);
            lineTwo.text = text.substr(index);
            size_t originalLineTwoTextLength = lineTwo.text.length();
            lineTwo.text = utils::string::ltrim(lineTwo.text);
            size_t whitespaceCount = originalLineTwoTextLength - lineTwo.text.length();
            if (styles.empty()) {
                result.emplace_back(CaptionLine(lineOne.text, lineOne.styles));
                result.emplace_back(CaptionLine(lineTwo.text, lineTwo.styles));
                return result;
            }
            sort(styles.begin(), styles.end(), [](const TextStyle& l, const TextStyle& r) { return l.charIndex < r.charIndex; });
            size_t appliedStyleIndex = 0;
            for (auto& style : styles) {
                if (style.charIndex < index) appliedStyleIndex++;
            }
            lineOne.styles = {styles.begin(), styles.begin() + appliedStyleIndex};
            lineTwo.styles = {styles.begin() + appliedStyleIndex, styles.end()};
            size_t indexOffset = 0;
            if (index > std::numeric_limits<size_t>::max() - whitespaceCount) {
                ACSDK_WARN(LX("lineSplitFailure").d("reason", "characterAdjustmentAmountWouldOverflow").d("index", index)
                    .d("whitespaceCount", whitespaceCount));
            } else indexOffset = index + whitespaceCount;
            for (auto& lineTwoStyle : lineTwo.styles) {
                if (indexOffset > lineTwoStyle.charIndex) lineTwoStyle.charIndex = 0;
                else lineTwoStyle.charIndex -= indexOffset;
            }
            if (lineTwo.styles.empty() || lineTwo.styles[0].charIndex != 0) {
                lineTwo.styles.insert(lineTwo.styles.begin(), lineOne.styles.back());
                if (!lineTwo.styles.empty()) lineTwo.styles[0].charIndex = 0;
            }
            result.emplace_back(CaptionLine(lineOne.text, lineOne.styles));
            result.emplace_back(CaptionLine(lineTwo.text, lineTwo.styles));
            return result;
        }
        bool CaptionLine::operator==(const CaptionLine& rhs) const {
            return (text == rhs.text) && (styles == rhs.styles);
        }
        bool CaptionLine::operator!=(const CaptionLine& rhs) const {
            return !(rhs == *this);
        }
        ostream& operator<<(ostream& stream, const CaptionLine& line) {
            stream << "CaptionLine(text:\"" << line.text << "\", styles:[";
            for (auto iter = line.styles.begin(); iter != line.styles.end(); iter++) {
                if (iter != line.styles.begin()) stream << ", ";
                stream << *iter;
            }
            stream << "])";
            return stream;
        }
    }
}