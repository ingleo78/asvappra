#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_PLAYLISTPARSER_ITERATIVEPLAYLISTPARSERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_PLAYLISTPARSER_ITERATIVEPLAYLISTPARSERINTERFACE_H_

#include <memory>
#include <string>
#include <vector>
#include "PlaylistParserObserverInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace playlistParser {
                class IterativePlaylistParserInterface {
                public:
                    virtual bool initializeParsing(std::string url) = 0;
                    virtual PlaylistEntry next() = 0;
                    virtual void abort() = 0;
                    virtual ~IterativePlaylistParserInterface() = default;
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_PLAYLISTPARSER_ITERATIVEPLAYLISTPARSERINTERFACE_H_
