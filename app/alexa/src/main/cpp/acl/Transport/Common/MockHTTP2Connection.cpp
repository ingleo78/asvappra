#include <acl/Transport/MockHTTP2Connection.h>
#include <util/HTTP2/HTTP2SendStatus.h>
#include "util/HTTP2/HTTP2ResponseFinishedStatus.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace http2 {
                namespace test {
                    using namespace testing;
                    using namespace http;
                    using namespace std;
                    using namespace chrono;
                    MockHTTP2Connection::MockHTTP2Connection(string dURL, string pingURL, bool delegateToReal) :
                                                             m_downchannelURL{dURL}, m_pingURL{pingURL},
                                                             m_postResponseCode{HTTPResponseCode::HTTP_RESPONSE_CODE_UNDEFINED},
                                                             m_maxPostRequestsEnqueued{0} {
                        if (delegateToReal) {
                            ON_CALL(*this, createAndSendRequest(_)).WillByDefault(Invoke(this, &MockHTTP2Connection::createAndSendRequestConcrete));
                        }
                    }
                    shared_ptr<HTTP2RequestInterface> MockHTTP2Connection::createAndSendRequestConcrete(const HTTP2RequestConfig& config) {
                        lock_guard<std::mutex> lock(m_requestMutex);
                        auto request = make_shared<MockHTTP2Request>(config);
                        m_requestQueue.push_back(request);
                        if (m_headerMatch.length() > 0) {
                            for (auto header : request->getSource()->getRequestHeaderLines()) {
                                if (header.find(m_headerMatch) != std::string::npos) {
                                    m_requestHeaderCv.notify_one();
                                }
                            }
                        }
                        if (request->getRequestType() == HTTP2RequestType::POST) {
                            lock_guard<std::mutex> lock(m_postRequestMutex);
                            m_postRequestQueue.push_back(request);
                            if (m_postResponseCode != HTTPResponseCode::HTTP_RESPONSE_CODE_UNDEFINED) {
                                request->getSink()->onReceiveResponseCode(responseCodeToInt(m_postResponseCode));
                            }
                            if (m_postRequestQueue.size() > m_maxPostRequestsEnqueued) {
                                m_maxPostRequestsEnqueued = m_postRequestQueue.size();
                            }
                            m_requestPostCv.notify_one();
                        } else if (m_downchannelURL == request->getUrl()) {
                            lock_guard<mutex> lock(m_downchannelRequestMutex);
                            m_downchannelRequestQueue.push_back(request);
                            m_downchannelRequestCv.notify_all();
                        } else if (m_pingURL == request->getUrl()) {
                            lock_guard<mutex> lock(m_pingRequestMutex);
                            m_pingRequestQueue.push_back(request);
                            m_pingRequestCv.notify_one();
                        }
                        m_requestCv.notify_one();
                        return request;
                    }
                    bool MockHTTP2Connection::isRequestQueueEmpty() {
                        lock_guard<mutex> lock(m_requestMutex);
                        return m_requestQueue.empty();
                    }
                    shared_ptr<MockHTTP2Request> MockHTTP2Connection::waitForRequest(milliseconds timeout, unsigned requestNum) {
                        unique_lock<mutex> lock(m_requestMutex);
                        if (!m_requestCv.wait_for(lock, timeout, [this, requestNum] {
                                return !m_requestQueue.empty() && m_requestQueue.size() >= requestNum;
                            })) {
                            return nullptr;
                        }
                        return m_requestQueue.front();
                    }
                    shared_ptr<MockHTTP2Request> MockHTTP2Connection::dequeRequest() {
                        lock_guard<mutex> lock(m_requestMutex);
                        auto req = m_requestQueue.front();
                        m_requestQueue.pop_front();
                        return req;
                    }
                    void MockHTTP2Connection::setWaitRequestHeader(const string& matchString) {
                        lock_guard<mutex> lock(m_requestMutex);
                        m_headerMatch = matchString;
                    }
                    bool MockHTTP2Connection::waitForRequestWithHeader(milliseconds timeout) {
                        if (waitForRequest(timeout)) {
                            unique_lock<mutex> lock(m_requestMutex);
                            return m_requestHeaderCv.wait_for(lock, timeout, [this] { return !m_requestQueue.empty(); });
                        }
                        return false;
                    }
                    shared_ptr<MockHTTP2Request> MockHTTP2Connection::waitForPostRequest(const milliseconds timeout) {
                        unique_lock<mutex> lock(m_postRequestMutex);
                        if (!m_requestPostCv.wait_for(lock, timeout, [this] { return !m_postRequestQueue.empty(); })) {
                            return nullptr;
                        }
                        auto request = m_postRequestQueue.back();
                        request->getMimeDecoder()->onReceiveResponseCode(
                            static_cast<underlying_type<HTTPResponseCode>::type>(HTTPResponseCode::SUCCESS_OK));
                        for (auto headerLine : request->getSource()->getRequestHeaderLines()) {
                            request->getMimeDecoder()->onReceiveHeaderLine(headerLine);
                        }
                        char buf[READ_DATA_BUF_SIZE] = {'\0'};
                        bool stop = false;
                        do {
                            auto res = request->getSource()->onSendData(buf, READ_DATA_BUF_SIZE);
                            switch (res.status) {
                                case HTTP2SendStatus::COMPLETE: case HTTP2SendStatus::ABORT: stop = true; break;
                                case HTTP2SendStatus::PAUSE: if (!isPauseOnSendReceived()) m_receivedPauseOnSend.setValue();
                                case HTTP2SendStatus::CONTINUE: break;
                            }
                            if (stop) break;
                            request->getMimeDecoder()->onReceiveData(buf, res.size);
                        } while (true);
                        return request;
                    }
                    shared_ptr<MockHTTP2Request> MockHTTP2Connection::waitForPingRequest(const milliseconds timeout) {
                        unique_lock<mutex> lock(m_pingRequestMutex);
                        if (!m_pingRequestCv.wait_for(lock, timeout, [this] { return !m_pingRequestQueue.empty(); })) return nullptr;
                        return m_pingRequestQueue.back();
                    }
                    bool MockHTTP2Connection::respondToDownchannelRequests(long responseCode, bool sendResponseFinished,
                                                                           const milliseconds timeout) {
                        unique_lock<mutex> lock(m_downchannelRequestMutex);
                        auto ret = m_downchannelRequestCv.wait_for(lock, timeout, [this] { return !m_downchannelRequestQueue.empty(); });
                        for (auto request : m_downchannelRequestQueue) {
                            request->getSink()->onReceiveResponseCode(responseCode);
                            if (sendResponseFinished) request->getSink()->onResponseFinished(HTTP2ResponseFinishedStatus::COMPLETE);
                        }
                        return ret;
                    }
                    void MockHTTP2Connection::setResponseToPOSTRequests(HTTPResponseCode responseCode) {
                        lock_guard<mutex> lock(m_postRequestMutex);
                        m_postResponseCode = responseCode;
                    }
                    shared_ptr<MockHTTP2Request> MockHTTP2Connection::getDownchannelRequest(std::chrono::milliseconds timeout) {
                        unique_lock<mutex> lock(m_downchannelRequestMutex);
                        m_downchannelRequestCv.wait_for(lock, timeout);
                        if (m_downchannelRequestQueue.empty()) return nullptr;
                        return m_downchannelRequestQueue.back();
                    }
                    bool MockHTTP2Connection::isPauseOnSendReceived(milliseconds timeout) {
                        return m_receivedPauseOnSend.waitFor(timeout);
                    }
                    size_t MockHTTP2Connection::getPostRequestsNum() {
                        return m_postRequestQueue.size();
                    }
                    size_t MockHTTP2Connection::getRequestsNum() {
                        return m_requestQueue.size();
                    }
                    size_t MockHTTP2Connection::getDownchannelRequestsNum() {
                        return m_downchannelRequestQueue.size();
                    }
                    shared_ptr<MockHTTP2Request> MockHTTP2Connection::dequePostRequest() {
                        lock_guard<std::mutex> lock(m_postRequestMutex);
                        if (m_postRequestQueue.empty()) return nullptr;
                        auto req = m_postRequestQueue.front();
                        m_postRequestQueue.pop_front();
                        return req;
                    }
                    shared_ptr<MockHTTP2Request> MockHTTP2Connection::dequePostRequest(const milliseconds timeout) {
                        auto req = dequePostRequest();
                        if (!req) {
                            if (waitForPostRequest(timeout) == nullptr) return nullptr;
                            req = dequePostRequest();
                        }
                        return req;
                    }
                    shared_ptr<MockHTTP2Request> MockHTTP2Connection::dequePingRequest() {
                        lock_guard<mutex> lock(m_pingRequestMutex);
                        if (m_pingRequestQueue.empty()) return nullptr;
                        auto req = m_pingRequestQueue.front();
                        m_pingRequestQueue.pop_front();
                        return req;
                    }
                    size_t MockHTTP2Connection::getMaxPostRequestsEnqueud() {
                        return m_maxPostRequestsEnqueued;
                    }
                    void MockHTTP2Connection::addObserver(shared_ptr<HTTP2ConnectionObserverInterface> observer) {
                        lock_guard<mutex> lock{m_observersMutex};
                        m_observers.insert(observer);
                    }
                    void MockHTTP2Connection::removeObserver(shared_ptr<HTTP2ConnectionObserverInterface> observer) {
                        lock_guard<std::mutex> lock{m_observersMutex};
                        m_observers.erase(observer);
                    }
                }
            }
        }
    }
}