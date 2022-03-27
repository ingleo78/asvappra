#include <logger/Logger.h>
#include <util/HTTP/HttpResponseCode.h>
#include "HTTP2MimeResponseDecoder.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace http2 {
                using namespace avsCommon::utils::http;
                static const std::string TAG("HTTP2MimeResponseDecoder");
                #define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)
                static const char CARRIAGE_RETURN_ASCII = 13;
                static const char LINE_FEED_ASCII = 10;
                static const char CRLF_SEQUENCE[] = {CARRIAGE_RETURN_ASCII, LINE_FEED_ASCII};
                static const int LEADING_CRLF_CHAR_SIZE = sizeof(CRLF_SEQUENCE) / sizeof(*CRLF_SEQUENCE);
                static const std::string BOUNDARY_PREFIX = "boundary=";
                static const int BOUNDARY_PREFIX_SIZE = BOUNDARY_PREFIX.size();
                static const std::string BOUNDARY_DELIMITER = ";";
                HTTP2MimeResponseDecoder::HTTP2MimeResponseDecoder(std::shared_ptr<HTTP2MimeResponseSinkInterface> sink) :
                        m_sink{sink},
                        m_responseCode{0},
                        m_lastStatus{HTTP2ReceiveDataStatus::SUCCESS},
                        m_index{0},
                        m_leadingCRLFCharsLeftToRemove{LEADING_CRLF_CHAR_SIZE},
                        m_boundaryFound{false},
                        m_lastSuccessIndex{0} {
                    m_multipartReader.onPartBegin = HTTP2MimeResponseDecoder::partBeginCallback;
                    m_multipartReader.onPartData = HTTP2MimeResponseDecoder::partDataCallback;
                    m_multipartReader.onPartEnd = HTTP2MimeResponseDecoder::partEndCallback;
                    m_multipartReader.userData = this;
                    ACSDK_DEBUG9(LX(__func__));
                }
                bool HTTP2MimeResponseDecoder::onReceiveResponseCode(long responseCode) {
                    ACSDK_DEBUG9(LX(__func__).d("responseCode", responseCode));
                    m_responseCode = responseCode;
                    if (!m_sink) {
                        ACSDK_WARN(LX("onReceiveResponseCodeIgnored").d("reason", "nullSink"));
                        return false;
                    }
                    return m_sink->onReceiveResponseCode(m_responseCode);
                }
                bool HTTP2MimeResponseDecoder::onReceiveHeaderLine(const std::string& line) {
                    ACSDK_DEBUG9(LX(__func__).d("line", line));
                    if (!m_sink) {
                        ACSDK_WARN(LX("onReceiveHeaderLineIgnored").d("reason", "nullSink"));
                        return false;
                    }
                    if (!m_boundaryFound) {
                        if (line.find(BOUNDARY_PREFIX) != std::string::npos) {
                            std::string boundary{line.substr(line.find(BOUNDARY_PREFIX))};
                            boundary = boundary.substr(BOUNDARY_PREFIX_SIZE, boundary.find(BOUNDARY_DELIMITER) - BOUNDARY_PREFIX_SIZE);
                            m_multipartReader.setBoundary(boundary);
                            m_boundaryFound = true;
                        }
                    }
                    return m_sink->onReceiveHeaderLine(line);
                }
                void HTTP2MimeResponseDecoder::partBeginCallback(const MultipartHeaders& headers, void* userData) {
                    HTTP2MimeResponseDecoder* decoder = static_cast<HTTP2MimeResponseDecoder*>(userData);
                    if (!decoder) {
                        ACSDK_ERROR(LX("partBeginCallbackFailed").d("reason", "nullDecoder"));
                        return;
                    }
                    switch (decoder->m_lastStatus) {
                        case HTTP2ReceiveDataStatus::SUCCESS:
                            if (!decoder->m_sink->onBeginMimePart(headers)) {
                                decoder->m_lastStatus = HTTP2ReceiveDataStatus::ABORT;
                            }
                        case HTTP2ReceiveDataStatus::PAUSE: break;
                        case HTTP2ReceiveDataStatus::ABORT: break;
                    }
                }
                void HTTP2MimeResponseDecoder::partDataCallback(const char* buffer, size_t size, void* userData) {
                    HTTP2MimeResponseDecoder* decoder = static_cast<HTTP2MimeResponseDecoder*>(userData);
                    if (!decoder) {
                        ACSDK_ERROR(LX("partDataCallbackFailed").d("reason", "nullDecoder"));
                        return;
                    }
                    if (!buffer) {
                        ACSDK_ERROR(LX("partDataCallbackFailed").d("reason", "nullBuffer"));
                        return;
                    }
                    decoder->m_index++;
                    if (decoder->m_lastStatus == HTTP2ReceiveDataStatus::PAUSE &&
                        decoder->m_index == (decoder->m_lastSuccessIndex + 1)) {
                        decoder->m_lastStatus = HTTP2ReceiveDataStatus::SUCCESS;
                    }
                    if (decoder->m_lastStatus == HTTP2ReceiveDataStatus::SUCCESS) {
                        decoder->m_lastStatus = decoder->m_sink->onReceiveMimeData(buffer, size);
                        if (decoder->m_lastStatus == HTTP2ReceiveDataStatus::SUCCESS) {
                            decoder->m_lastSuccessIndex = decoder->m_index;
                        }
                    }
                }
                void HTTP2MimeResponseDecoder::partEndCallback(void* userData) {
                    HTTP2MimeResponseDecoder* decoder = static_cast<HTTP2MimeResponseDecoder*>(userData);
                    if (!decoder) {
                        ACSDK_ERROR(LX("partEndCallbackFailed").d("reason", "nullDecoder"));
                        return;
                    }
                    if ((decoder->m_lastStatus == HTTP2ReceiveDataStatus::SUCCESS) && !decoder->m_sink->onEndMimePart()) {
                        decoder->m_lastStatus = HTTP2ReceiveDataStatus::ABORT;
                    }
                }
                HTTP2ReceiveDataStatus HTTP2MimeResponseDecoder::onReceiveData(const char* bytes, size_t size) {
                    ACSDK_DEBUG9(LX(__func__).d("size", size));
                    if (!bytes) {
                        ACSDK_ERROR(LX("onReceivedDataFailed").d("reason", "nullBytes"));
                        return HTTP2ReceiveDataStatus::ABORT;
                    }
                    if (m_lastStatus != HTTP2ReceiveDataStatus::ABORT) {
                        if (m_responseCode != HTTPResponseCode::SUCCESS_OK) {
                            if (m_sink) return m_sink->onReceiveNonMimeData(bytes, size);
                            else return HTTP2ReceiveDataStatus::ABORT;
                        }
                        auto readerCheckpoint = m_multipartReader;
                        auto oldLeadingCRLFCharsLeftToRemove = m_leadingCRLFCharsLeftToRemove;
                        if (!m_boundaryFound) return HTTP2ReceiveDataStatus::ABORT;
                        if (m_leadingCRLFCharsLeftToRemove > 0) {
                            size_t posInCRLF = LEADING_CRLF_CHAR_SIZE - m_leadingCRLFCharsLeftToRemove;
                            while (m_leadingCRLFCharsLeftToRemove > 0 && size > 0) {
                                if (*bytes == CRLF_SEQUENCE[posInCRLF]) {
                                    bytes++;
                                    size--;
                                    m_leadingCRLFCharsLeftToRemove--;
                                    posInCRLF++;
                                } else {
                                    if (1 == m_leadingCRLFCharsLeftToRemove) return HTTP2ReceiveDataStatus::ABORT;
                                    m_leadingCRLFCharsLeftToRemove = 0;
                                    break;
                                }
                            }
                            if (0 == size) return HTTP2ReceiveDataStatus::SUCCESS;
                        }
                        m_index = 0;
                        m_multipartReader.feed(bytes, size);
                        if (m_multipartReader.hasError()) {
                            ACSDK_ERROR(LX("onReceiveDataFailed")
                                            .d("reason", "mimeParseError")
                                            .d("error", m_multipartReader.getErrorMessage()));
                            m_lastStatus = HTTP2ReceiveDataStatus::ABORT;
                        }
                        switch (m_lastStatus) {
                            case HTTP2ReceiveDataStatus::SUCCESS: m_lastSuccessIndex = 0; break;
                            case HTTP2ReceiveDataStatus::PAUSE:
                                m_multipartReader = readerCheckpoint;
                                m_leadingCRLFCharsLeftToRemove = oldLeadingCRLFCharsLeftToRemove;
                                break;
                            case HTTP2ReceiveDataStatus::ABORT: break;
                        }
                    }
                    return m_lastStatus;
                }
                void HTTP2MimeResponseDecoder::onResponseFinished(HTTP2ResponseFinishedStatus status) {
                    ACSDK_DEBUG9(LX(__func__).d("status", status));
                    if (!m_sink) {
                        ACSDK_WARN(LX("onResponseFinishedIgnored").d("reason", "nullSink"));
                        return;
                    }
                    m_sink->onResponseFinished(status);
                }
            }
        }
    }
}
