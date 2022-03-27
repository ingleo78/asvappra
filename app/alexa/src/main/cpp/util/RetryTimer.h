#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_RETRYTIMER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_RETRYTIMER_H_

#include <chrono>
#include <vector>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            class RetryTimer {
                public:
                    RetryTimer(const std::vector<int>& retryTable);
                    RetryTimer(const std::vector<int>& retryTable, int randomizationPercentage);
                    RetryTimer(const std::vector<int>& retryTable, int decreasePercentage, int increasePercentage);
                    ~RetryTimer() {}
                    std::chrono::milliseconds calculateTimeToRetry(int retryCount) const;
                private:
                    const std::vector<int> m_RetryTable;
                    const size_t m_RetrySize;
                    const int m_RetryDecreasePercentage;
                    const int m_RetryIncreasePercentage;
            };
        }
    }
}
#endif