#include "DataPointDurationBuilder.h"
#include "logger/LogEntry.h"
#include "logger/Logger.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace metrics {
                static const std::string TAG("DataPointDurationBuilder");
                #define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)
                DataPointDurationBuilder::DataPointDurationBuilder() : DataPointDurationBuilder(std::chrono::milliseconds(0)) {}
                DataPointDurationBuilder::DataPointDurationBuilder(std::chrono::milliseconds duration) :
                        m_startTime{std::chrono::steady_clock::now()},
                        m_isStartDurationTimerRunning{false} {
                    std::chrono::milliseconds zeroMilliseconds(0);
                    if (duration < zeroMilliseconds) {
                        ACSDK_WARN(LX("dataPointDurationBuilderConstructorWarning").d("reason", "negativeDurationSetToZero"));
                        duration = zeroMilliseconds;
                    }
                    m_duration = duration;
                }
                DataPointDurationBuilder& DataPointDurationBuilder::setName(const std::string& name) {
                    m_name = name;
                    return *this;
                }
                DataPointDurationBuilder& DataPointDurationBuilder::startDurationTimer() {
                    m_startTime = std::chrono::steady_clock::now();
                    m_isStartDurationTimerRunning = true;
                    return *this;
                }
                DataPointDurationBuilder& DataPointDurationBuilder::stopDurationTimer() {
                    if (!m_isStartDurationTimerRunning) {
                        ACSDK_WARN(LX("stopDurationTimerFailed").d("reason", "startDurationTimerNotRunning"));
                        return *this;
                    }

                    std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now();
                    m_duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - m_startTime);
                    m_isStartDurationTimerRunning = false;
                    return *this;
                }
                DataPoint DataPointDurationBuilder::build() {
                    return DataPoint{m_name, std::to_string(m_duration.count()), DataType::DURATION};
                }
            }
        }
    }
}