#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_METRICS_METRICEVENTBUILDER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_METRICS_METRICEVENTBUILDER_H_

#include <unordered_map>
#include "DataPoint.h"
#include "MetricEvent.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace metrics {
                class MetricEventBuilder {
                    public:
                        MetricEventBuilder();
                        MetricEventBuilder& setActivityName(const std::string& activityName);
                        MetricEventBuilder& setPriority(Priority priority);
                        MetricEventBuilder& addDataPoint(const DataPoint& dataPoint);
                        MetricEventBuilder& addDataPoints(const std::vector<DataPoint>& dataPoints);
                        MetricEventBuilder& removeDataPoint(const DataPoint& dataPoint);
                        MetricEventBuilder& removeDataPoint(const std::string& name, DataType dataType);
                        MetricEventBuilder& removeDataPoints();
                        void clear();
                        std::shared_ptr<MetricEvent> build();
                        static std::string generateKey(const std::string& name, DataType dataType);
                    private:
                        MetricEventBuilder& removeDataPoint(const std::string& key);
                        std::string m_activityName;
                        Priority m_priority;
                        std::unordered_map<std::string, DataPoint> m_dataPoints;
                    };
            }
        }
    }
}
#endif