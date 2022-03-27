#include <algorithm>
#include <iostream>
#include <sstream>
#include <avs/ExceptionErrorType.h>
#include <logger/Logger.h>
#include <util/Metrics.h>
#include "DirectiveSequencer.h"

using namespace std;
using namespace alexaClientSDK::avsCommon;
using namespace avs;
using namespace sdkInterfaces;
using namespace utils;
using namespace logger;
using namespace metrics;

static const string TAG("DirectiveSequencer");
#define LX(event) LogEntry(TAG, event)
namespace alexaClientSDK {
    namespace adsl {
        unique_ptr<DirectiveSequencerInterface> DirectiveSequencer::create(shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
                                                                           shared_ptr<MetricRecorderInterface> metricRecorder) {
            if (!exceptionSender) {
                ACSDK_INFO(LX("createFailed").d("reason", "nullptrExceptionSender"));
                return nullptr;
            }
            return unique_ptr<DirectiveSequencerInterface>(new DirectiveSequencer(exceptionSender, metricRecorder));
        }
        bool DirectiveSequencer::addDirectiveHandler(shared_ptr<DirectiveHandlerInterface> handler) {
            return m_directiveRouter.addDirectiveHandler(handler);
        }
        bool DirectiveSequencer::removeDirectiveHandler(shared_ptr<DirectiveHandlerInterface> handler) {
            return m_directiveRouter.removeDirectiveHandler(handler);
        }
        void DirectiveSequencer::setDialogRequestId(const string& dialogRequestId) {
            m_directiveProcessor->setDialogRequestId(dialogRequestId);
        }
        string DirectiveSequencer::getDialogRequestId() {
            return m_directiveProcessor->getDialogRequestId();
        }
        bool DirectiveSequencer::onDirective(shared_ptr<AVSDirective> directive) {
            if (!directive) {
                ACSDK_ERROR(LX("onDirectiveFailed").d("action", "ignored").d("reason", "nullptrDirective"));
                return false;
            }
            lock_guard<mutex> lock(m_mutex);
            if (m_isShuttingDown || !m_isEnabled) {
                ACSDK_WARN(LX("onDirectiveFailed").d("directive", directive->getHeaderAsString()).d("action", "ignored")
                               .d("reason", m_isShuttingDown ? "isShuttingDown" : "disabled"));
                return false;
            }
            ACSDK_INFO(LX("onDirective").d("directive", directive->getHeaderAsString()));
            m_receivingQueue.push_back(directive);
            m_wakeReceivingLoop.notify_one();
            return true;
        }
        DirectiveSequencer::DirectiveSequencer(shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender, shared_ptr<MetricRecorderInterface> metricRecorder) :
                                               DirectiveSequencerInterface{"DirectiveSequencer"}, m_mutex{}, m_exceptionSender{exceptionSender},
                                               m_isShuttingDown{false}, m_isEnabled{true}, m_directiveRouter{metricRecorder} {
            m_directiveProcessor = make_shared<DirectiveProcessor>(&m_directiveRouter);
            m_receivingThread = thread(&DirectiveSequencer::receivingLoop, this);
        }
        void DirectiveSequencer::doShutdown() {
            ACSDK_DEBUG9(LX("doShutdown"));
            {
                lock_guard<mutex> lock(m_mutex);
                m_isShuttingDown = true;
                m_wakeReceivingLoop.notify_one();
            }
            if (m_receivingThread.joinable()) m_receivingThread.join();
            m_directiveProcessor->shutdown();
            m_directiveRouter.shutdown();
            m_exceptionSender.reset();
        }
        void DirectiveSequencer::disable() {
            ACSDK_DEBUG9(LX("disable"));
            lock_guard<mutex> lock(m_mutex);
            m_isEnabled = false;
            m_directiveProcessor->setDialogRequestId("");
            m_directiveProcessor->disable();
            m_wakeReceivingLoop.notify_one();
        }
        void DirectiveSequencer::enable() {
            ACSDK_DEBUG9(LX("disable"));
            lock_guard<mutex> lock(m_mutex);
            m_isEnabled = true;
            m_directiveProcessor->enable();
            m_wakeReceivingLoop.notify_one();
        }
        void DirectiveSequencer::receivingLoop() {
            auto wake = [this]() { return !m_receivingQueue.empty() || m_isShuttingDown; };
            unique_lock<mutex> lock(m_mutex);
            while(true) {
                m_wakeReceivingLoop.wait(lock, wake);
                if (m_isShuttingDown) break;
                receiveDirectiveLocked(lock);
            }
        }
        void DirectiveSequencer::receiveDirectiveLocked(unique_lock<mutex>& lock) {
            if (m_receivingQueue.empty()) return;
            auto directive = m_receivingQueue.front();
            m_receivingQueue.pop_front();
            lock.unlock();
            if (directive->getName() == "StopCapture" || directive->getName() == "Speak") ACSDK_METRIC_MSG(TAG, directive, Metrics::Location::ADSL_DEQUEUE);
            bool handled = false;
        #ifdef DIALOG_REQUEST_ID_IN_ALL_RESPONSE_DIRECTIVES
            if (directive->getDialogRequestId().empty()) handled = m_directiveRouter.handleDirectiveImmediately(directive);
            else handled = m_directiveProcessor->onDirective(directive);
        #else
            handled = m_directiveProcessor->onDirective(directive);
        #endif
            if (!handled) {
                ACSDK_INFO(LX("sendingExceptionEncountered").d("messageId", directive->getMessageId()));
                m_exceptionSender->sendExceptionEncountered(directive->getUnparsedDirective(),ExceptionErrorType::UNSUPPORTED_OPERATION,
                                              "Unsupported operation");
            }
            lock.lock();
        }
    }
}