#include "HTTP2RequestConfig.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace http2 {
                inline HTTP2RequestConfig::HTTP2RequestConfig(HTTP2RequestType type, const std::string& url, const std::string& idPrefix) : m_type(type),
                        m_url(url), m_connectionTimeout{std::chrono::milliseconds::zero()}, m_transferTimeout{std::chrono::milliseconds::zero()},
                        m_activityTimeout{std::chrono::milliseconds::zero()}, m_priority{DEFAULT_PRIORITY}, m_isIntermittentTransferExpected{false} {
                    static std::atomic<uint64_t> nextId{1};
                    m_id = idPrefix + std::to_string(std::atomic_fetch_add(&nextId, uint64_t{2}));
                }
                inline void HTTP2RequestConfig::setConnectionTimeout(std::chrono::milliseconds timeout) { m_connectionTimeout = timeout; }
                inline void HTTP2RequestConfig::setTransferTimeout(std::chrono::milliseconds timeout) { m_transferTimeout = timeout; }
                inline void HTTP2RequestConfig::setActivityTimeout(std::chrono::milliseconds timeout) { m_activityTimeout = timeout; }
                inline void HTTP2RequestConfig::setPriority(uint8_t priority) { m_priority = priority; }
                inline void HTTP2RequestConfig::setRequestSource(std::shared_ptr<HTTP2RequestSourceInterface> source) { m_source = std::move(source); }
                inline void HTTP2RequestConfig::setResponseSink(std::shared_ptr<HTTP2ResponseSinkInterface> sink) { m_sink = std::move(sink); }
                inline void HTTP2RequestConfig::setIntermittentTransferExpected() { m_isIntermittentTransferExpected = true; }
                inline std::string HTTP2RequestConfig::getId() const { return m_id; }
                inline HTTP2RequestType HTTP2RequestConfig::getRequestType() const { return m_type; }
                inline std::string HTTP2RequestConfig::getUrl() const { return m_url; }
                inline std::chrono::milliseconds HTTP2RequestConfig::getConnectionTimeout() const { return m_connectionTimeout; }
                inline std::chrono::milliseconds HTTP2RequestConfig::getTransferTimeout() const { return m_transferTimeout; }
                inline std::chrono::milliseconds HTTP2RequestConfig::getActivityTimeout() const { return m_activityTimeout; }
                inline uint8_t HTTP2RequestConfig::getPriority() const { return m_priority; }
                inline std::shared_ptr<HTTP2RequestSourceInterface> HTTP2RequestConfig::getSource() const { return m_source; }
                inline std::shared_ptr<HTTP2ResponseSinkInterface> HTTP2RequestConfig::getSink() const { return m_sink; }
                inline bool HTTP2RequestConfig::isIntermittentTransferExpected() const { return m_isIntermittentTransferExpected; }
            }
        }
    }
}
