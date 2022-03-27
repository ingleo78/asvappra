#include <metrics/DataPointCounterBuilder.h>
#include <metrics/DataPointStringBuilder.h>
#include <metrics/MetricEventBuilder.h>
#include <util/Metrics.h>
#include <logger/Logger.h>
#include "MessageInterpreter.h"

namespace alexaClientSDK {
    namespace adsl {
        using namespace std;
        using namespace avsCommon;
        using namespace avs;
        using namespace attachment;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace logger;
        using namespace metrics;
        static const string PARSE_COMPLETE("PARSE_COMPLETE");
        static const string PARSE_COMPLETE_ACTIVITY_NAME("MESSAGE_INTERPRETER-" + PARSE_COMPLETE);
        static const string TAG("MessageInterpreter");
        #define LX(event) LogEntry(TAG, event)
        MessageInterpreter::MessageInterpreter(shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender,
                                               shared_ptr<DirectiveSequencerInterface> directiveSequencer, shared_ptr<AttachmentManagerInterface> attachmentManager,
                                               shared_ptr<MetricRecorderInterface> metricRecorder) : m_exceptionEncounteredSender{exceptionEncounteredSender},
                                               m_directiveSequencer{directiveSequencer}, m_attachmentManager{attachmentManager}, m_metricRecorder{metricRecorder} {}
        void MessageInterpreter::receive(const string& contextId, const string& message) {
            auto createResult = AVSDirective::create(message, m_attachmentManager, contextId);
            shared_ptr<AVSDirective> avsDirective{move(createResult.first)};
            if (!avsDirective) {
                if (m_exceptionEncounteredSender) {
                    const string errorDescription = "Unable to parse Directive - JSON error:" + avsDirectiveParseStatusToString(createResult.second);
                    ACSDK_ERROR(LX("receiveFailed").m(errorDescription));
                    m_exceptionEncounteredSender->sendExceptionEncountered(message, ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED, errorDescription);
                } else { ACSDK_ERROR(LX("receiveFailed").m("unable to send AVS Exception due to nullptr sender.")); }
                return;
            }
            auto metricEvent = MetricEventBuilder{}.setActivityName(PARSE_COMPLETE_ACTIVITY_NAME)
                    .addDataPoint(DataPointCounterBuilder{}.setName(PARSE_COMPLETE).increment(1).build()).addDataPoint(DataPointStringBuilder{}
                    .setName("HTTP2_STREAM").setValue(avsDirective->getAttachmentContextId()).build())
                    .addDataPoint(DataPointStringBuilder{}.setName("DIRECTIVE_MESSAGE_ID").setValue(avsDirective->getMessageId()).build())
                    .build();
            if (metricEvent == nullptr) {
                ACSDK_ERROR(LX("Error creating metric."));
                return;
            }
            recordMetric(m_metricRecorder, metricEvent);
            if (avsDirective->getName() == "StopCapture" || avsDirective->getName() == "Speak")
                ACSDK_METRIC_MSG(TAG, avsDirective, Metrics::Location::ADSL_ENQUEUE);
            m_directiveSequencer->onDirective(avsDirective);
        }
    }
}