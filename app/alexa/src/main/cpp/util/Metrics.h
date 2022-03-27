#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_METRICS_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_METRICS_H_

#include <logger/LogEntry.h>
#include <avs/AVSMessage.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            using namespace std;
            using namespace avs;
            using namespace logger;
            const string METRICS_TAG = ":METRICS:";
            class Metrics {
            public:
                enum Location {
                    ADSL_ENQUEUE,
                    ADSL_DEQUEUE,
                    SPEECH_SYNTHESIZER_RECEIVE,
                    AIP_RECEIVE,
                    AIP_SEND,
                    BUILDING_MESSAGE
                };
                static LogEntry& d(LogEntry& logEntry, const string& name, const string& messageId, const string& dialogRequestId, Location location);
                static LogEntry& d(LogEntry& logEntry, const shared_ptr<AVSMessage> msg, Location location);
            private:
                static const string locationToString(Location location);
            };
        }
    }
}
#ifdef ACSDK_LATENCY_LOG_ENABLED
#define ACSDK_METRIC_WITH_nENTRY(entry) ACSDK_INFO(entry)
#define ACSDK_METRIC_IDS(TAG, name, messageId, dialogRequestId, location) \
    do { \
        alexaClientSDK::avsCommon::utils::logger::LogEntry logEntry(TAG, __func__ + alexaClientSDK::avsCommon::utils::METRICS_TAG); \
        alexaClientSDK::avsCommon::utils::Metrics::d(logEntry, name, messageId, dialogRequestId, location); \
        ACSDK_METRIC_WITH_ENTRY(logEntry); \
    } while(false);
#define ACSDK_METRIC_MSG(TAG, msg, location) \
    do { \
        alexaClientSDK::avsCommon::utils::logger::LogEntry logEntry(TAG, __func__ + alexaClientSDK::avsCommon::utils::METRICS_TAG); \
        alexaClientSDK::avsCommon::utils::Metrics::d(logEntry, msg, location); \
        ACSDK_METRIC_WITH_ENTRY(logEntry); \
    } while(false);
#else
#define ACSDK_METRIC_IDS(TAG, name, messageId, dialogRequestId, location)
#define ACSDK_METRIC_MSG(TAG, msg, location)
#endif
#endif