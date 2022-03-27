#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_METRICS_PRIORITY_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_METRICS_PRIORITY_H_

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace metrics {
                enum class Priority {
                    NORMAL,
                    HIGH
                };
                inline std::ostream& operator<<(std::ostream& stream, const Priority& priority) {
                    switch (priority) {
                        case Priority::NORMAL: stream << "NORMAL"; break;
                        case Priority::HIGH: stream << "HIGH"; break;
                    }
                    return stream;
                }
            }
        }
    }
}
#endif