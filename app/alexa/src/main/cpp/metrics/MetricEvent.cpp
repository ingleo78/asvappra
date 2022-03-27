#include "MetricEvent.h"
#include "logger/LogEntry.h"
#include "logger/Logger.h"
#include "MetricEventBuilder.h"
#include "util/Optional.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace metrics {
                using namespace std;
                using namespace chrono;
                static const std::string TAG("MetricEvent");
                #define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)
                MetricEvent::MetricEvent(const string& activityName, Priority priority, const unordered_map<string, DataPoint>& dataPoints,
                                         steady_clock::time_point timestamp) : m_activityName{activityName}, m_priority{priority},
                                         m_dataPoints{dataPoints}, m_timestamp{timestamp} {
                }
                string MetricEvent::getActivityName() const {
                    return m_activityName;
                }
                Priority MetricEvent::getPriority() const {
                    return m_priority;
                }
                Optional<DataPoint> MetricEvent::getDataPoint(const string& name, DataType dataType) const {
                    string key = MetricEventBuilder::generateKey(name, dataType);
                    if (m_dataPoints.find(key) == m_dataPoints.end()) {
                        ACSDK_WARN(LX("getDataPointWarning").d("reason", "dataPointDoesntExist"));
                        return Optional<DataPoint>{};
                    }
                    return Optional<DataPoint>{m_dataPoints.find(key)->second};
                }
                vector<DataPoint> MetricEvent::getDataPoints() const {
                    vector<DataPoint> dataPoints;
                    for (const auto& keyValuePair : m_dataPoints) {
                        dataPoints.push_back(keyValuePair.second);
                    }
                    return dataPoints;
                }
                system_clock::time_point MetricEvent::getTimestamp() const {
                    return system_clock::now() - duration_cast<system_clock::duration>(steady_clock::now() - m_timestamp);
                }
                steady_clock::time_point MetricEvent::getSteadyTimestamp() const {
                    return m_timestamp;
                }
            }
        }
    }
}