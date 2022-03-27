#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AUTHOBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AUTHOBSERVERINTERFACE_H_

#include <ostream>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            class AuthObserverInterface {
            public:
                enum class State {
                    UNINITIALIZED,
                    REFRESHED,
                    EXPIRED,
                    UNRECOVERABLE_ERROR
                };
                enum class Error {
                    SUCCESS,
                    UNKNOWN_ERROR,
                    AUTHORIZATION_FAILED,
                    UNAUTHORIZED_CLIENT,
                    SERVER_ERROR,
                    INVALID_REQUEST,
                    INVALID_VALUE,
                    AUTHORIZATION_EXPIRED,
                    UNSUPPORTED_GRANT_TYPE,
                    INVALID_CODE_PAIR,
                    AUTHORIZATION_PENDING,
                    SLOW_DOWN,
                    INTERNAL_ERROR,
                    INVALID_CBL_CLIENT_ID
                };
                virtual ~AuthObserverInterface() = default;
                virtual void onAuthStateChange(State newState, Error error) = 0;
            };
            inline std::ostream& operator<<(std::ostream& stream, const AuthObserverInterface::State& state) {
                switch (state) {
                    case AuthObserverInterface::State::UNINITIALIZED: return stream << "UNINITIALIZED";
                    case AuthObserverInterface::State::REFRESHED: return stream << "REFRESHED";
                    case AuthObserverInterface::State::EXPIRED: return stream << "EXPIRED";
                    case AuthObserverInterface::State::UNRECOVERABLE_ERROR: return stream << "UNRECOVERABLE_ERROR";
                }
                return stream << "Unknown AuthObserverInterface::State!: " << state;
            }
            inline std::ostream& operator<<(std::ostream& stream, const AuthObserverInterface::Error& error) {
                switch (error) {
                    case AuthObserverInterface::Error::SUCCESS: return stream << "SUCCESS";
                    case AuthObserverInterface::Error::UNKNOWN_ERROR: return stream << "UNKNOWN_ERROR";
                    case AuthObserverInterface::Error::AUTHORIZATION_FAILED: return stream << "AUTHORIZATION_FAILED";
                    case AuthObserverInterface::Error::UNAUTHORIZED_CLIENT: return stream << "UNAUTHORIZED_CLIENT";
                    case AuthObserverInterface::Error::SERVER_ERROR: return stream << "SERVER_ERROR";
                    case AuthObserverInterface::Error::INVALID_REQUEST: return stream << "INVALID_REQUEST";
                    case AuthObserverInterface::Error::INVALID_VALUE: return stream << "INVALID_VALUE";
                    case AuthObserverInterface::Error::AUTHORIZATION_EXPIRED: return stream << "AUTHORIZATION_EXPIRED";
                    case AuthObserverInterface::Error::UNSUPPORTED_GRANT_TYPE: return stream << "UNSUPPORTED_GRANT_TYPE";
                    case AuthObserverInterface::Error::INVALID_CODE_PAIR: return stream << "INVALID_CODE_PAIR";
                    case AuthObserverInterface::Error::AUTHORIZATION_PENDING: return stream << "AUTHORIZATION_PENDING";
                    case AuthObserverInterface::Error::SLOW_DOWN: return stream << "SLOW_DOWN";
                    case AuthObserverInterface::Error::INTERNAL_ERROR: return stream << "INTERNAL_ERROR";
                    case AuthObserverInterface::Error::INVALID_CBL_CLIENT_ID: return stream << "INVALID_CBL_CLIENT_ID";
                }
                return stream << "Unknown AuthObserverInterface::Error!: " << error;
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_AUTHOBSERVERINTERFACE_H_
