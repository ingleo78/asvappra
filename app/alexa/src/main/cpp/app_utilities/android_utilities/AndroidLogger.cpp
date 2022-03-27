#include <android/log.h>
#include "AndroidLogger.h"

static const char* TAG{"AlexaSampleApp"};

namespace alexaClientSDK {
    namespace applicationUtilities {
        namespace androidUtilities {
            using namespace std;
            using namespace chrono;
            using namespace avsCommon::utils::logger;
            static android_LogPriority convertToAndroidLevel(Level level) {
                switch (level) {
                    case Level::ERROR: return ANDROID_LOG_ERROR;
                    case Level::CRITICAL: return ANDROID_LOG_FATAL;
                    case Level::INFO: return ANDROID_LOG_INFO;
                    case Level::WARN: return ANDROID_LOG_WARN;
                    case Level::NONE: return ANDROID_LOG_SILENT;
                    case Level::DEBUG0: case Level::DEBUG1: case Level::DEBUG2: case Level::DEBUG3: case Level::DEBUG4: return ANDROID_LOG_DEBUG;
                    case Level::DEBUG5: case Level::DEBUG6: case Level::DEBUG7: case Level::DEBUG8: case Level::DEBUG9: return ANDROID_LOG_VERBOSE;
                    case Level::UNKNOWN: return ANDROID_LOG_UNKNOWN;
                }
            }
            AndroidLogger::AndroidLogger(Level level) : Logger{level}, m_tag{TAG} {}
            AndroidLogger::AndroidLogger(const string& tag, Level level) : Logger{level}, m_tag{tag} {}
            void AndroidLogger::emit(Level level, system_clock::time_point time, const char* threadMoniker, const char* text) {
                __android_log_print(convertToAndroidLevel(level), m_tag.c_str(), "[%s] %c %s", threadMoniker, convertLevelToChar(level), text);
            }
        }
    }
}