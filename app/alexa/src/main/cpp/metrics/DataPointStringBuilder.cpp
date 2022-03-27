#include "DataPointStringBuilder.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace metrics {
                DataPointStringBuilder& DataPointStringBuilder::setName(const std::string& name) {
                    m_name = name;
                    return *this;
                }
                DataPointStringBuilder& DataPointStringBuilder::setValue(const std::string& value) {
                    m_value = value;
                    return *this;
                }
                DataPoint DataPointStringBuilder::build() {
                    return DataPoint{m_name, m_value, DataType::STRING};
                }
            }
        }
    }
}