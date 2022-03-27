#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_PLAYLISTPARSER_PLAYLISTPARSEROBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_PLAYLISTPARSER_PLAYLISTPARSEROBSERVERINTERFACE_H_

#include <chrono>
#include <ostream>
#include <queue>
#include <string>
#include "PlaylistEntry.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace playlistParser {
                class PlaylistParserObserverInterface {
                public:
                    virtual ~PlaylistParserObserverInterface() = default;
                    virtual void onPlaylistEntryParsed(int requestId, PlaylistEntry playlistEntry) = 0;
                };
                inline std::ostream& operator<<(std::ostream& stream, const PlaylistParseResult& result) {
                    switch (result) {
                        case PlaylistParseResult::FINISHED: stream << "FINISHED"; break;
                        case PlaylistParseResult::ERROR: stream << "ERROR"; break;
                        case PlaylistParseResult::STILL_ONGOING: stream << "STILL_ONGOING"; break;
                        case PlaylistParseResult::SHUTDOWN: stream << "SHUTDOWN"; break;
                    }
                    return stream;
                }
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_PLAYLISTPARSER_PLAYLISTPARSEROBSERVERINTERFACE_H_
