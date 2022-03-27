#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2MIMERESPONSESINKINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2MIMERESPONSESINKINTERFACE_H_

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include "HTTP2ResponseSinkInterface.h"
#include "HTTP2ReceiveDataStatus.h"
#include "HTTP2ResponseFinishedStatus.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace http2 {
                class HTTP2MimeResponseSinkInterface {
                    public:
                        virtual ~HTTP2MimeResponseSinkInterface() = default;
                        virtual bool onReceiveResponseCode(long responseCode) = 0;
                        virtual bool onReceiveHeaderLine(const std::string& line) = 0;
                        virtual bool onBeginMimePart(const std::multimap<std::string, std::string>& headers) = 0;
                        virtual HTTP2ReceiveDataStatus onReceiveMimeData(const char* bytes, size_t size) = 0;
                        virtual bool onEndMimePart() = 0;
                        virtual HTTP2ReceiveDataStatus onReceiveNonMimeData(const char* bytes, size_t size) = 0;
                        virtual void onResponseFinished(HTTP2ResponseFinishedStatus status) = 0;
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2MIMERESPONSESINKINTERFACE_H_
