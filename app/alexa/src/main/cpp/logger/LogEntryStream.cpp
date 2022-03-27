#include "logger/LogEntryStream.h"
#include "util/RetryTimer.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace logger {
                LogEntryStream::LogEntryStream() : LogEntryBuffer{}, std::ostream{this} {}
                const char* LogEntryStream::LogEntryStream::c_str() const { return LogEntryBuffer::c_str(); }
            }
        }
    }
}
