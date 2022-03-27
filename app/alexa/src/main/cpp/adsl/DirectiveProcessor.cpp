#include <algorithm>
#include <iostream>
#include <sstream>
#include <avs/ExceptionErrorType.h>
#include <logger/Logger.h>
#include <memory/Memory.h>
#include "DirectiveProcessor.h"

using namespace std;
using namespace alexaClientSDK::avsCommon;
using namespace avs;
using namespace utils;
using namespace logger;
using namespace sdkInterfaces;

static const string TAG("DirectiveProcessor");
#define LX(event) LogEntry(TAG, event)
namespace alexaClientSDK {
    namespace adsl {
        mutex DirectiveProcessor::m_handleMapMutex;
        DirectiveProcessor::ProcessorHandle DirectiveProcessor::m_nextProcessorHandle = 0;
        unordered_map<DirectiveProcessor::ProcessorHandle, DirectiveProcessor*> DirectiveProcessor::m_handleMap;
        DirectiveProcessor::DirectiveProcessor(DirectiveRouter* directiveRouter) : m_directiveRouter{directiveRouter}, m_isShuttingDown{false}, m_isEnabled{true} {
            std::lock_guard<std::mutex> lock(m_handleMapMutex);
            m_handle = ++m_nextProcessorHandle;
            m_handleMap[m_handle] = this;
            m_processingThread = thread(&DirectiveProcessor::processingLoop, this);
        }
        DirectiveProcessor::~DirectiveProcessor() {
            shutdown();
        }
        void DirectiveProcessor::setDialogRequestId(const string& dialogRequestId) {
            lock_guard<mutex> lock(m_mutex);
            setDialogRequestIdLocked(dialogRequestId);
        }
        string DirectiveProcessor::getDialogRequestId() {
            lock_guard<mutex> lock(m_mutex);
            return m_dialogRequestId;
        }
        bool DirectiveProcessor::onDirective(shared_ptr<AVSDirective> directive) {
            if (!directive) {
                ACSDK_ERROR(LX("onDirectiveFailed").d("action", "ignored").d("reason", "nullptrDirective"));
                return false;
            }
            lock_guard<mutex> onDirectiveLock(m_onDirectiveMutex);
            unique_lock<mutex> lock(m_mutex);
            if (m_isShuttingDown || !m_isEnabled) {
                ACSDK_WARN(LX("onDirectiveFailed").d("messageId", directive->getMessageId()).d("action", "ignored")
                               .d("reason", m_isShuttingDown ? "shuttingDown" : "disabled"));
                return false;
            }
            if (!directive->getDialogRequestId().empty() && directive->getDialogRequestId() != m_dialogRequestId) {
                ACSDK_INFO(LX("onDirective").d("messageId", directive->getMessageId()).d("action", "dropped")
                               .d("reason", "dialogRequestIdDoesNotMatch").d("namespace", directive->getNamespace()).d("name", directive->getName())
                               .d("directivesDialogRequestId", directive->getDialogRequestId()).d("dialogRequestId", m_dialogRequestId));
                return true;
            }
            auto policy = m_directiveRouter->getPolicy(directive);
            m_directiveBeingPreHandled = directive;
            lock.unlock();
            auto preHandled = m_directiveRouter->preHandleDirective(directive, memory::make_unique<DirectiveHandlerResult>(m_handle, directive));
            lock.lock();
            if (!m_directiveBeingPreHandled && preHandled) return true;
            m_directiveBeingPreHandled.reset();
            if (!preHandled) return false;
            auto item = make_pair(directive, policy);
            m_handlingQueue.push_back(item);
            m_wakeProcessingLoop.notify_one();
            return true;
        }
        void DirectiveProcessor::shutdown() {
            {
                lock_guard<mutex> lock(m_handleMapMutex);
                m_handleMap.erase(m_handle);
            }
            {
                lock_guard<mutex> lock(m_mutex);
                queueAllDirectivesForCancellationLocked();
                m_isShuttingDown = true;
                m_wakeProcessingLoop.notify_one();
            }
            if (m_processingThread.joinable()) m_processingThread.join();
        }
        void DirectiveProcessor::disable() {
            lock_guard<mutex> lock(m_mutex);
            ACSDK_DEBUG(LX("disable"));
            queueAllDirectivesForCancellationLocked();
            m_isEnabled = false;
            m_wakeProcessingLoop.notify_one();
        }
        bool DirectiveProcessor::enable() {
            lock_guard<mutex> lock{m_mutex};
            m_isEnabled = true;
            return m_isEnabled;
        }
        DirectiveProcessor::DirectiveHandlerResult::DirectiveHandlerResult(DirectiveProcessor::ProcessorHandle processorHandle, shared_ptr<AVSDirective> directive) :
                                                                           m_processorHandle{processorHandle}, m_directive{directive} {}
        void DirectiveProcessor::DirectiveHandlerResult::setCompleted() {
            lock_guard<mutex> lock(m_handleMapMutex);
            auto it = m_handleMap.find(m_processorHandle);
            if (it == m_handleMap.end()) {
                ACSDK_DEBUG(LX("setCompletedIgnored").d("reason", "directiveSequencerAlreadyShutDown"));
                return;
            }
            it->second->onHandlingCompleted(m_directive);
        }
        void DirectiveProcessor::DirectiveHandlerResult::setFailed(const string& description) {
            lock_guard<mutex> lock(m_handleMapMutex);
            auto it = m_handleMap.find(m_processorHandle);
            if (it == m_handleMap.end()) {
                ACSDK_DEBUG(LX("setFailedIgnored").d("reason", "directiveSequencerAlreadyShutDown"));
                return;
            }
            it->second->onHandlingFailed(m_directive, description);
        }
        void DirectiveProcessor::onHandlingCompleted(shared_ptr<AVSDirective> directive) {
            lock_guard<mutex> lock(m_mutex);
            ACSDK_DEBUG(LX("onHandlingCompeted").d("messageId", directive->getMessageId())
                            .d("directiveBeingPreHandled", m_directiveBeingPreHandled ? m_directiveBeingPreHandled->getMessageId() : "(nullptr)"));
            removeDirectiveLocked(directive);
        }
        void DirectiveProcessor::onHandlingFailed(shared_ptr<AVSDirective> directive, const string& description) {
            unique_lock<mutex> lock(m_mutex);
            ACSDK_DEBUG(LX("onHandlingFailed").d("messageId", directive->getMessageId()).d("namespace", directive->getNamespace())
                            .d("name", directive->getName())
                            .d("directiveBeingPreHandled", m_directiveBeingPreHandled ? m_directiveBeingPreHandled->getMessageId() : "(nullptr)")
                            .d("description", description));
            removeDirectiveLocked(directive);
            scrubDialogRequestIdLocked(directive->getDialogRequestId());
        }
        void DirectiveProcessor::removeDirectiveLocked(shared_ptr<AVSDirective> directive) {
            auto matches = [directive](shared_ptr<AVSDirective> item) { return item == directive; };
            auto handlingMatches = [directive](pair<shared_ptr<AVSDirective>, BlockingPolicy> directiveAndPolicy) {
                return directiveAndPolicy.first == directive;
            };
            m_cancelingQueue.erase(remove_if(m_cancelingQueue.begin(), m_cancelingQueue.end(), matches), m_cancelingQueue.end());
            if (matches(m_directiveBeingPreHandled)) m_directiveBeingPreHandled.reset();
            m_handlingQueue.erase(remove_if(m_handlingQueue.begin(), m_handlingQueue.end(), handlingMatches), m_handlingQueue.end());
            if (m_directivesBeingHandled[BlockingPolicy::Medium::AUDIO] && matches(m_directivesBeingHandled[BlockingPolicy::Medium::AUDIO])) {
                m_directivesBeingHandled[BlockingPolicy::Medium::AUDIO].reset();
            }
            if (m_directivesBeingHandled[BlockingPolicy::Medium::VISUAL] && matches(m_directivesBeingHandled[BlockingPolicy::Medium::VISUAL])) {
                m_directivesBeingHandled[BlockingPolicy::Medium::VISUAL].reset();
            }
            if (!m_cancelingQueue.empty() || !m_handlingQueue.empty()) m_wakeProcessingLoop.notify_one();
        }
        void DirectiveProcessor::setDirectiveBeingHandledLocked(const shared_ptr<AVSDirective>& directive, const BlockingPolicy policy) {
            if (policy.getMediums()[BlockingPolicy::Medium::AUDIO]) m_directivesBeingHandled[BlockingPolicy::Medium::AUDIO] = directive;
            if (policy.getMediums()[BlockingPolicy::Medium::VISUAL]) m_directivesBeingHandled[BlockingPolicy::Medium::VISUAL] = directive;
        }
        void DirectiveProcessor::clearDirectiveBeingHandledLocked(const BlockingPolicy policy) {
            if ((policy.getMediums()[BlockingPolicy::Medium::AUDIO]) && m_directivesBeingHandled[BlockingPolicy::Medium::AUDIO]) {
                m_directivesBeingHandled[BlockingPolicy::Medium::AUDIO].reset();
            }
            if ((policy.getMediums()[BlockingPolicy::Medium::VISUAL]) && m_directivesBeingHandled[BlockingPolicy::Medium::VISUAL]) {
                m_directivesBeingHandled[BlockingPolicy::Medium::VISUAL].reset();
            }
        }
        set<shared_ptr<AVSDirective>> DirectiveProcessor::clearDirectiveBeingHandledLocked(
            function<bool(const shared_ptr<AVSDirective>&)> shouldClear) {
            set<shared_ptr<AVSDirective>> freed;
            auto directive = m_directivesBeingHandled[BlockingPolicy::Medium::AUDIO];
            if (directive && shouldClear(directive)) {
                freed.insert(directive);
                m_directivesBeingHandled[BlockingPolicy::Medium::AUDIO].reset();
            }
            directive = m_directivesBeingHandled[BlockingPolicy::Medium::VISUAL];
            if (directive && shouldClear(directive)) {
                freed.insert(directive);
                m_directivesBeingHandled[BlockingPolicy::Medium::VISUAL].reset();
            }
            return freed;
        }
        deque<DirectiveProcessor::DirectiveAndPolicy>::iterator DirectiveProcessor::getNextUnblockedDirectiveLocked() {
            array<bool, BlockingPolicy::Medium::COUNT> blockedMediums;
            blockedMediums[BlockingPolicy::Medium::AUDIO] = (m_directivesBeingHandled[BlockingPolicy::Medium::AUDIO] != nullptr);
            blockedMediums[BlockingPolicy::Medium::VISUAL] = (m_directivesBeingHandled[BlockingPolicy::Medium::VISUAL] != nullptr);
            for (auto it = m_handlingQueue.begin(); it != m_handlingQueue.end(); it++) {
                auto policy = it->second;
                bool currentUsingAudio = false;
                bool currentUsingVisual = false;
                if (policy.getMediums()[BlockingPolicy::Medium::AUDIO]) currentUsingAudio = true;
                if (policy.getMediums()[BlockingPolicy::Medium::VISUAL]) currentUsingVisual = true;
                if ((currentUsingAudio && blockedMediums[BlockingPolicy::Medium::AUDIO]) || (currentUsingVisual && blockedMediums[BlockingPolicy::Medium::VISUAL])) {
                    if (it->second.isBlocking()) {
                        blockedMediums[BlockingPolicy::Medium::AUDIO] = (blockedMediums[BlockingPolicy::Medium::AUDIO] || currentUsingAudio);
                        blockedMediums[BlockingPolicy::Medium::VISUAL] = (blockedMediums[BlockingPolicy::Medium::VISUAL] || currentUsingVisual);
                    }
                } else return it;
            }
            return m_handlingQueue.end();
        }
        void DirectiveProcessor::processingLoop() {
            ACSDK_DEBUG9(LX("processingLoop"));
            auto haveUnblockedDirectivesToHandle = [this] {
                return getNextUnblockedDirectiveLocked() != m_handlingQueue.end();
            };
            auto wake = [this, haveUnblockedDirectivesToHandle]() {
                return !m_cancelingQueue.empty() || (!m_handlingQueue.empty() && haveUnblockedDirectivesToHandle()) || m_isShuttingDown;
            };
            unique_lock<std::mutex> lock(m_mutex);
            do {
                m_wakeProcessingLoop.wait(lock, wake);
                bool cancelHandled = false;
                bool queuedHandled = false;
                do {
                    cancelHandled = processCancelingQueueLocked(lock);
                    queuedHandled = handleQueuedDirectivesLocked(lock);
                } while(cancelHandled || queuedHandled);
            } while(!m_isShuttingDown);
        }
        bool DirectiveProcessor::processCancelingQueueLocked(unique_lock<mutex>& lock) {
            ACSDK_DEBUG9(LX("processCancelingQueueLocked").d("size", m_cancelingQueue.size()));
            if (m_cancelingQueue.empty()) return false;
            deque<shared_ptr<AVSDirective>> temp;
            swap(m_cancelingQueue, temp);
            lock.unlock();
            for (auto directive : temp) {
                m_directiveRouter->cancelDirective(directive);
            }
            lock.lock();
            return true;
        }
        bool DirectiveProcessor::handleQueuedDirectivesLocked(unique_lock<mutex>& lock) {
            if (m_handlingQueue.empty()) return false;
            bool handleDirectiveCalled = false;
            ACSDK_DEBUG9(LX("handleQueuedDirectivesLocked").d("queue size", m_handlingQueue.size()));
            while(!m_handlingQueue.empty()) {
                auto it = getNextUnblockedDirectiveLocked();
                if (it == m_handlingQueue.end()) {
                    ACSDK_DEBUG9(LX("handleQueuedDirectivesLocked").m("all queued directives are blocked"));
                    break;
                }
                auto directive = it->first;
                auto policy = it->second;
                setDirectiveBeingHandledLocked(directive, policy);
                m_handlingQueue.erase((it));
                ACSDK_DEBUG9(LX("handleQueuedDirectivesLocked").d("proceeding with directive", directive->getMessageId()).d("policy", policy));
                handleDirectiveCalled = true;
                lock.unlock();
                auto handleDirectiveSucceeded = m_directiveRouter->handleDirective(directive);
                lock.lock();
                if (!handleDirectiveSucceeded || !policy.isBlocking()) clearDirectiveBeingHandledLocked(policy);
                if (!handleDirectiveSucceeded) {
                    ACSDK_ERROR(LX("handleDirectiveFailed").d("message id", directive->getMessageId()).d("namespace", directive->getNamespace())
                                    .d("name", directive->getName()).d("reason", "dialog request id does not match")
                                    .d("directive's dialog request id", directive->getDialogRequestId()).d("dialog request id", m_dialogRequestId));
                    scrubDialogRequestIdLocked(directive->getDialogRequestId());
                }
            }
            return handleDirectiveCalled;
        }
        void DirectiveProcessor::setDialogRequestIdLocked(const string& dialogRequestId) {
            if (dialogRequestId == m_dialogRequestId) {
                ACSDK_WARN(LX("setDialogRequestIdLockedIgnored").d("reason", "unchanged").d("dialogRequestId", dialogRequestId));
                return;
            }
            ACSDK_INFO(LX("setDialogRequestIdLocked").d("oldValue", m_dialogRequestId).d("newValue", dialogRequestId));
            scrubDialogRequestIdLocked(m_dialogRequestId);
            m_dialogRequestId = dialogRequestId;
        }
        void DirectiveProcessor::scrubDialogRequestIdLocked(const string& dialogRequestId) {
            if (dialogRequestId.empty()) {
                ACSDK_DEBUG(LX("scrubDialogRequestIdLocked").d("reason", "emptyDialogRequestId"));
                return;
            }
            ACSDK_DEBUG(LX("scrubDialogRequestIdLocked").d("dialogRequestId", dialogRequestId));
            bool changed = false;
            if (m_directiveBeingPreHandled) {
                auto id = m_directiveBeingPreHandled->getDialogRequestId();
                if (!id.empty() && id == dialogRequestId) {
                    m_cancelingQueue.push_back(m_directiveBeingPreHandled);
                    m_directiveBeingPreHandled.reset();
                    changed = true;
                }
            }
            auto freed = clearDirectiveBeingHandledLocked([dialogRequestId](const shared_ptr<AVSDirective>& directive) {
                return directive->getDialogRequestId() == dialogRequestId;
            });
            if (!freed.empty()) {
                m_cancelingQueue.insert(m_cancelingQueue.end(), freed.begin(), freed.end());
                changed = true;
            }
            deque<DirectiveAndPolicy> temp;
            for (const auto& directiveAndPolicy : m_handlingQueue) {
                auto id = (directiveAndPolicy.first)->getDialogRequestId();
                if (!id.empty() && id == dialogRequestId) {
                    m_cancelingQueue.push_back(directiveAndPolicy.first);
                    changed = true;
                } else temp.push_back(directiveAndPolicy);
            }
            swap(temp, m_handlingQueue);
            if (dialogRequestId == m_dialogRequestId) m_dialogRequestId.clear();
            if (changed) {
                ACSDK_DEBUG9(LX("notifyingProcessingLoop").d("size:", m_cancelingQueue.size()));
                m_wakeProcessingLoop.notify_one();
            }
        }
        void DirectiveProcessor::queueAllDirectivesForCancellationLocked() {
            ACSDK_DEBUG9(LX("queueAllDirectivesForCancellationLocked"));
            bool changed = false;
            m_dialogRequestId.clear();
            auto freed = clearDirectiveBeingHandledLocked([](const shared_ptr<AVSDirective>& directive) { return true; });
            if (!freed.empty()) {
                changed = true;
                m_cancelingQueue.insert(m_cancelingQueue.end(), freed.begin(), freed.end());
            }
            if (!m_handlingQueue.empty()) {
                transform(m_handlingQueue.begin(),m_handlingQueue.end(),back_inserter(m_cancelingQueue),
                          [](DirectiveAndPolicy directiveAndPolicy) { return directiveAndPolicy.first; });
                m_handlingQueue.clear();
                changed = true;
            }
            if (m_directiveBeingPreHandled) {
                m_cancelingQueue.push_back(m_directiveBeingPreHandled);
                m_directiveBeingPreHandled.reset();
            }
            if (changed) {
                ACSDK_DEBUG9(LX("notifyingProcessingLoop"));
                m_wakeProcessingLoop.notify_one();
            }
        }
    }
}