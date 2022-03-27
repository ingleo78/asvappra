#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LOGGER_LOGENTRYSTREAM_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LOGGER_LOGENTRYSTREAM_H_

#include <ostream>
#include "LogEntryBuffer.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace logger {
                class LogEntryStream : private LogEntryBuffer , public std::ostream {
                public:
                    LogEntryStream();
                    const char* c_str() const;
                };
            }
        }
    }
}
#endif