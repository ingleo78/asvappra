#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_ERROR_SUCCESSRESULT_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_ERROR_SUCCESSRESULT_H_

#include "Result.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace error {
                template <typename TValue> class SuccessResult : public utils::error::Result<bool, TValue> {
                public:
                    inline static SuccessResult<TValue> success(TValue value);
                    inline static SuccessResult<TValue> failure();
                    inline SuccessResult(bool succeeded, TValue value);
                    inline bool isSucceeded();
                protected:
                    explicit inline SuccessResult(bool succeeded);
                };
                template <typename TValue> SuccessResult<TValue>::SuccessResult(bool succeeded) : Result<bool, TValue>(succeeded) {}
                template <typename TValue> SuccessResult<TValue>::SuccessResult(bool succeeded, TValue value) : Result<bool, TValue>(succeeded, value) {}
                template <typename TValue> bool SuccessResult<TValue>::isSucceeded() {
                    return Result<bool, TValue>::status();
                }
                template <typename TValue> SuccessResult<TValue> SuccessResult<TValue>::failure() {
                    return SuccessResult(false);
                }
                template <typename TValue> SuccessResult<TValue> SuccessResult<TValue>::success(TValue value) {
                    return SuccessResult<TValue>(true, value);
                }
            }
        }
    }
}
#endif