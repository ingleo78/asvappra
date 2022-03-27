#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_METRICS_METRICEVENT_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_METRICS_METRICEVENT_H_

#include <unordered_map>
#include <vector>
#include <chrono>
#include "DataPoint.h"
#include "Priority.h"
#include "util/Optional.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace metrics {
                using namespace std;
                using namespace chrono;
                class MetricEvent {
                public:
                    MetricEvent(const string& activityName, Priority priority, const unordered_map<string, DataPoint>& dataPoints,
                                steady_clock::time_point timestamp);
                    string getActivityName() const;
                    Priority getPriority() const;
                    Optional<DataPoint> getDataPoint(const string& name, DataType dataType) const;
                    vector<DataPoint> getDataPoints() const;
                    system_clock::time_point getTimestamp() const;
                    steady_clock::time_point getSteadyTimestamp() const;
                private:
                    const std::string m_activityName;
                    const Priority m_priority;
                    const unordered_map<std::string, DataPoint> m_dataPoints;
                    const steady_clock::time_point m_timestamp;
                };
            }
        }
    }
}
#endif