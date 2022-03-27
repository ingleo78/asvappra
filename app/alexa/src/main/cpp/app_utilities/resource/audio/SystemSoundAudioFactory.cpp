#include <util/stream/StreamFunctions.h>
#include "data/med_ui_endpointing.wav.h"
#include "data/med_ui_wakesound.wav.h"
#include "SystemSoundAudioFactory.h"

namespace alexaClientSDK {
    namespace applicationUtilities {
        namespace resources {
            namespace audio {
                using namespace std;
                using namespace data;
                using namespace avsCommon::utils;
                using namespace stream;
                static pair<unique_ptr<istream>, const MediaType> wakeWordNotificationToneFactory() {
                    return make_pair(streamFromData(med_ui_wakesound_wav, sizeof(med_ui_wakesound_wav)),
                                     MimeTypeToMediaType(med_ui_wakesound_wav_mimetype));
                }
                static pair<unique_ptr<istream>, const MediaType> endSpeechToneFactory() {
                    return make_pair(streamFromData(med_ui_endpointing_wav, sizeof(med_ui_endpointing_wav)),
                                     MimeTypeToMediaType(med_ui_endpointing_wav_mimetype));
                }
                function<pair<unique_ptr<istream>, const MediaType>()> SystemSoundAudioFactory::wakeWordNotificationTone() const {
                    return wakeWordNotificationToneFactory;
                }
                function<pair<unique_ptr<istream>, const MediaType>()> SystemSoundAudioFactory::endSpeechTone() const {
                    return endSpeechToneFactory;
                }
            }
        }
    }
}