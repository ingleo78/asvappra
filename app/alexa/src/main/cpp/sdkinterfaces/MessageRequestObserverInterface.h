#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_MESSAGEREQUESTOBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_MESSAGEREQUESTOBSERVERINTERFACE_H_

#include <iostream>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class MessageRequestObserverInterface {
            public:
                enum class Status {
                    PENDING,
                    SUCCESS,
                    SUCCESS_ACCEPTED,
                    SUCCESS_NO_CONTENT,
                    NOT_CONNECTED,
                    NOT_SYNCHRONIZED,
                    TIMEDOUT,
                    PROTOCOL_ERROR,
                    INTERNAL_ERROR,
                    SERVER_INTERNAL_ERROR_V2,
                    REFUSED,
                    CANCELED,
                    THROTTLED,
                    INVALID_AUTH,
                    BAD_REQUEST,
                    SERVER_OTHER_ERROR
                };
                virtual ~MessageRequestObserverInterface() = default;
                virtual void onSendCompleted(MessageRequestObserverInterface::Status status);
                virtual void onExceptionReceived(const std::string& exceptionMessage);
            };
            inline std::ostream& operator<<(std::ostream& stream, MessageRequestObserverInterface::Status status) {
                switch(status) {
                    case MessageRequestObserverInterface::Status::PENDING: return stream << "PENDING";
                    case MessageRequestObserverInterface::Status::SUCCESS: return stream << "SUCCESS";
                    case MessageRequestObserverInterface::Status::SUCCESS_ACCEPTED: return stream << "SUCCESS_ACCEPTED";
                    case MessageRequestObserverInterface::Status::SUCCESS_NO_CONTENT: return stream << "SUCCESS_NO_CONTENT";
                    case MessageRequestObserverInterface::Status::NOT_CONNECTED: return stream << "NOT_CONNECTED";
                    case MessageRequestObserverInterface::Status::NOT_SYNCHRONIZED: return stream << "NOT_SYNCHRONIZED";
                    case MessageRequestObserverInterface::Status::TIMEDOUT: return stream << "TIMEDOUT";
                    case MessageRequestObserverInterface::Status::PROTOCOL_ERROR: return stream << "PROTOCOL_ERROR";
                    case MessageRequestObserverInterface::Status::INTERNAL_ERROR: return stream << "INTERNAL_ERROR";
                    case MessageRequestObserverInterface::Status::SERVER_INTERNAL_ERROR_V2: return stream << "SERVER_INTERNAL_ERROR_V2";
                    case MessageRequestObserverInterface::Status::REFUSED: return stream << "REFUSED";
                    case MessageRequestObserverInterface::Status::CANCELED: return stream << "CANCELED";
                    case MessageRequestObserverInterface::Status::THROTTLED: return stream << "THROTTLED";
                    case MessageRequestObserverInterface::Status::INVALID_AUTH: return stream << "INVALID_AUTH";
                    case MessageRequestObserverInterface::Status::BAD_REQUEST: return stream << "CLIENT_ERROR_BAD_REQUEST";
                    case MessageRequestObserverInterface::Status::SERVER_OTHER_ERROR: return stream << "SERVER_OTHER_ERROR";
                }
                return stream << "Unknown MessageRequestObserverInterface::Status";
            }
        }
    }
}
#endif