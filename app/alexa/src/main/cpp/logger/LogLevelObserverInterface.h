#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LOGGER_LOGLEVELOBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LOGGER_LOGLEVELOBSERVERINTERFACE_H_

#include "Level.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace logger {
                class LogLevelObserverInterface {
                    public:
                        virtual void onLogLevelChanged(Level level) = 0;
                        virtual ~LogLevelObserverInterface() = default;
                };
            }
        }
    }
}
#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LOGGER_LOGLEVELOBSERVERINTERFACE_H_
