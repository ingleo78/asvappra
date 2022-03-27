#include <sstream>
#include "metrics/DataPoint.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace metrics {
                DataPoint::DataPoint() : m_dataType{DataType::STRING} {}
                DataPoint::DataPoint(const std::string& name, const std::string& value, DataType dataType) : m_name{name}, m_value{value}, m_dataType{dataType} {}
                std::string DataPoint::getName() const {
                    return m_name;
                }
                std::string DataPoint::getValue() const {
                    return m_value;
                }
                DataType DataPoint::getDataType() const {
                    return m_dataType;
                }
                bool DataPoint::isValid() const {
                    return !m_name.empty() && !m_value.empty();
                }
            }
        }
    }
}