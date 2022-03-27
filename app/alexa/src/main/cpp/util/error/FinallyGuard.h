#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_ERROR_FINALLYGUARD_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_ERROR_FINALLYGUARD_H_

#include <functional>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace error {
                using namespace std;
                class FinallyGuard {
                public:
                    FinallyGuard(const function<void()>& finallyFunction);
                    ~FinallyGuard();
                private:
                    function<void()>& m_function;
                };
                inline FinallyGuard::FinallyGuard(const function<void()>& finallyFunction) : m_function{(function<void()>&)finallyFunction} {}
                inline FinallyGuard::~FinallyGuard() {
                    if ((function<void()>&)m_function) m_function();
                }
            }
        }
    }
}
#endif