#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_METRICS_DATATYPE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_METRICS_DATATYPE_H_

#include <iostream>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace metrics {
                enum class DataType {
                    DURATION,
                    COUNTER,
                    STRING
                };
                inline std::ostream& operator<<(std::ostream& stream, const DataType& dataType) {
                    switch(dataType) {
                        case DataType::DURATION: stream << "DURATION"; break;
                        case DataType::COUNTER: stream << "COUNTER"; break;
                        case DataType::STRING: stream << "STRING"; break;
                    }
                    return stream;
                }
            }
        }
    }
}
#endif