#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_METRICS_DATAPOINTDURATIONBUILDER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_METRICS_DATAPOINTDURATIONBUILDER_H_

#include <chrono>
#include "DataPoint.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace metrics {
                class DataPointDurationBuilder {
                public:
                    DataPointDurationBuilder();
                    explicit DataPointDurationBuilder(std::chrono::milliseconds duration);
                    DataPointDurationBuilder& setName(const std::string& name);
                    DataPointDurationBuilder& startDurationTimer();
                    DataPointDurationBuilder& stopDurationTimer();
                    DataPoint build();
                private:
                    std::string m_name;
                    std::chrono::milliseconds m_duration;
                    std::chrono::steady_clock::time_point m_startTime;
                    bool m_isStartDurationTimerRunning;
                };
            }
        }
    }
}
#endif