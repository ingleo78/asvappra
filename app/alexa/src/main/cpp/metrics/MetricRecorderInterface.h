#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_METRICS_METRICRECORDERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_METRICS_METRICRECORDERINTERFACE_H_

#include "MetricEvent.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace metrics {
                class MetricRecorderInterface {
                public:
                    virtual ~MetricRecorderInterface() = default;
                    virtual void recordMetric(std::shared_ptr<MetricEvent> metricEvent);
                };
                inline void recordMetric(const std::shared_ptr<MetricRecorderInterface>& recorder, std::shared_ptr<MetricEvent> metricEvent) {
                #ifdef ACSDK_ENABLE_METRICS_RECORDING
                    if (recorder) recorder->recordMetric(std::move(metricEvent));
                #endif
                }
            }
        }
    }
}
#endif