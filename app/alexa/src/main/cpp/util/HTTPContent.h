#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTPCONTENT_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTPCONTENT_H_

#include <chrono>
#include <future>
#include <memory>
#include <string>
#include <avs/attachment/InProcessAttachment.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            using namespace std;
            using namespace avs;
            using namespace attachment;
            using namespace chrono;
            class HTTPContent {
            public:
                HTTPContent(future<long> httpStatusCode, future<string> httpContentType, shared_ptr<InProcessAttachment> stream);
                bool isStatusCodeSuccess();
                long getStatusCode();
                bool isReady(const milliseconds timeout) const;
                string getContentType();
                shared_ptr<InProcessAttachment> getDataStream();
            private:
                mutable shared_future<long> m_statusCode;
                shared_future<string> m_contentType;
                shared_ptr<InProcessAttachment> m_dataStream;
            };
            inline HTTPContent::HTTPContent(future<long> httpStatusCode, future<string> httpContentType, shared_ptr<InProcessAttachment> stream) :
                                            m_statusCode{std::move(httpStatusCode)}, m_contentType{std::move(httpContentType)}, m_dataStream{stream} {};
            inline long HTTPContent::getStatusCode() {
                auto statusCodeFuture = m_statusCode;
                return statusCodeFuture.get();
            }
            inline bool HTTPContent::isStatusCodeSuccess() {
                auto statusCode = getStatusCode();
                return (statusCode >= 200) && (statusCode < 300);
            }
            inline bool HTTPContent::isReady(const milliseconds timeout) const {
                auto statusCodeFuture = m_statusCode;
                auto status = statusCodeFuture.wait_for(timeout);
                return future_status::ready == status;
            }
            inline string HTTPContent::getContentType() {
                auto contentTypeFuture = m_contentType;
                return contentTypeFuture.get();
            }
            inline shared_ptr<InProcessAttachment> HTTPContent::getDataStream() {
                return m_dataStream;
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTPCONTENT_H_
