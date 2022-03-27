#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_METRICS_DATAPOINTSTRINGBUILDER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_METRICS_DATAPOINTSTRINGBUILDER_H_

#include "DataPoint.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace metrics {
                class DataPointStringBuilder {
                public:
                    DataPointStringBuilder& setName(const std::string& name);
                    DataPointStringBuilder& setValue(const std::string& value);
                    DataPoint build();
                private:
                    std::string m_name;
                    std::string m_value;
                };
            }
        }
    }
}
#endif