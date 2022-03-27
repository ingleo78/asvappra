#include <limits>
#include "DataPointCounterBuilder.h"
#include "logger/LogEntry.h"
#include "logger/Logger.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace metrics {
                static const std::string TAG("DataPointCounterBuilder");
                #define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)
                DataPointCounterBuilder::DataPointCounterBuilder() : m_value{0} {}
                DataPointCounterBuilder& DataPointCounterBuilder::setName(const std::string& name) {
                    m_name = name;
                    return *this;
                }
                DataPointCounterBuilder& DataPointCounterBuilder::increment(uint64_t toAdd) {
                    if (m_value < (std::numeric_limits<uint64_t>::max() - toAdd)) {
                        m_value += toAdd;
                    } else {
                        ACSDK_WARN(LX("incrementWarning").d("reason", "counterValueOverflow"));
                        m_value = std::numeric_limits<uint64_t>::max();
                    }

                    return *this;
                }
                DataPoint DataPointCounterBuilder::build() {
                    return DataPoint{m_name, std::to_string(m_value), DataType::COUNTER};
                }
            }
        }
    }
}