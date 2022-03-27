#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2REQUESTCONFIG_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2REQUESTCONFIG_H_

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include "HTTP2RequestType.h"
#include "HTTP2RequestSourceInterface.h"
#include "HTTP2ResponseSinkInterface.h"
#include "HTTP2RequestType.h"

#ifdef ACSDK_EMIT_SENSITIVE_LOGS
    #define ACSDK_EMIT_CURL_LOGS
#endif

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace http2 {
                class HTTP2RequestConfig {
                    public:
                        HTTP2RequestConfig(HTTP2RequestType type, const std::string& url, const std::string& idPrefix);
                        void setConnectionTimeout(std::chrono::milliseconds timeout);
                        void setTransferTimeout(std::chrono::milliseconds timeout);
                        void setActivityTimeout(std::chrono::milliseconds timeout);
                        void setPriority(uint8_t priority);
                        void setRequestSource(std::shared_ptr<HTTP2RequestSourceInterface> source);
                        void setResponseSink(std::shared_ptr<HTTP2ResponseSinkInterface> sink);
                        void setIntermittentTransferExpected();
                        void setLogicalStreamIdPrefix(std::string logicalStreamIdPrefix);
                        HTTP2RequestType getRequestType() const;
                        std::string getUrl() const;
                        std::chrono::milliseconds getConnectionTimeout() const;
                        std::chrono::milliseconds getTransferTimeout() const;
                        std::chrono::milliseconds getActivityTimeout() const;
                        uint8_t getPriority() const;
                        std::shared_ptr<HTTP2RequestSourceInterface> getSource() const;
                        std::shared_ptr<HTTP2ResponseSinkInterface> getSink() const;
                        bool isIntermittentTransferExpected() const;
                        std::string getId() const;
                    private:
                        const HTTP2RequestType m_type;
                        const std::string m_url;
                        static const uint8_t DEFAULT_PRIORITY = 16;
                        std::chrono::milliseconds m_connectionTimeout;
                        std::chrono::milliseconds m_transferTimeout;
                        std::chrono::milliseconds m_activityTimeout;
                        uint8_t m_priority;
                        std::shared_ptr<HTTP2RequestSourceInterface> m_source;
                        std::shared_ptr<HTTP2ResponseSinkInterface> m_sink;
                        bool m_isIntermittentTransferExpected;
                        std::string m_id;
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2REQUESTCONFIG_H_
