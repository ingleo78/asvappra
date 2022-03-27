#include "Metrics.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            using namespace std;
            using namespace avs;
            using namespace utils::logger;
            const string Metrics::locationToString(Metrics::Location location) {
                switch (location) {
                    case ADSL_ENQUEUE: return "ADSL Enqueue";
                    case ADSL_DEQUEUE: return "ADSL Dequeue";
                    case SPEECH_SYNTHESIZER_RECEIVE: return "SpeechSynthesizer Receive";
                    case AIP_RECEIVE: return "AIP Receive";
                    case AIP_SEND: return "AIP Send";
                    case BUILDING_MESSAGE: return "Building Message";
                }
                return "unknown";
            }
            LogEntry& Metrics::d(LogEntry& logEntry, const shared_ptr<AVSMessage> msg, Metrics::Location location) {
                return d(logEntry, msg->getName(), msg->getMessageId(), msg->getDialogRequestId(), location);
            }
            LogEntry& Metrics::d(LogEntry& logEntry, const string& name, const string& messageId, const string& dialogRequestId, Location location) {
                logEntry.d("Location", locationToString(location)).d("NAME", name);
                if (!messageId.empty() || !dialogRequestId.empty()) {
                    logEntry.d("MessageID", messageId).d("DialogRequestID", dialogRequestId);
                };
                return logEntry;
            }
        }
    }
}
