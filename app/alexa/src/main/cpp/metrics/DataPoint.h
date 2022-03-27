#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_METRICS_DATAPOINT_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_METRICS_DATAPOINT_H_

#include <string>
#include "DataType.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace metrics {
                class DataPoint {
                public:
                    DataPoint();
                    DataPoint(const std::string& name, const std::string& value, DataType dataType);
                    std::string getName() const;
                    std::string getValue() const;
                    DataType getDataType() const;
                    bool isValid() const;
                private:
                    const std::string m_name;
                    const std::string m_value;
                    const DataType m_dataType;
                };
            }
        }
    }
}
#endif