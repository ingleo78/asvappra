#ifndef ALEXA_CLIENT_SDK_ADSL_INCLUDE_ADSL_DIRECTIVEROUTER_H_
#define ALEXA_CLIENT_SDK_ADSL_INCLUDE_ADSL_DIRECTIVEROUTER_H_

#include <map>
#include <set>
#include <unordered_map>
#include <avs/CapabilityTag.h>
#include <avs/DirectiveHandlerConfiguration.h>
#include <avs/HandlerAndPolicy.h>
#include <util/RequiresShutdown.h>
#include <metrics/MetricEventBuilder.h>
#include <metrics/MetricRecorderInterface.h>

namespace alexaClientSDK {
    namespace adsl {
        using namespace std;
        using namespace avsCommon;
        using namespace avs;
        using namespace utils;
        using namespace sdkInterfaces;
        using namespace metrics;
        class DirectiveRouter : public RequiresShutdown {
        public:
            DirectiveRouter(shared_ptr<MetricRecorderInterface> metricRecorder = nullptr);
            bool addDirectiveHandler(shared_ptr<DirectiveHandlerInterface> handler);
            bool removeDirectiveHandler(shared_ptr<DirectiveHandlerInterface> handler);
            bool handleDirectiveImmediately(shared_ptr<AVSDirective> directive);
            bool preHandleDirective(shared_ptr<AVSDirective> directive, unique_ptr<DirectiveHandlerResultInterface> result);
            bool handleDirective(const shared_ptr<AVSDirective>& directive);
            bool cancelDirective(shared_ptr<AVSDirective> directive);
            BlockingPolicy getPolicy(const shared_ptr<AVSDirective>& directive);
        private:
            void doShutdown() override;
            class HandlerCallScope {
            public:
                HandlerCallScope(unique_lock<std::mutex>& lock, DirectiveRouter* router, shared_ptr<DirectiveHandlerInterface> handler);
                ~HandlerCallScope();
            private:
                unique_lock<mutex>& m_lock;
                DirectiveRouter* m_router;
                shared_ptr<DirectiveHandlerInterface> m_handler;
            };
            HandlerAndPolicy getHandlerAndPolicyLocked(const shared_ptr<AVSDirective>& directive);
            shared_ptr<DirectiveHandlerInterface> getHandlerLocked(shared_ptr<AVSDirective> directive);
            void incrementHandlerReferenceCountLocked(shared_ptr<DirectiveHandlerInterface> handler);
            void decrementHandlerReferenceCountLocked(unique_lock<mutex>& lock, shared_ptr<DirectiveHandlerInterface> handler);
            bool removeDirectiveHandlerLocked(shared_ptr<DirectiveHandlerInterface> handler);
            shared_ptr<MetricRecorderInterface> m_metricRecorder;
            mutex m_mutex;
            unordered_map<CapabilityTag, HandlerAndPolicy> m_configuration;
            unordered_map<shared_ptr<DirectiveHandlerInterface>, int> m_handlerReferenceCounts;
            void submitMetric(MetricEventBuilder& metricEventBuilder, const shared_ptr<AVSDirective>& directive);
        };
    }
}
#endif