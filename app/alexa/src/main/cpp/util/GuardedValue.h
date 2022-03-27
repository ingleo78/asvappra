#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_GUARDEDVALUE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_GUARDEDVALUE_H_

#include <mutex>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            template <typename ValueT> class GuardedValue {
            public:
                operator ValueT() const;
                ValueT operator=(const ValueT& value);
                GuardedValue(const ValueT& value);
            private:
                mutable std::mutex m_mutex;
                ValueT m_value;
            };
            template <typename ValueT> ValueT GuardedValue<ValueT>::operator=(const ValueT& value) {
                std::lock_guard<std::mutex> lock{m_mutex};
                m_value = value;
                return m_value;
            }
            template <typename ValueT> GuardedValue<ValueT>::operator ValueT() const {
                std::lock_guard<std::mutex> lock{m_mutex};
                return m_value;
            }
            template <typename ValueT> GuardedValue<ValueT>::GuardedValue(const ValueT& value) : m_value{value} {}
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_GUARDEDVALUE_H_
