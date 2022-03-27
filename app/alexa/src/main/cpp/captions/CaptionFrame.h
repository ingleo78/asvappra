#ifndef ALEXA_CLIENT_SDK_CAPTIONS_INTERFACE_INCLUDE_CAPTIONS_CAPTIONFRAME_H_
#define ALEXA_CLIENT_SDK_CAPTIONS_INTERFACE_INCLUDE_CAPTIONS_CAPTIONFRAME_H_

#include <chrono>
#include <iostream>
#include <vector>
#include <media_player/MediaPlayerObserverInterface.h>
#include "CaptionLine.h"

namespace alexaClientSDK {
    namespace captions {
        using namespace std;
        using namespace chrono;
        using namespace avsCommon;
        using namespace utils;
        using namespace mediaPlayer;
        class CaptionFrame {
        public:
            using MediaPlayerSourceId = MediaPlayerInterface::SourceId;
            static int getLineWrapLimit();
            CaptionFrame(MediaPlayerSourceId sourceId = 0, milliseconds duration = milliseconds(0), milliseconds delay = milliseconds(0),
                         const vector<CaptionLine>& captionLines = {});
            milliseconds getDuration() const;
            milliseconds getDelay() const;
            MediaPlayerSourceId getSourceId() const;
            vector<CaptionLine> getCaptionLines() const;
            bool operator==(const CaptionFrame& rhs) const;
            bool operator!=(const CaptionFrame& rhs) const;
        private:
            MediaPlayerSourceId m_id;
            milliseconds m_duration;
            milliseconds m_delay;
            vector<CaptionLine> m_captionLines;
        };
        ostream& operator<<(ostream& stream, const CaptionFrame& frame);
    }
}
#endif