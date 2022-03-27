#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP_HTTPRESPONSECODE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP_HTTPRESPONSECODE_H_

#include <iostream>
#include <string>
#include <logger/LoggerUtils.h>
#include <logger/LogEntry.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace http {
                enum HTTPResponseCode {
                    HTTP_RESPONSE_CODE_UNDEFINED = 0,
                    SUCCESS_OK = 200,
                    SUCCESS_CREATED = 201,
                    SUCCESS_ACCEPTED = 202,
                    SUCCESS_NO_CONTENT = 204,
                    SUCCESS_PARTIAL_CONTENT = 206,
                    REDIRECTION_MULTIPLE_CHOICES = 300,
                    REDIRECTION_MOVED_PERMANENTLY = 301,
                    REDIRECTION_FOUND = 302,
                    REDIRECTION_SEE_ANOTHER = 303,
                    REDIRECTION_TEMPORARY_REDIRECT = 307,
                    REDIRECTION_PERMANENT_REDIRECT = 308,
                    CLIENT_ERROR_BAD_REQUEST = 400,
                    CLIENT_ERROR_FORBIDDEN = 403,
                    SERVER_ERROR_INTERNAL = 500,
                    SERVER_ERROR_NOT_IMPLEMENTED = 501,
                    SUCCESS_START_CODE = SUCCESS_OK,
                    SUCCESS_END_CODE = 299,
                    REDIRECTION_START_CODE = REDIRECTION_MULTIPLE_CHOICES,
                    REDIRECTION_END_CODE = REDIRECTION_PERMANENT_REDIRECT
                };
                inline bool isStatusCodeSuccess(HTTPResponseCode code) {
                    return (HTTPResponseCode::SUCCESS_START_CODE <= code) && (code <= HTTPResponseCode::SUCCESS_END_CODE);
                }
                inline bool isRedirect(HTTPResponseCode code) {
                    return HTTPResponseCode::REDIRECTION_MULTIPLE_CHOICES == code ||
                           HTTPResponseCode::REDIRECTION_MOVED_PERMANENTLY == code || HTTPResponseCode::REDIRECTION_FOUND == code ||
                           HTTPResponseCode::REDIRECTION_SEE_ANOTHER == code ||
                           HTTPResponseCode::REDIRECTION_TEMPORARY_REDIRECT == code ||
                           HTTPResponseCode::REDIRECTION_PERMANENT_REDIRECT == code;
                }
                inline HTTPResponseCode intToHTTPResponseCode(int code) {
                    switch (code) {
                        case 200: return HTTPResponseCode::SUCCESS_OK;
                        case 201: return HTTPResponseCode::SUCCESS_CREATED;
                        case 202: return HTTPResponseCode::SUCCESS_ACCEPTED;
                        case 204: return HTTPResponseCode::SUCCESS_NO_CONTENT;
                        case 206: return HTTPResponseCode::SUCCESS_PARTIAL_CONTENT;
                        case 300: return HTTPResponseCode::REDIRECTION_MULTIPLE_CHOICES;
                        case 301: return HTTPResponseCode::REDIRECTION_MOVED_PERMANENTLY;
                        case 302: return HTTPResponseCode::REDIRECTION_FOUND;
                        case 303: return HTTPResponseCode::REDIRECTION_SEE_ANOTHER;
                        case 307: return HTTPResponseCode::REDIRECTION_TEMPORARY_REDIRECT;
                        case 308: return HTTPResponseCode::REDIRECTION_PERMANENT_REDIRECT;
                        case 400: return HTTPResponseCode::CLIENT_ERROR_BAD_REQUEST;
                        case 403: return HTTPResponseCode::CLIENT_ERROR_FORBIDDEN;
                        case 500: return HTTPResponseCode::SERVER_ERROR_INTERNAL;
                    }
                    logger::acsdkError(logger::LogEntry("HttpResponseCodes", __func__).d("code", code).m("Unknown HTTP response code."));
                    return HTTPResponseCode::HTTP_RESPONSE_CODE_UNDEFINED;
                }
                inline int responseCodeToInt(HTTPResponseCode responseCode) {
                    return static_cast<std::underlying_type<HTTPResponseCode>::type>(responseCode);
                }
                inline std::string_view responseCodeToString(HTTPResponseCode responseCode) {
                    switch (responseCode) {
                        case HTTPResponseCode::HTTP_RESPONSE_CODE_UNDEFINED: return "HTTP_RESPONSE_CODE_UNDEFINED";
                        case HTTPResponseCode::SUCCESS_OK: return "SUCCESS_OK";
                        case HTTPResponseCode::SUCCESS_CREATED: return "SUCCESS_CREATED";
                        case HTTPResponseCode::SUCCESS_ACCEPTED: return "SUCCESS_ACCEPTED";
                        case HTTPResponseCode::SUCCESS_NO_CONTENT: return "SUCCESS_NO_CONTENT";
                        case HTTPResponseCode::SUCCESS_PARTIAL_CONTENT: return "SUCCESS_PARTIAL_CONTENT";
                        case HTTPResponseCode::SUCCESS_END_CODE: return "SUCCESS_END_CODE";
                        case HTTPResponseCode::REDIRECTION_MULTIPLE_CHOICES: return "REDIRECTION_MULTIPLE_CHOICES";
                        case HTTPResponseCode::REDIRECTION_MOVED_PERMANENTLY: return "REDIRECTION_MOVED_PERMANENTLY";
                        case HTTPResponseCode::REDIRECTION_FOUND: return "REDIRECTION_FOUND";
                        case HTTPResponseCode::REDIRECTION_SEE_ANOTHER: return "REDIRECTION_SEE_ANOTHER";
                        case HTTPResponseCode::REDIRECTION_TEMPORARY_REDIRECT: return "REDIRECTION_TEMPORARY_REDIRECT";
                        case HTTPResponseCode::REDIRECTION_PERMANENT_REDIRECT: return "REDIRECTION_PERMANENT_REDIRECT";
                        case HTTPResponseCode::CLIENT_ERROR_BAD_REQUEST: return "CLIENT_ERROR_BAD_REQUEST";
                        case HTTPResponseCode::CLIENT_ERROR_FORBIDDEN: return "CLIENT_ERROR_FORBIDDEN";
                        case HTTPResponseCode::SERVER_ERROR_INTERNAL: return "SERVER_ERROR_INTERNAL";
                        case HTTPResponseCode::SERVER_ERROR_NOT_IMPLEMENTED: return "SERVER_ERROR_NOT_IMPLEMENTED";
                    }
                    logger::acsdkError(logger::LogEntry("HttpResponseCodes", __func__)
                                           .d("longValue", static_cast<long>(responseCode))
                                           .m("Unknown HTTP response code."));
                    return "";
                }
                inline std::ostream& operator<<(std::ostream& os, const HTTPResponseCode& code) {
                    os << responseCodeToString(code);
                    return os;
                }
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_HTTP_HTTPRESPONSECODE_H_
