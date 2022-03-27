#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_ERROR_RESULT_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_ERROR_RESULT_H_

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace error {
                template <typename TStatus, typename TValue> class Result {
                public:
                    inline Result(TStatus status, TValue value);
                    inline Result(TStatus status);
                    inline TStatus status();
                    inline TValue& value();
                private:
                    TStatus m_status;
                    TValue m_value;
                };
                template <typename TStatus, typename TValue> Result<TStatus, TValue>::Result(TStatus status, TValue value) : m_status{status}, m_value(value) {}
                template <typename TStatus, typename TValue> Result<TStatus, TValue>::Result(TStatus status) : m_status{status} {}
                template <typename TStatus, typename TValue> TValue& Result<TStatus, TValue>::value() {
                    return m_value;
                }
                template <typename TStatus, typename TValue> TStatus Result<TStatus, TValue>::status() {
                    return m_status;
                }
            }
        }
    }
}
#endif