#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LOGGER_THREADMONIKER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LOGGER_THREADMONIKER_H_

#include <iomanip>
#include <string>
#include <sstream>
#include <thread>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace logger {
                class ThreadMoniker {
                    public:
                        static inline std::string getThisThreadMoniker();
                        static std::string generateMoniker();
                        static inline void setThisThreadMoniker(const std::string& moniker);
                        ~ThreadMoniker() { m_moniker.clear(); }
                    private:
                        ThreadMoniker(const std::string& moniker = std::string());
                        static inline const ThreadMoniker& getMonikerObject(const std::string& moniker = std::string());
                        static const ThreadMoniker& getMonikerObjectFromMap(const std::string& moniker = std::string());
                        std::string m_moniker;
                };
            }
        }
    }
}
#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_LOGGER_THREADMONIKER_H_
