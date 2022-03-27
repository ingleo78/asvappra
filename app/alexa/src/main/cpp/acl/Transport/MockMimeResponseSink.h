#ifndef ALEXA_CLIENT_SDK_ACL_TEST_TRANSPORT_MOCKMIMERESPONSESINK_H_
#define ALEXA_CLIENT_SDK_ACL_TEST_TRANSPORT_MOCKMIMERESPONSESINK_H_

#include <vector>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <util/HTTP2/HTTP2MimeResponseSinkInterface.h>
#include <util/HTTP2/HTTP2ReceiveDataStatus.h>
#include <util/HTTP2/HTTP2ResponseFinishedStatus.h>

namespace alexaClientSDK {
    namespace acl {
        namespace test {
            using namespace std;
            using namespace avsCommon::utils::http2;
            class MockMimeResponseSink : public HTTP2MimeResponseSinkInterface {
            public:
                MockMimeResponseSink();
                virtual ~MockMimeResponseSink() = default;
                bool onReceiveResponseCode(long responseCode) override;
                bool onReceiveHeaderLine(const string& line) override;
                bool onBeginMimePart(const multimap<string, string>& headers) override;
                HTTP2ReceiveDataStatus onReceiveMimeData(const char* bytes, size_t size);
                bool onEndMimePart() override;
                HTTP2ReceiveDataStatus onReceiveNonMimeData(const char* bytes, size_t size);
                void onResponseFinished(HTTP2ResponseFinishedStatus status);
                vector<char> getMimePart(unsigned part);
                unsigned getCountOfMimeParts();
            private:
                vector<vector<char>> m_mimeContents;
                vector<char> m_mimeCurrentContent;
            };
        }
    }
}
#endif