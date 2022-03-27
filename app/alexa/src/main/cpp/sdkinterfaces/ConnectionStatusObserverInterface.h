#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_CONNECTIONSTATUSOBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_CONNECTIONSTATUSOBSERVERINTERFACE_H_

#include <iostream>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class ConnectionStatusObserverInterface {
            public:
                enum class Status {
                    DISCONNECTED,
                    PENDING,
                    CONNECTED
                };
                enum class ChangedReason {
                    NONE,
                    SUCCESS,
                    UNRECOVERABLE_ERROR,
                    ACL_CLIENT_REQUEST,
                    ACL_DISABLED,
                    DNS_TIMEDOUT,
                    CONNECTION_TIMEDOUT,
                    CONNECTION_THROTTLED,
                    INVALID_AUTH,
                    PING_TIMEDOUT,
                    WRITE_TIMEDOUT,
                    READ_TIMEDOUT,
                    FAILURE_PROTOCOL_ERROR,
                    INTERNAL_ERROR,
                    SERVER_INTERNAL_ERROR,
                    SERVER_SIDE_DISCONNECT,
                    SERVER_ENDPOINT_CHANGED
                };
                virtual ~ConnectionStatusObserverInterface() = default;
                virtual void onConnectionStatusChanged(const Status status, const ChangedReason reason) = 0;
            };
            inline std::ostream& operator<<(std::ostream& stream, ConnectionStatusObserverInterface::Status status) {
                switch (status) {
                    case ConnectionStatusObserverInterface::Status::DISCONNECTED: stream << "DISCONNECTED"; break;
                    case ConnectionStatusObserverInterface::Status::PENDING: stream << "PENDING"; break;
                    case ConnectionStatusObserverInterface::Status::CONNECTED: stream << "CONNECTED"; break;
                }
                return stream;
            }
            inline std::ostream& operator<<(std::ostream& stream, ConnectionStatusObserverInterface::ChangedReason reason) {
                switch(reason) {
                    case ConnectionStatusObserverInterface::ChangedReason::NONE: stream << "NONE"; break;
                    case ConnectionStatusObserverInterface::ChangedReason::SUCCESS: stream << "SUCCESS"; break;
                    case ConnectionStatusObserverInterface::ChangedReason::UNRECOVERABLE_ERROR: stream << "UNRECOVERABLE_ERROR"; break;
                    case ConnectionStatusObserverInterface::ChangedReason::ACL_CLIENT_REQUEST: stream << "ACL_CLIENT_REQUEST"; break;
                    case ConnectionStatusObserverInterface::ChangedReason::ACL_DISABLED: stream << "ACL_DISABLED"; break;
                    case ConnectionStatusObserverInterface::ChangedReason::DNS_TIMEDOUT: stream << "DNS_TIMEDOUT"; break;
                    case ConnectionStatusObserverInterface::ChangedReason::CONNECTION_TIMEDOUT: stream << "CONNECTION_TIMEDOUT"; break;
                    case ConnectionStatusObserverInterface::ChangedReason::CONNECTION_THROTTLED: stream << "CONNECTION_THROTTLED"; break;
                    case ConnectionStatusObserverInterface::ChangedReason::INVALID_AUTH: stream << "INVALID_AUTH"; break;
                    case ConnectionStatusObserverInterface::ChangedReason::PING_TIMEDOUT: stream << "PING_TIMEDOUT"; break;
                    case ConnectionStatusObserverInterface::ChangedReason::WRITE_TIMEDOUT: stream << "WRITE_TIMEDOUT"; break;
                    case ConnectionStatusObserverInterface::ChangedReason::READ_TIMEDOUT: stream << "READ_TIMEDOUT"; break;
                    case ConnectionStatusObserverInterface::ChangedReason::FAILURE_PROTOCOL_ERROR: stream << "FAILURE_PROTOCOL_ERROR"; break;
                    case ConnectionStatusObserverInterface::ChangedReason::INTERNAL_ERROR: stream << "INTERNAL_ERROR"; break;
                    case ConnectionStatusObserverInterface::ChangedReason::SERVER_INTERNAL_ERROR: stream << "SERVER_INTERNAL_ERROR"; break;
                    case ConnectionStatusObserverInterface::ChangedReason::SERVER_SIDE_DISCONNECT: stream << "SERVER_SIDE_DISCONNECT"; break;
                    case ConnectionStatusObserverInterface::ChangedReason::SERVER_ENDPOINT_CHANGED: stream << "SERVER_ENDPOINT_CHANGED"; break;
                }
                return stream;
            }
        }
    }
}
#endif