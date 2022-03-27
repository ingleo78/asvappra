#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_REQUIRESSHUTDOWN_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_REQUIRESSHUTDOWN_H_

#include <atomic>
#include <memory>
#include <mutex>
#include <string>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            class RequiresShutdown {
            public:
                RequiresShutdown(const std::string& name);
                virtual ~RequiresShutdown();
                const std::string& name() const;
                void shutdown();
                bool isShutdown() const;
            protected:
                virtual void doShutdown();
            private:
                const std::string m_name;
                mutable std::mutex m_mutex;
                bool m_isShutdown;
            };
        }
    }
}
#endif