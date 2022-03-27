#include "CaptionFrame.h"

namespace alexaClientSDK {
    namespace captions {
        static const int LINE_WRAP_LIMIT = 200;
        CaptionFrame::CaptionFrame(MediaPlayerSourceId id, milliseconds duration, milliseconds delay, const vector<CaptionLine>& captionLines) :
                                   m_id{id}, m_duration{duration}, m_delay{delay}, m_captionLines{captionLines} {}
        CaptionFrame::MediaPlayerSourceId CaptionFrame::getSourceId() const {
            return m_id;
        }
        milliseconds CaptionFrame::getDuration() const {
            return m_duration;
        }
        milliseconds CaptionFrame::getDelay() const {
            return m_delay;
        }
        vector<CaptionLine> CaptionFrame::getCaptionLines() const {
            return m_captionLines;
        }
        int CaptionFrame::getLineWrapLimit() {
            return LINE_WRAP_LIMIT;
        }
        bool CaptionFrame::operator==(const CaptionFrame& rhs) const {
            return (m_id == rhs.getSourceId()) && (m_duration == rhs.getDuration()) && (m_delay == rhs.getDelay()) &&
                   (m_captionLines == rhs.getCaptionLines());
        }
        bool CaptionFrame::operator!=(const CaptionFrame& rhs) const {
            return !(rhs == *this);
        }
        ostream& operator<<(ostream& stream, const CaptionFrame& frame) {
            stream << "CaptionFrame(id:" << frame.getSourceId() << ", duration:" << frame.getDuration().count()
                   << ", delay:" << frame.getDelay().count() << ", lines:[";
            const auto captionLinesCopy = frame.getCaptionLines();
            for (auto iter = captionLinesCopy.begin(); iter != captionLinesCopy.end(); iter++) {
                if (iter != captionLinesCopy.begin()) stream << ", ";
                stream << *iter;
            }
            stream << "])";
            return stream;
        }
    }
}