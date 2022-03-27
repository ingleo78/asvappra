#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LOGGER_SINKOBSERVERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LOGGER_SINKOBSERVERINTERFACE_H_

#include <memory>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace logger {
                class Logger;
                class SinkObserverInterface {
                    public:
                        virtual void onSinkChanged(const std::shared_ptr<Logger>& sink) = 0;
                        virtual ~SinkObserverInterface() = default;
                };
            }
        }
    }
}
#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LOGGER_SINKOBSERVERINTERFACE_H_
