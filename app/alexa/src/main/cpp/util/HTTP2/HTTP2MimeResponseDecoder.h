#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2MIMERESPONSEDECODER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2MIMERESPONSEDECODER_H_

#include <memory>
#include <multiparser/MultipartReader.h>
#include "HTTP2MimeResponseSinkInterface.h"
#include "HTTP2ResponseSinkInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace http2 {
                class HTTP2MimeResponseDecoder : public HTTP2ResponseSinkInterface {
                public:
                    HTTP2MimeResponseDecoder(std::shared_ptr<HTTP2MimeResponseSinkInterface> sink);
                    ~HTTP2MimeResponseDecoder() = default;
                    bool onReceiveResponseCode(long responseCode) override;
                    bool onReceiveHeaderLine(const std::string& line) override;
                    HTTP2ReceiveDataStatus onReceiveData(const char* bytes, size_t size) override;
                    void onResponseFinished(HTTP2ResponseFinishedStatus status) override;
                private:
                    static void partBeginCallback(const MultipartHeaders& headers, void* userData);
                    static void partDataCallback(const char* buffer, size_t size, void* userData);
                    static void partEndCallback(void* userData);
                    std::shared_ptr<HTTP2MimeResponseSinkInterface> m_sink;
                    long m_responseCode;
                    MultipartReader m_multipartReader;
                    HTTP2ReceiveDataStatus m_lastStatus;
                    size_t m_index;
                    size_t m_leadingCRLFCharsLeftToRemove;
                    bool m_boundaryFound;
                    size_t m_lastSuccessIndex;
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2MIMERESPONSEDECODER_H_
