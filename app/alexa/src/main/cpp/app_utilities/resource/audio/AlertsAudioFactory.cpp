#include <util/stream/StreamFunctions.h>
#include "data/med_system_alerts_melodic_01.mp3.h"
#include "data/med_system_alerts_melodic_01_short.wav.h"
#include "data/med_system_alerts_melodic_02.mp3.h"
#include "data/med_system_alerts_melodic_02_short.wav.h"
#include "AlertsAudioFactory.h"

namespace alexaClientSDK {
    namespace applicationUtilities {
        namespace resources {
            namespace audio {
                using namespace std;
                using namespace avsCommon;
                using namespace utils;
                using namespace stream;
                using namespace data;
                static pair<unique_ptr<istream>, const MediaType> alarmDefaultFactory() {
                    return make_pair(streamFromData(med_system_alerts_melodic_01_mp3, sizeof(med_system_alerts_melodic_01_mp3)),
                                     MimeTypeToMediaType(med_system_alerts_melodic_01_mp3_mimetype));
                }
                static pair<unique_ptr<istream>, const MediaType> alarmShortFactory() {
                    return make_pair(streamFromData(med_system_alerts_melodic_01_short_wav, sizeof(med_system_alerts_melodic_01_short_wav)),
                                     MimeTypeToMediaType(med_system_alerts_melodic_01_short_wav_mimetype));
                }
                static pair<unique_ptr<istream>, const MediaType> timerDefaultFactory() {
                    return make_pair(stream::streamFromData(med_system_alerts_melodic_02_mp3, sizeof(med_system_alerts_melodic_02_mp3)),
                                     MimeTypeToMediaType(med_system_alerts_melodic_02_mp3_mimetype));
                }
                static pair<unique_ptr<istream>, const MediaType> timerShortFactory() {
                    return make_pair(streamFromData(med_system_alerts_melodic_02_short_wav, sizeof(med_system_alerts_melodic_02_short_wav)),
                                                    MimeTypeToMediaType(med_system_alerts_melodic_02_short_wav_mimetype));
                }
                static pair<unique_ptr<istream>, const MediaType> reminderDefaultFactory() {
                    return make_pair(streamFromData(med_system_alerts_melodic_01_mp3, sizeof(med_system_alerts_melodic_01_mp3)),
                                     MimeTypeToMediaType(med_system_alerts_melodic_01_mp3_mimetype));
                }
                static pair<unique_ptr<istream>, const MediaType> reminderShortFactory() {
                    return make_pair(streamFromData(med_system_alerts_melodic_01_short_wav, sizeof(med_system_alerts_melodic_01_short_wav)),
                                     MimeTypeToMediaType(med_system_alerts_melodic_01_short_wav_mimetype));
                }
                function<pair<unique_ptr<istream>, const MediaType>()> AlertsAudioFactory::alarmDefault() const {
                    return alarmDefaultFactory;
                }
                function<pair<unique_ptr<istream>, const MediaType>()> AlertsAudioFactory::alarmShort() const {
                    return alarmShortFactory;
                }
                function<pair<unique_ptr<istream>, const MediaType>()> AlertsAudioFactory::timerDefault() const {
                    return timerDefaultFactory;
                }
                function<pair<unique_ptr<istream>, const MediaType>()> AlertsAudioFactory::timerShort() const {
                    return timerShortFactory;
                }
                function<pair<unique_ptr<istream>, const MediaType>()> AlertsAudioFactory::reminderDefault() const {
                    return reminderDefaultFactory;
                }
                function<pair<unique_ptr<istream>, const MediaType>()> AlertsAudioFactory::reminderShort() const {
                    return reminderShortFactory;
                }
            }
        }
    }
}