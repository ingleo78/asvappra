#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LOGGER_LEVEL_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LOGGER_LEVEL_H_

#include <iostream>
#include <string>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace logger {
                enum class Level {
                    DEBUG9,
                    DEBUG8,
                    DEBUG7,
                    DEBUG6,
                    DEBUG5,
                    DEBUG4,
                    DEBUG3,
                    DEBUG2,
                    DEBUG1,
                    DEBUG0,
                    INFO,
                    WARN,
                    ERROR,
                    CRITICAL,
                    NONE,
                    UNKNOWN
                };
                std::string convertLevelToName(Level level);
                char convertLevelToChar(Level level);
                Level convertNameToLevel(const std::string& name);
                inline std::ostream& operator<<(std::ostream& stream, Level level) {
                    stream << convertLevelToName(level);
                    return stream;
                }
            }
        }
    }
}
#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LOGGER_LEVEL_H_
