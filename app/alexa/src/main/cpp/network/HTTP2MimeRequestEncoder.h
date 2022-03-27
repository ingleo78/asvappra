#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2MIMEREQUESTENCODER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2MIMEREQUESTENCODER_H_

#include <memory>
#include <ostream>
#include <string>
#include <vector>
#include "HTTP2MimeRequestSourceInterface.h"
#include "HTTP2RequestSourceInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace http2 {
                class HTTP2MimeRequestEncoder : public HTTP2RequestSourceInterface {
                    public:
                        HTTP2MimeRequestEncoder(const std::string& boundary, std::shared_ptr<HTTP2MimeRequestSourceInterface> source);
                        ~HTTP2MimeRequestEncoder() = default;
                        HTTP2SendDataResult onSendData(char* bytes, size_t size) override;
                        std::vector<std::string> getRequestHeaderLines() override;
                    private:
                        enum class State {
                            NEW,
                            GETTING_1ST_PART_HEADERS,
                            SENDING_1ST_BOUNDARY,
                            SENDING_PART_HEADERS,
                            SENDING_PART_DATA,
                            SENDING_END_BOUNDARY,
                            GETTING_NTH_PART_HEADERS,
                            SENDING_CRLF_AFTER_BOUNDARY,
                            SENDING_TERMINATING_DASHES,
                            DONE,
                            ABORT
                        };
                        friend std::ostream& operator<<(std::ostream& stream, State state);
                        void setState(State newState);
                        bool sendString(char* bytes, size_t size, const std::string& text);
                        bool sendStringAndCRLF(char* bytes, size_t size, const std::string& text);
                        HTTP2SendDataResult continueResult();
                        State m_state;
                        std::string m_rawBoundary;
                        std::string m_prefixedBoundary;
                        std::shared_ptr<HTTP2MimeRequestSourceInterface> m_source;
                        size_t m_bytesCopied;
                        HTTP2GetMimeHeadersResult m_getMimeHeaderLinesResult;
                        std::vector<std::string>::const_iterator m_headerLine;
                        size_t m_stringIndex;
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2MIMEREQUESTENCODER_H_
