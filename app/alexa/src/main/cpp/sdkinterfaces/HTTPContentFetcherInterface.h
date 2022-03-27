#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_HTTPCONTENTFETCHERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_HTTPCONTENTFETCHERINTERFACE_H_

#include <memory>
#include <stdint.h>
#include <avs/attachment/AttachmentWriter.h>
#include <util/HTTP/HttpResponseCode.h>
#include <util/HTTPContent.h>
#include <util/SDKVersion.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            typedef signed int ssize_t;
            using namespace std;
            using namespace avs;
            using namespace attachment;
            using namespace utils;
            using namespace http;
            class HTTPContentFetcherInterface {
            public:
                enum class FetchOptions {
                    CONTENT_TYPE,
                    ENTIRE_BODY
                };
                enum class State {
                    INITIALIZED,
                    FETCHING_HEADER,
                    HEADER_DONE,
                    FETCHING_BODY,
                    BODY_DONE,
                    ERROR
                };
                struct Header {
                    bool successful;
                    HTTPResponseCode responseCode;
                    string contentType;
                    ssize_t contentLength;
                    Header() : successful{false}, responseCode{HTTPResponseCode::HTTP_RESPONSE_CODE_UNDEFINED}, contentType{""}, contentLength{0}{}
                };
                virtual ~HTTPContentFetcherInterface() = default;
                virtual State getState() = 0;
                virtual string getUrl() const = 0;
                virtual Header getHeader(atomic<bool>* shouldShutdown) = 0;
                virtual bool getBody(shared_ptr<AttachmentWriter> writer) = 0;
                virtual void shutdown() = 0;
                virtual unique_ptr<HTTPContent> getContent(FetchOptions option, unique_ptr<AttachmentWriter> writer, const vector<string>& customHeaders) = 0;
                static string getUserAgent();
                static string_view stateToString(State state);
            };
            inline string HTTPContentFetcherInterface::getUserAgent() {
                return "AvsDeviceSdk/" + sdkVersion::getCurrentVersion();
            }
            inline string_view HTTPContentFetcherInterface::stateToString(HTTPContentFetcherInterface::State state) {
                switch (state) {
                    case State::INITIALIZED: return "INITIALIZED";
                    case State::FETCHING_HEADER: return "FETCHING_HEADER";
                    case State::HEADER_DONE: return "HEADER_DONE";
                    case State::FETCHING_BODY: return "FETCHING_BODY";
                    case State::BODY_DONE: return "BODY_DONE";
                    case State::ERROR: return "ERROR";
                }
                return "";
            }
            inline ostream& operator<<(ostream& os, const HTTPContentFetcherInterface::State& state) {
                os << HTTPContentFetcherInterface::stateToString(state);
                return os;
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_HTTPCONTENTFETCHERINTERFACE_H_
