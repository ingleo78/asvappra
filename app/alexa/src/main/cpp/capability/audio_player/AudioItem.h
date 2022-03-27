#ifndef ACSDKAUDIOPLAYER_AUDIOITEM_H_
#define ACSDKAUDIOPLAYER_AUDIOITEM_H_

#include <chrono>
#include <memory>
#include <string>
#include <avs/attachment/AttachmentReader.h>
#include <media_player/MediaPlayerObserverInterface.h>
#include <captions/CaptionData.h>
#include "StreamFormat.h"

namespace alexaClientSDK {
    namespace acsdkAudioPlayer {
        using namespace std;
        using namespace chrono;
        using namespace captions;
        using namespace avsCommon;
        using namespace avs;
        using namespace attachment;
        using namespace utils;
        using namespace mediaPlayer;
        struct AudioItem {
            string id;
            struct Stream {
                string url;
                shared_ptr<AttachmentReader> reader;
                StreamFormat format;
                milliseconds offset;
                milliseconds endOffset;
                steady_clock::time_point expiryTime;
                struct ProgressReport {
                    milliseconds delay;
                    milliseconds interval;
                } progressReport;
                string token;
                string expectedPreviousToken;
            } stream;
            CaptionData captionData;
            MediaPlayerObserverInterface::VectorOfTags cachedMetadata;
            time_point<steady_clock> lastMetadataEvent;
            PlaybackContext playbackContext;
        };
    }
}
#endif