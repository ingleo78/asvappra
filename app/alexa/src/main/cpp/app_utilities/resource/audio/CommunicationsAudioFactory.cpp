#include <util/stream/StreamFunctions.h>
#include "data/med_comms_call_connected.mp3.h"
#include "data/med_comms_call_disconnected.mp3.h"
#include "data/med_comms_call_incoming_ringtone.mp3.h"
#include "data/med_comms_drop_in_incoming.mp3.h"
#include "data/med_comms_outbound_ringtone.mp3.h"
#include "CommunicationsAudioFactory.h"

namespace alexaClientSDK {
    namespace applicationUtilities {
        namespace resources {
            namespace audio {
                using namespace std;
                using namespace data;
                using namespace avsCommon::utils;
                using namespace stream;
                static pair<unique_ptr<istream>, const MediaType> callConnectedRingtoneFactory() {
                    return make_pair(streamFromData(med_comms_call_connected_mp3, sizeof(med_comms_call_connected_mp3)),
                                     MimeTypeToMediaType(med_comms_call_connected_mp3_mimetype));
                }
                static pair<unique_ptr<istream>, const MediaType> callDisconnectedRingtoneFactory() {
                    return make_pair(streamFromData(med_comms_call_disconnected_mp3, sizeof(med_comms_call_disconnected_mp3)),
                                     MimeTypeToMediaType(med_comms_call_disconnected_mp3_mimetype));
                }
                static pair<unique_ptr<istream>, const MediaType> outboundRingtoneFactory() {
                    return make_pair(streamFromData(med_comms_outbound_ringtone_mp3, sizeof(med_comms_outbound_ringtone_mp3)),
                                     MimeTypeToMediaType(med_comms_outbound_ringtone_mp3_mimetype));
                }
                static pair<unique_ptr<istream>, const MediaType> dropInIncomingFactory() {
                    return make_pair(streamFromData(med_comms_drop_in_incoming_mp3, sizeof(med_comms_drop_in_incoming_mp3)),
                                     MimeTypeToMediaType(med_comms_drop_in_incoming_mp3_mimetype));
                }
                static pair<unique_ptr<istream>, const MediaType> callIncomingRingtoneFactory() {
                    return make_pair(streamFromData(med_comms_call_incoming_ringtone_mp3, sizeof(med_comms_call_incoming_ringtone_mp3)),
                                     MimeTypeToMediaType(med_comms_call_incoming_ringtone_mp3_mimetype));
                }
                function<pair<unique_ptr<istream>, const MediaType>()> CommunicationsAudioFactory::callConnectedRingtone() const {
                    return callConnectedRingtoneFactory;
                }
                function<pair<unique_ptr<istream>, const MediaType>()> CommunicationsAudioFactory::callDisconnectedRingtone() const {
                    return callDisconnectedRingtoneFactory;
                }
                function<pair<unique_ptr<istream>, const MediaType>()> CommunicationsAudioFactory::outboundRingtone() const {
                    return outboundRingtoneFactory;
                }
                function<pair<unique_ptr<istream>, const MediaType>()> CommunicationsAudioFactory::dropInIncoming() const {
                    return dropInIncomingFactory;
                }
                function<pair<unique_ptr<istream>, const MediaType>()> CommunicationsAudioFactory::callIncomingRingtone() const {
                    return callIncomingRingtoneFactory;
                }
            }
        }
    }
}