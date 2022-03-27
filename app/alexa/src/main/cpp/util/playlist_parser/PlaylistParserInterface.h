#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_PLAYLISTPARSER_PLAYLISTPARSERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_PLAYLISTPARSER_PLAYLISTPARSERINTERFACE_H_

#include <memory>
#include <string>
#include <vector>
#include "PlaylistParserObserverInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace playlistParser {
                using namespace std;
                class PlaylistParserInterface {
                public:
                    enum class PlaylistType {
                        M3U,
                        EXT_M3U,
                        PLS
                    };
                    virtual ~PlaylistParserInterface() = default;
                    virtual int parsePlaylist(string url, shared_ptr<PlaylistParserObserverInterface> observer,
                                              vector<PlaylistType> playlistTypesToNotBeParsed = vector<PlaylistType>()) = 0;
                };
            }
        }
    }
}
#endif