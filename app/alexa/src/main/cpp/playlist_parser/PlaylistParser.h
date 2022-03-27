#ifndef ALEXA_CLIENT_SDK_PLAYLISTPARSER_INCLUDE_PLAYLISTPARSER_PLAYLISTPARSER_H_
#define ALEXA_CLIENT_SDK_PLAYLISTPARSER_INCLUDE_PLAYLISTPARSER_PLAYLISTPARSER_H_

#include <deque>
#include <memory>
#include <unordered_map>
#include <avs/attachment/AttachmentReader.h>
#include <sdkinterfaces/HTTPContentFetcherInterfaceFactoryInterface.h>
#include <util/playlist_parser/PlaylistParserInterface.h>
#include <util/RequiresShutdown.h>
#include <threading/Executor.h>

namespace alexaClientSDK {
    namespace playlistParser {
        using namespace std;
        using namespace avsCommon;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace threading;
        using namespace utils::playlistParser;
        class PlaylistParser : public PlaylistParserInterface, public RequiresShutdown {
        public:
            static unique_ptr<PlaylistParser> create(shared_ptr<HTTPContentFetcherInterfaceFactoryInterface> contentFetcherFactory);
            int parsePlaylist(string url, shared_ptr<PlaylistParserObserverInterface> observer,
                              vector<PlaylistType> playlistTypesToNotBeParsed = vector<PlaylistType>()) override;
            static const int START_FAILURE = 0;
            void doShutdown() override;
        private:
            PlaylistParser(shared_ptr<HTTPContentFetcherInterfaceFactoryInterface> contentFetcherFactory);
            void doDepthFirstSearch(int id, shared_ptr<PlaylistParserObserverInterface> observer, const string& rootUrl,
                                    vector<PlaylistType> playlistTypesToNotBeParsed);
            bool getPlaylistContent(unique_ptr<HTTPContentFetcherInterface> contentFetcher, string* content);
            shared_ptr<HTTPContentFetcherInterfaceFactoryInterface> m_contentFetcherFactory;
            atomic<bool> m_shuttingDown;
            Executor m_executor;
        };
    }
}

#endif