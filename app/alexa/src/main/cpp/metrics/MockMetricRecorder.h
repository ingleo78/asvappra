#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_TEST_AVSCOMMON_UTILS_METRICS_MOCKMETRICRECORDER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_TEST_AVSCOMMON_UTILS_METRICS_MOCKMETRICRECORDER_H_

#include <metrics/MetricRecorderInterface.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace metrics {
                namespace test {
                    using namespace std;
                    using namespace avsCommon;
                    using namespace utils;
                    using namespace metrics;
                    class MockMetricRecorder : public MetricRecorderInterface {
                    public:
                        MOCK_METHOD1(recordMetric, void(shared_ptr<MetricEvent>));
                    };
                }
            }
        }
    }
}
#endif