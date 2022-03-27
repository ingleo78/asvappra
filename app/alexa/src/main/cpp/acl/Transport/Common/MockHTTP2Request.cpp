#include "../MockHTTP2Request.h"
#include "../MockMimeResponseSink.h"
#include "../../../util/HTTP2/HTTP2MimeResponseDecoder.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace http2 {
                namespace test {
                    using namespace std;
                    using namespace acl::test;
                    MockHTTP2Request::MockHTTP2Request(const HTTP2RequestConfig& config) : m_url{config.getUrl()}, m_source{config.getSource()},
                                                       m_sink{config.getSink()}, m_type{config.getRequestType()} {
                        m_mimeResponseSink = make_shared<MockMimeResponseSink>();
                        m_mimeDecoder = make_shared<HTTP2MimeResponseDecoder>(m_mimeResponseSink);
                    }
                    const string MockHTTP2Request::getUrl() {
                        return m_url;
                    }
                    shared_ptr<HTTP2RequestSourceInterface> MockHTTP2Request::getSource() {
                        return m_source;
                    }
                    shared_ptr<HTTP2ResponseSinkInterface> MockHTTP2Request::getSink() {
                        return m_sink;
                    }
                    HTTP2RequestType MockHTTP2Request::getRequestType() {
                        return m_type;
                    }
                    shared_ptr<MockMimeResponseSink> MockHTTP2Request::getMimeResponseSink() {
                        return m_mimeResponseSink;
                    }
                    shared_ptr<HTTP2MimeResponseDecoder> MockHTTP2Request::getMimeDecoder() {
                        return m_mimeDecoder;
                    }
                }
            }
        }
    }
}