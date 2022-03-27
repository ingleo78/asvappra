#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_PLAYLISTPARSER_PLAYLISTENTRY_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_PLAYLISTPARSER_PLAYLISTENTRY_H_

#include <chrono>
#include <ostream>
#include <queue>
#include <string>
#include <tuple>
#include <sdkinterfaces/HTTPContentFetcherInterface.h>
#include <util/HTTPContent.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace playlistParser {
                using namespace std;
                using namespace chrono;
                using namespace sdkInterfaces;
                using namespace playlistParser;
                typedef tuple<long, long> ByteRange;
                enum class PlaylistParseResult {
                    FINISHED,
                    ERROR,
                    STILL_ONGOING,
                    SHUTDOWN
                };
                struct EncryptionInfo {
                    enum class Method {
                        NONE,
                        AES_128,
                        SAMPLE_AES
                    };
                    EncryptionInfo();
                    EncryptionInfo(Method method, string url, string initVector = string());
                    bool isValid() const;
                    Method method;
                    string keyURL;
                    string initVector;
                };
                struct PlaylistEntry {
                    enum class Type {
                        MEDIA_INFO,
                        MEDIA_INIT_INFO,
                        AUDIO_CONTENT
                    };
                    static PlaylistEntry createErrorEntry(const string& url, ByteRange& byteRange);
                    static PlaylistEntry createShutdownEntry(const string& url, ByteRange& byteRange);
                    static PlaylistEntry createMediaInitInfo(string url, ByteRange& byteRange);
                    PlaylistEntry(string _url, milliseconds _duration, PlaylistParseResult _parseResult, Type _type, ByteRange& byteRange,
                                  EncryptionInfo _encryptionInfo = EncryptionInfo(), shared_ptr<HTTPContentFetcherInterface> _contentFetcher = nullptr) :
                                  type(_type), url(_url), duration(_duration), parseResult(_parseResult), byteRange(byteRange), encryptionInfo(_encryptionInfo),
                                  contentFetcher(_contentFetcher) {}
                    bool hasValidByteRange() const;
                    Type type;
                    string url;
                    milliseconds duration;
                    PlaylistParseResult parseResult;;
                    ByteRange& byteRange;
                    EncryptionInfo encryptionInfo;
                    shared_ptr<HTTPContentFetcherInterface> contentFetcher;
                };
                inline PlaylistEntry PlaylistEntry::createErrorEntry(const string& url, ByteRange& byteRange) {
                    return PlaylistEntry(url, milliseconds(-1), PlaylistParseResult::ERROR, Type::MEDIA_INFO, byteRange);
                }
                inline PlaylistEntry PlaylistEntry::createShutdownEntry(const string& url, ByteRange& byteRange) {
                    return PlaylistEntry(url, milliseconds(-1), PlaylistParseResult::SHUTDOWN, Type::MEDIA_INFO, byteRange);
                }
                inline PlaylistEntry PlaylistEntry::createMediaInitInfo(string url, ByteRange& byteRange) {
                    auto duration = milliseconds(-1);
                    return PlaylistEntry(url, duration, PlaylistParseResult::STILL_ONGOING, Type::MEDIA_INIT_INFO, byteRange);
                }
                inline EncryptionInfo::EncryptionInfo() : method(Method::NONE) {}
                inline EncryptionInfo::EncryptionInfo(Method method, string url, string initVector) : method(method), keyURL(url), initVector(initVector) {}
                inline bool EncryptionInfo::isValid() const {
                    return (Method::NONE == method) || (Method::NONE != method && !keyURL.empty() && !initVector.empty());
                }
                inline bool PlaylistEntry::hasValidByteRange() const {
                    auto [start, end] = byteRange;
                    return start >= 0 && end > 0;
                }
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_PLAYLISTPARSER_PLAYLISTENTRY_H_
