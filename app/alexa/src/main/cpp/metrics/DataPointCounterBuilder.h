#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_METRICS_DATAPOINTCOUNTERBUILDER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_METRICS_DATAPOINTCOUNTERBUILDER_H_

#include "DataPoint.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace metrics {
                class DataPointCounterBuilder {
                public:
                    DataPointCounterBuilder();
                    DataPointCounterBuilder& setName(const std::string& name);
                    DataPointCounterBuilder& increment(uint64_t toAdd);
                    DataPoint build();
                private:
                    std::string m_name;
                    uint64_t m_value;
                };
            }
        }
    }
}
#endif