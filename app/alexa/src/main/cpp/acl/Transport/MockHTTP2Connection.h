#ifndef ALEXA_CLIENT_SDK_ACL_TEST_TRANSPORT_MOCKHTTP2CONNECTION_H_
#define ALEXA_CLIENT_SDK_ACL_TEST_TRANSPORT_MOCKHTTP2CONNECTION_H_

#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <util/PromiseFuturePair.h>
#include <util/HTTP/HttpResponseCode.h>
#include <util/HTTP2/HTTP2ConnectionInterface.h>
#include "MessageConsumerInterface.h"
#include "MockHTTP2Request.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace http2 {
                namespace test {
                    using namespace std;
                    using namespace http;
                    using namespace chrono;
                    class MockHTTP2Connection : public HTTP2ConnectionInterface {
                    public:
                        MockHTTP2Connection(string dURL, string pingURL, bool delegateToReal = true);
                        ~MockHTTP2Connection() = default;
                        MOCK_METHOD1(createAndSendRequest, shared_ptr<HTTP2RequestInterface>(const HTTP2RequestConfig& config));
                        MOCK_METHOD0(disconnect, void());
                        void addObserver(shared_ptr<HTTP2ConnectionObserverInterface> observer);
                        void removeObserver(shared_ptr<HTTP2ConnectionObserverInterface> observer);
                        shared_ptr<HTTP2RequestInterface> createAndSendRequestConcrete(const HTTP2RequestConfig& config);
                        bool isRequestQueueEmpty();
                        shared_ptr<MockHTTP2Request> waitForRequest(milliseconds timeout, unsigned requestNum = 1);
                        shared_ptr<MockHTTP2Request> dequeRequest();
                        void setWaitRequestHeader(const string& matchString);
                        bool waitForRequestWithHeader(milliseconds timeout);
                        shared_ptr<MockHTTP2Request> waitForPostRequest(const milliseconds timeout);
                        shared_ptr<MockHTTP2Request> waitForPingRequest(const milliseconds timeout);
                        bool respondToDownchannelRequests(long responseCode, bool sendResponseFinished, const milliseconds timeout);
                        void setResponseToPOSTRequests(http::HTTPResponseCode responseCode);
                        shared_ptr<MockHTTP2Request> getDownchannelRequest(milliseconds timeout = milliseconds(0));
                        bool isPauseOnSendReceived(milliseconds timeout = milliseconds(0));
                        size_t getPostRequestsNum();
                        size_t getRequestsNum();
                        size_t getDownchannelRequestsNum();
                        shared_ptr<MockHTTP2Request> dequePostRequest(const milliseconds timeout);
                        shared_ptr<MockHTTP2Request> dequePostRequest();
                        shared_ptr<MockHTTP2Request> dequePingRequest();
                        size_t getMaxPostRequestsEnqueud();
                    private:
                        deque<shared_ptr<MockHTTP2Request>> m_requestQueue;
                        mutex m_requestMutex;
                        condition_variable m_requestCv;
                        string m_downchannelURL;
                        string m_pingURL;
                        deque<shared_ptr<MockHTTP2Request>> m_downchannelRequestQueue;
                        mutex m_downchannelRequestMutex;
                        condition_variable m_downchannelRequestCv;
                        deque<shared_ptr<MockHTTP2Request>> m_postRequestQueue;
                        mutex m_postRequestMutex;
                        condition_variable m_requestPostCv;
                        deque<shared_ptr<MockHTTP2Request>> m_pingRequestQueue;
                        mutex m_observersMutex;
                        unordered_set<shared_ptr<HTTP2ConnectionObserverInterface>> m_observers;
                        mutex m_pingRequestMutex;
                        condition_variable m_pingRequestCv;
                        string m_headerMatch;
                        condition_variable m_requestHeaderCv;
                        PromiseFuturePair<void> m_receivedPauseOnSend;
                        HTTPResponseCode m_postResponseCode;
                        size_t m_maxPostRequestsEnqueued;
                        static const unsigned READ_DATA_BUF_SIZE = 100;
                    };
                }
            }
        }
    }
}
#endif