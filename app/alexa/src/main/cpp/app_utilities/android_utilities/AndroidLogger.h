#ifndef ALEXA_CLIENT_SDK_APPLICATIONUTILITIES_ANDROIDUTILITIES_INCLUDE_ANDROIDUTILITIES_ANDROIDLOGGER_H_
#define ALEXA_CLIENT_SDK_APPLICATIONUTILITIES_ANDROIDUTILITIES_INCLUDE_ANDROIDUTILITIES_ANDROIDLOGGER_H_

#include <string>
#include <logger/LogStringFormatter.h>
#include <logger/Logger.h>

namespace alexaClientSDK {
    namespace applicationUtilities {
        namespace androidUtilities {
            using namespace std;
            using namespace chrono;
            using namespace avsCommon::utils::logger;
            class AndroidLogger : public Logger {
            public:
                void emit(Level level, system_clock::time_point time, const char* threadMoniker, const char* text) override;
                AndroidLogger(Level level);
                AndroidLogger(const string& tag, Level level);
            private:
                string m_tag;
            };
        }
    }
}
#endif