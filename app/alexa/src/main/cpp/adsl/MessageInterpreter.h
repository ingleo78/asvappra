#ifndef ALEXA_CLIENT_SDK_ADSL_INCLUDE_ADSL_MESSAGEINTERPRETER_H_
#define ALEXA_CLIENT_SDK_ADSL_INCLUDE_ADSL_MESSAGEINTERPRETER_H_

#include <memory>
#include <sdkinterfaces/ExceptionEncounteredSenderInterface.h>
#include <sdkinterfaces/DirectiveSequencerInterface.h>
#include <sdkinterfaces/MessageObserverInterface.h>
#include <metrics/MetricRecorderInterface.h>

namespace alexaClientSDK {
    namespace adsl {
        using namespace std;
        using namespace avsCommon;
        using namespace avs;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace attachment;
        using namespace metrics;
        class MessageInterpreter : public MessageObserverInterface {
        public:
            MessageInterpreter(shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender,
                               shared_ptr<DirectiveSequencerInterface> directiveSequencer, shared_ptr<AttachmentManagerInterface> attachmentManager,
                               shared_ptr<MetricRecorderInterface> metricRecorder = nullptr);
            void receive(const string& contextId, const string& message) override;
        private:
            shared_ptr<ExceptionEncounteredSenderInterface> m_exceptionEncounteredSender;
            shared_ptr<DirectiveSequencerInterface> m_directiveSequencer;
            shared_ptr<AttachmentManagerInterface> m_attachmentManager;
            shared_ptr<MetricRecorderInterface> m_metricRecorder;
        };
    }
}
#endif