#ifndef ALEXA_CLIENT_SDK_ACL_TEST_TRANSPORT_MOCKHTTP2REQUEST_H_
#define ALEXA_CLIENT_SDK_ACL_TEST_TRANSPORT_MOCKHTTP2REQUEST_H_

#include <memory>
#include <string>
#include "../../gmock/gmock.h"
#include "../../gtest/gtest.h"
#include "../../util/HTTP2/HTTP2MimeResponseDecoder.h"
#include "../../util/HTTP2/HTTP2RequestConfig.h"
#include "../../util/HTTP2/HTTP2RequestInterface.h"
#include "MockMimeResponseSink.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace http2 {
                namespace test {
                    using namespace std;
                    using namespace acl::test;
                    class MockHTTP2Request : public HTTP2RequestInterface {
                    public:
                        MockHTTP2Request(const HTTP2RequestConfig& config);
                        MOCK_METHOD0(cancel, bool());
                        MOCK_CONST_METHOD0(getId, string());
                        const string getUrl();
                        shared_ptr<HTTP2RequestSourceInterface> getSource();
                        shared_ptr<HTTP2ResponseSinkInterface> getSink();
                        HTTP2RequestType getRequestType();
                        shared_ptr<MockMimeResponseSink> getMimeResponseSink();
                        shared_ptr<HTTP2MimeResponseDecoder> getMimeDecoder();
                    private:
                        const string m_url;
                        shared_ptr<HTTP2RequestSourceInterface> m_source;
                        shared_ptr<HTTP2ResponseSinkInterface> m_sink;
                        HTTP2RequestType m_type;
                        shared_ptr<MockMimeResponseSink> m_mimeResponseSink;
                        shared_ptr<HTTP2MimeResponseDecoder> m_mimeDecoder;
                    };
                }
            }
        }
    }
}
#endif