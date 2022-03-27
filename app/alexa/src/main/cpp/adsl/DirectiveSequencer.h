#ifndef ALEXA_CLIENT_SDK_ADSL_INCLUDE_ADSL_DIRECTIVESEQUENCER_H_
#define ALEXA_CLIENT_SDK_ADSL_INCLUDE_ADSL_DIRECTIVESEQUENCER_H_

#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>
#include <sdkinterfaces/ExceptionEncounteredSenderInterface.h>
#include <sdkinterfaces/DirectiveSequencerInterface.h>
#include <metrics/MetricRecorderInterface.h>
#include "DirectiveProcessor.h"
#include "DirectiveRouter.h"

namespace alexaClientSDK {
    namespace adsl {
        using namespace std;
        using namespace avsCommon;
        using namespace avs;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace metrics;
        class DirectiveSequencer : public DirectiveSequencerInterface {
        public:
            static unique_ptr<DirectiveSequencerInterface> create(shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
                                                                  shared_ptr<MetricRecorderInterface> metricRecorder = nullptr);
            bool addDirectiveHandler(shared_ptr<DirectiveHandlerInterface> handler) override;
            bool removeDirectiveHandler(shared_ptr<DirectiveHandlerInterface> handler) override;
            void setDialogRequestId(const string& dialogRequestId) override;
            string getDialogRequestId() override;
            bool onDirective(shared_ptr<AVSDirective> directive) override;
            void disable() override;
            void enable() override;
        private:
            DirectiveSequencer(shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender, shared_ptr<MetricRecorderInterface> metricRecorder);
            void doShutdown() override;
            void receivingLoop();
            void receiveDirectiveLocked(unique_lock<mutex>& lock);
            mutex m_mutex;
            shared_ptr<ExceptionEncounteredSenderInterface> m_exceptionSender;
            bool m_isShuttingDown;
            bool m_isEnabled;
            DirectiveRouter m_directiveRouter;
            shared_ptr<DirectiveProcessor> m_directiveProcessor;
            deque<shared_ptr<AVSDirective>> m_receivingQueue;
            condition_variable m_wakeReceivingLoop;
            thread m_receivingThread;
        };
    }
}
#endif