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
                using namespace std;
                using namespace chrono;
                class HTTP2RequestConfig {
                public:
                    HTTP2RequestConfig(HTTP2RequestType type, const string& url, const string& idPrefix);
                    void setConnectionTimeout(milliseconds timeout);
                    void setTransferTimeout(milliseconds timeout);
                    void setActivityTimeout(milliseconds timeout);
                    void setPriority(uint8_t priority);
                    void setRequestSource(shared_ptr<HTTP2RequestSourceInterface> source);
                    void setResponseSink(shared_ptr<HTTP2ResponseSinkInterface> sink);
                    void setIntermittentTransferExpected();
                    void setLogicalStreamIdPrefix(string logicalStreamIdPrefix);
                    HTTP2RequestType getRequestType() const;
                    string getUrl() const;
                    milliseconds getConnectionTimeout() const;
                    milliseconds getTransferTimeout() const;
                    milliseconds getActivityTimeout() const;
                    uint8_t getPriority() const;
                    shared_ptr<HTTP2RequestSourceInterface> getSource() const;
                    shared_ptr<HTTP2ResponseSinkInterface> getSink() const;
                    bool isIntermittentTransferExpected() const;
                    string getId() const;
                private:
                    const HTTP2RequestType m_type;
                    const string m_url;
                    static const uint8_t DEFAULT_PRIORITY = 16;
                    milliseconds m_connectionTimeout;
                    milliseconds m_transferTimeout;
                    milliseconds m_activityTimeout;
                    uint8_t m_priority;
                    shared_ptr<HTTP2RequestSourceInterface> m_source;
                    shared_ptr<HTTP2ResponseSinkInterface> m_sink;
                    bool m_isIntermittentTransferExpected;
                    string m_id;
                };
                inline HTTP2RequestConfig::HTTP2RequestConfig(HTTP2RequestType type, const string& url, const string& idPrefix) : m_type(type), m_url(url),
                                                              m_connectionTimeout{milliseconds::zero()}, m_transferTimeout{milliseconds::zero()},
                                                              m_activityTimeout{milliseconds::zero()}, m_priority{DEFAULT_PRIORITY},
                                                              m_isIntermittentTransferExpected{false} {
                    static atomic<uint64_t> nextId{1};
                    m_id = idPrefix + to_string(atomic_fetch_add(&nextId, uint64_t{2}));
                };
                inline void HTTP2RequestConfig::setConnectionTimeout(milliseconds timeout) {
                    m_connectionTimeout = timeout;
                }
                inline void HTTP2RequestConfig::setTransferTimeout(milliseconds timeout) {
                    m_transferTimeout = timeout;
                }
                inline void HTTP2RequestConfig::setActivityTimeout(milliseconds timeout) {
                    m_activityTimeout = timeout;
                }
                inline void HTTP2RequestConfig::setPriority(uint8_t priority) {
                    m_priority = priority;
                }
                inline void HTTP2RequestConfig::setRequestSource(shared_ptr<HTTP2RequestSourceInterface> source) {
                    m_source = std::move(source);
                }
                inline void HTTP2RequestConfig::setResponseSink(shared_ptr<HTTP2ResponseSinkInterface> sink) {
                    m_sink = std::move(sink);
                }
                inline void HTTP2RequestConfig::setIntermittentTransferExpected() {
                    m_isIntermittentTransferExpected = true;
                }
                inline string HTTP2RequestConfig::getId() const {
                    return m_id;
                }
                inline HTTP2RequestType HTTP2RequestConfig::getRequestType() const {
                    return m_type;
                }
                inline string HTTP2RequestConfig::getUrl() const {
                    return m_url;
                };
                inline milliseconds HTTP2RequestConfig::getConnectionTimeout() const {
                    return m_connectionTimeout;
                }
                inline milliseconds HTTP2RequestConfig::getTransferTimeout() const {
                    return m_transferTimeout;
                }
                inline milliseconds HTTP2RequestConfig::getActivityTimeout() const {
                    return m_activityTimeout;
                }
                inline uint8_t HTTP2RequestConfig::getPriority() const {
                    return m_priority;
                }
                inline shared_ptr<HTTP2RequestSourceInterface> HTTP2RequestConfig::getSource() const {
                    return m_source;
                }
                inline shared_ptr<HTTP2ResponseSinkInterface> HTTP2RequestConfig::getSink() const {
                    return m_sink;
                }
                inline bool HTTP2RequestConfig::isIntermittentTransferExpected() const {
                    return m_isIntermittentTransferExpected;
                }
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP2_HTTP2REQUESTCONFIG_H_
