#ifndef ALEXA_CLIENT_SDK_ADSL_INCLUDE_ADSL_DIRECTIVEPROCESSOR_H_
#define ALEXA_CLIENT_SDK_ADSL_INCLUDE_ADSL_DIRECTIVEPROCESSOR_H_

#include <array>
#include <condition_variable>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <unordered_map>
#include <avs/AVSDirective.h>
#include <sdkinterfaces/DirectiveHandlerInterface.h>
#include "DirectiveRouter.h"

namespace alexaClientSDK {
    namespace adsl {
        using namespace std;
        using namespace avsCommon;
        using namespace avs;
        using namespace sdkInterfaces;
        class DirectiveProcessor {
        public:
            DirectiveProcessor(DirectiveRouter* directiveRouter);
            ~DirectiveProcessor();
            void setDialogRequestId(const string& dialogRequestId);
            string getDialogRequestId();
            bool onDirective(shared_ptr<AVSDirective> directive);
            void shutdown();
            void disable();
            bool enable();
        private:
            using ProcessorHandle = unsigned int;
            using DirectiveAndPolicy = pair<shared_ptr<AVSDirective>, BlockingPolicy>;
            class DirectiveHandlerResult : public DirectiveHandlerResultInterface {
            public:
                DirectiveHandlerResult(ProcessorHandle processorHandle, shared_ptr<AVSDirective> directive);
                void setCompleted() override;
                void setFailed(const string& description) override;
            private:
                ProcessorHandle m_processorHandle;
                shared_ptr<AVSDirective> m_directive;
            };
            void onHandlingCompleted(shared_ptr<AVSDirective> directive);
            void onHandlingFailed(shared_ptr<AVSDirective> directive, const string& description);
            void removeDirectiveLocked(shared_ptr<AVSDirective> directive);
            void processingLoop();
            bool processCancelingQueueLocked(unique_lock<mutex>& lock);
            bool handleQueuedDirectivesLocked(unique_lock<mutex>& lock);
            void setDialogRequestIdLocked(const string& dialogRequestId);
            void scrubDialogRequestIdLocked(const string& dialogRequestId);
            void queueAllDirectivesForCancellationLocked();
            void setDirectiveBeingHandledLocked(const shared_ptr<AVSDirective>& directive, const BlockingPolicy policy);
            void clearDirectiveBeingHandledLocked(const BlockingPolicy policy);
            set<shared_ptr<AVSDirective>> clearDirectiveBeingHandledLocked(function<bool(const shared_ptr<AVSDirective>&)> shouldClear);
            deque<DirectiveAndPolicy>::iterator getNextUnblockedDirectiveLocked();
            int m_handle;
            mutex m_mutex;
            DirectiveRouter* m_directiveRouter;
            bool m_isShuttingDown;
            bool m_isEnabled;
            string m_dialogRequestId;
            deque<shared_ptr<AVSDirective>> m_cancelingQueue;
            shared_ptr<AVSDirective> m_directiveBeingPreHandled;
            deque<DirectiveAndPolicy> m_handlingQueue;
            condition_variable m_wakeProcessingLoop;
            thread m_processingThread;
            mutex m_onDirectiveMutex;
            static mutex m_handleMapMutex;
            static unordered_map<ProcessorHandle, DirectiveProcessor*> m_handleMap;
            static ProcessorHandle m_nextProcessorHandle;
            array<shared_ptr<AVSDirective>, BlockingPolicy::Medium::COUNT> m_directivesBeingHandled;
        };
    }
}
#endif