#include <iostream>
#include <set>
#include <sstream>
#include <vector>
#include <avs/DirectiveRoutingRule.h>
#include <logger/Logger.h>
#include <metrics/DataPointCounterBuilder.h>
#include <metrics/DataPointStringBuilder.h>
#include <util/Optional.h>
#include "DirectiveRouter.h"

using namespace std;
using namespace alexaClientSDK::avsCommon;
using namespace avs;
using namespace directiveRoutingRule;
using namespace sdkInterfaces;
using namespace utils;
using namespace logger;
using namespace metrics;

static const string TAG("DirectiveRouter");
#define LX(event) LogEntry(TAG, event)
namespace alexaClientSDK {
    namespace adsl {
        static const string DIRECTIVE_SEQUENCER_METRIC_PREFIX = "DIRECTIVE_SEQUENCER-";
        static const string DIRECTIVE_DISPATCHED_IMMEDIATE = "DIRECTIVE_DISPATCHED_IMMEDIATE";
        static const string DIRECTIVE_DISPATCHED_PRE_HANDLE = "DIRECTIVE_DISPATCHED_PRE_HANDLE";
        static const string DIRECTIVE_DISPATCHED_HANDLE = "DIRECTIVE_DISPATCHED_HANDLE";
        void DirectiveRouter::submitMetric(MetricEventBuilder& metricEventBuilder, const shared_ptr<AVSDirective>& directive) {
            if (directive) {
                metricEventBuilder.addDataPoint(DataPointStringBuilder{}.setName("HTTP2_STREAM").setValue(directive->getAttachmentContextId()).build());
                metricEventBuilder.addDataPoint(DataPointStringBuilder{}.setName("DIRECTIVE_MESSAGE_ID").setValue(directive->getMessageId()).build());
            }
            auto metricEvent = metricEventBuilder.build();
            if (metricEvent) recordMetric(m_metricRecorder, metricEvent);
            else { ACSDK_ERROR(LX("submitMetricFailed").d("reason", "buildMetricFailed")); }
        }
        DirectiveRouter::DirectiveRouter(shared_ptr<MetricRecorderInterface> metricRecorder) : RequiresShutdown{"DirectiveRouter"},
                                                                                               m_metricRecorder{metricRecorder} {}
        bool DirectiveRouter::addDirectiveHandler(shared_ptr<DirectiveHandlerInterface> handler) {
            lock_guard<mutex> lock(m_mutex);
            if (isShutdown()) {
                ACSDK_ERROR(LX("addDirectiveHandlersFailed").d("reason", "isShutdown"));
                return false;
            }
            if (!handler) {
                ACSDK_ERROR(LX("addDirectiveHandlersFailed").d("reason", "emptyHandler"));
                return false;
            }
            auto configuration = handler->getConfiguration();
            for (auto& item : configuration) {
                if (!(item.second.isValid())) {
                    ACSDK_ERROR(LX("addDirectiveHandlersFailed").d("reason", "nonePolicy"));
                    return false;
                }
                if (!isDirectiveRoutingRuleValid(item.first)) {
                    ACSDK_ERROR(LX("addDirectiveHandlersFailed").d("reason", "invalidRule").d("rule", item.first));
                    return false;
                }
                auto it = m_configuration.find(item.first);
                if (m_configuration.end() != it) {
                    ACSDK_ERROR(LX("addDirectiveHandlersFailed").d("reason", "alreadySet").d("namespace", item.first.nameSpace).d("name", item.first.name));
                    return false;
                }
            }
            for (auto item : configuration) {
                HandlerAndPolicy handlerAndPolicy(handler, item.second);
                m_configuration[item.first] = handlerAndPolicy;
                incrementHandlerReferenceCountLocked(handler);
                ACSDK_DEBUG9(LX("addDirectiveHandlers").d("action", "added").d("namespace", item.first.nameSpace).d("name", item.first.name)
                                 .d("handler", handler.get()).d("policy", item.second));
            }
            return true;
        }
        bool DirectiveRouter::removeDirectiveHandlerLocked(shared_ptr<DirectiveHandlerInterface> handler) {
            if (!handler) {
                ACSDK_ERROR(LX("removeDirectiveHandlersFailed").d("reason", "nullptrHandler"));
                return false;
            }
            auto configuration = handler->getConfiguration();
            for (auto item : configuration) {
                auto it = m_configuration.find(item.first);
                if (m_configuration.end() == it || it->second != HandlerAndPolicy(handler, item.second)) {
                    ACSDK_ERROR(LX("removeDirectiveHandlersFailed").d("reason", "notFound").d("namespace", item.first.nameSpace)
                                    .d("name", item.first.name).d("handler", handler.get()).d("policy", item.second));
                    return false;
                }
            }
            for (auto item : configuration) {
                m_configuration.erase(item.first);
                ACSDK_DEBUG9(LX("removeDirectiveHandlers").d("action", "removed").d("namespace", item.first.nameSpace)
                                 .d("name", item.first.name).d("handler", handler.get()).d("policy", item.second));
                auto it = m_handlerReferenceCounts.find(handler);
                if (0 == --(it->second)) m_handlerReferenceCounts.erase(it);
            }
            return true;
        }
        bool DirectiveRouter::removeDirectiveHandler(shared_ptr<DirectiveHandlerInterface> handler) {
            unique_lock<mutex> lock(m_mutex);
            if (!removeDirectiveHandlerLocked(handler)) return false;
            lock.unlock();
            ACSDK_DEBUG9(LX("onDeregisteredCalled").d("handler", handler.get()));
            handler->onDeregistered();
            return true;
        }
        bool DirectiveRouter::handleDirectiveImmediately(shared_ptr<AVSDirective> directive) {
            unique_lock<std::mutex> lock(m_mutex);
            auto handlerAndPolicy = getHandlerAndPolicyLocked(directive);
            if (!handlerAndPolicy) {
                ACSDK_WARN(LX("handleDirectiveImmediatelyFailed").d("messageId", directive->getMessageId()).d("reason", "noHandlerRegistered"));
                return false;
            }
            ACSDK_INFO(LX("handleDirectiveImmediately").d("messageId", directive->getMessageId()).d("action", "calling"));
            HandlerCallScope scope(lock, this, handlerAndPolicy.handler);
            submitMetric(MetricEventBuilder{}.setActivityName(DIRECTIVE_SEQUENCER_METRIC_PREFIX + DIRECTIVE_DISPATCHED_IMMEDIATE)
                    .addDataPoint(DataPointCounterBuilder{}.setName(DIRECTIVE_DISPATCHED_IMMEDIATE).increment(1).build()), directive);
            handlerAndPolicy.handler->handleDirectiveImmediately(directive);
            return true;
        }
        bool DirectiveRouter::preHandleDirective(shared_ptr<AVSDirective> directive, unique_ptr<DirectiveHandlerResultInterface> result) {
            unique_lock<mutex> lock(m_mutex);
            auto handler = getHandlerLocked(directive);
            if (!handler) {
                ACSDK_WARN(LX("preHandleDirectiveFailed").d("messageId", directive->getMessageId()).d("reason", "noHandlerRegistered"));
                return false;
            }
            ACSDK_INFO(LX("preHandleDirective").d("messageId", directive->getMessageId()).d("action", "calling"));
            HandlerCallScope scope(lock, this, handler);
            submitMetric(MetricEventBuilder{}.setActivityName(DIRECTIVE_SEQUENCER_METRIC_PREFIX + DIRECTIVE_DISPATCHED_PRE_HANDLE)
                    .addDataPoint(DataPointCounterBuilder{}.setName(DIRECTIVE_DISPATCHED_PRE_HANDLE).increment(1).build()), directive);
            handler->preHandleDirective(directive, std::move(result));
            return true;
        }
        bool DirectiveRouter::handleDirective(const shared_ptr<AVSDirective>& directive) {
            unique_lock<mutex> lock(m_mutex);
            auto handler = getHandlerLocked(directive);
            if (!handler) {
                ACSDK_WARN(LX("handleDirectiveFailed").d("messageId", directive->getMessageId()).d("reason", "noHandlerRegistered"));
                return false;
            }
            ACSDK_INFO(LX("handleDirective").d("messageId", directive->getMessageId()).d("action", "calling"));
            HandlerCallScope scope(lock, this, handler);
            submitMetric(MetricEventBuilder{}.setActivityName(DIRECTIVE_SEQUENCER_METRIC_PREFIX + DIRECTIVE_DISPATCHED_HANDLE)
                    .addDataPoint(DataPointCounterBuilder{}.setName(DIRECTIVE_DISPATCHED_HANDLE).increment(1).build()),directive);
            auto result = handler->handleDirective(directive->getMessageId());
            if (!result) {
                ACSDK_WARN(LX("messageIdNotRecognized").d("handler", handler.get()).d("messageId", directive->getMessageId())
                               .d("reason", "handleDirectiveReturnedFalse"));
            }
            return result;
        }
        bool DirectiveRouter::cancelDirective(shared_ptr<AVSDirective> directive) {
            unique_lock<mutex> lock(m_mutex);
            auto handler = getHandlerLocked(directive);
            if (!handler) {
                ACSDK_WARN(LX("cancelDirectiveFailed").d("messageId", directive->getMessageId()).d("reason", "noHandlerRegistered"));
                return false;
            }
            ACSDK_INFO(LX("cancelDirective").d("messageId", directive->getMessageId()).d("action", "calling"));
            HandlerCallScope scope(lock, this, handler);
            handler->cancelDirective(directive->getMessageId());
            return true;
        }
        void DirectiveRouter::doShutdown() {
            vector<shared_ptr<DirectiveHandlerInterface>> releasedHandlers;
            unique_lock<mutex> lock(m_mutex);
            size_t numConfigurations = m_configuration.size();
            for (size_t i = 0; i < numConfigurations && !m_configuration.empty(); ++i) {
                auto handler = m_configuration.begin()->second.handler;
                if (removeDirectiveHandlerLocked(handler)) releasedHandlers.push_back(handler);
            }
            m_configuration.clear();
            lock.unlock();
            for (auto releasedHandler : releasedHandlers) {
                ACSDK_DEBUG9(LX("onDeregisteredCalled").d("handler", releasedHandler.get()));
                releasedHandler->onDeregistered();
            }
        }
        DirectiveRouter::HandlerCallScope::HandlerCallScope(unique_lock<mutex>& lock, DirectiveRouter* router, shared_ptr<DirectiveHandlerInterface> handler) :
                                                            m_lock(lock), m_router{router}, m_handler{handler} {
            m_router->incrementHandlerReferenceCountLocked(m_handler);
            m_lock.unlock();
        }
        DirectiveRouter::HandlerCallScope::~HandlerCallScope() {
            m_lock.lock();
            m_router->decrementHandlerReferenceCountLocked(m_lock, m_handler);
        }
        BlockingPolicy DirectiveRouter::getPolicy(const shared_ptr<AVSDirective>& directive) {
            unique_lock<mutex> lock(m_mutex);
            return getHandlerAndPolicyLocked(directive).policy;
        }
        HandlerAndPolicy DirectiveRouter::getHandlerAndPolicyLocked(const shared_ptr<AVSDirective>& directive) {
            if (!directive) {
                ACSDK_ERROR(LX("getConfiguredHandlerAndPolicyLockedFailed").d("reason", "nullptrDirective"));
                return HandlerAndPolicy();
            }
            const auto nameSpace = directive->getNamespace();
            const auto name = directive->getName();
            const auto endpointId = directive->getEndpoint().hasValue() ? directive->getEndpoint().value().endpointId : "";
            auto instance = Optional<std::string>();
            if (!directive->getHeader()->getInstance().empty()) instance.set(directive->getHeader()->getInstance());
            vector<DirectiveRoutingRule> matchers = {
                routingRulePerDirective(endpointId, instance, nameSpace, name), routingRulePerNamespace(endpointId, instance, nameSpace),
                routingRulePerInstance(endpointId, instance), routingRulePerNamespaceAnyInstance(endpointId, nameSpace),
                routingRulePerEndpoint(endpointId)
            };
            for (const auto& matcher : matchers) {
                auto it = m_configuration.find(matcher);
                if (it != m_configuration.end()) {
                    ACSDK_DEBUG5(LX(__func__).m("configuration found").sensitive("config", it->first));
                    return it->second;
                }
            }
            ACSDK_DEBUG5(LX(__func__).m("noMatcher").sensitive("matcher", matchers[0]));
            return HandlerAndPolicy();
        }
        shared_ptr<DirectiveHandlerInterface> DirectiveRouter::getHandlerLocked(shared_ptr<AVSDirective> directive) {
            auto handlerAndPolicy = getHandlerAndPolicyLocked(directive);
            if (!handlerAndPolicy) {
                ACSDK_DEBUG0(LX("noHandlerFoundForDirective").d("namespace", directive->getNamespace()).d("name", directive->getName()));
                return nullptr;
            }
            return handlerAndPolicy.handler;
        }
        void DirectiveRouter::incrementHandlerReferenceCountLocked(shared_ptr<DirectiveHandlerInterface> handler) {
            const auto it = m_handlerReferenceCounts.find(handler);
            if (it != m_handlerReferenceCounts.end()) it->second++;
            else m_handlerReferenceCounts[handler] = 1;
        }
        void DirectiveRouter::decrementHandlerReferenceCountLocked(
            unique_lock<mutex>& lock,
            shared_ptr<DirectiveHandlerInterface> handler) {
            const auto it = m_handlerReferenceCounts.find(handler);
            if (it != m_handlerReferenceCounts.end()) {
                if (0 == --(it->second)) {
                    m_handlerReferenceCounts.erase(it);
                    ACSDK_DEBUG9(LX("onDeregisteredCalled").d("handler", handler.get()));
                    lock.unlock();
                    handler->onDeregistered();
                    lock.lock();
                }
            } else ACSDK_ERROR(LX("removeHandlerReferenceLockedFailed").d("reason", "handlerNotFound"));
        }
    }
}