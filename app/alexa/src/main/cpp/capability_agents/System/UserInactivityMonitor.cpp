#include <functional>
#include <json/document.h>
#include <json/pointer.h>
#include <json/stringbuffer.h>
#include <json/writer.h>
#include <json/JSONUtils.h>
#include "UserInactivityMonitor.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace system {
            using namespace logger;
            using namespace json;
            using namespace rapidjson;
            static const string TAG("UserInactivityMonitor");
            static const int SECONDS_IN_HOUR = 3600;
            #define LX(event) LogEntry(TAG, event)
            static const string USER_INACTIVITY_MONITOR_NAMESPACE = "System";
            static const string INACTIVITY_EVENT_NAME = "UserInactivityReport";
            static const string INACTIVITY_EVENT_PAYLOAD_KEY = "inactiveTimeInSeconds";
            static const string RESET_DIRECTIVE_NAME = "ResetUserInactivity";
            void UserInactivityMonitor::removeDirectiveGracefully(shared_ptr<DirectiveInfo> info, bool isFailure, const string& report) {
                if (info) {
                    if (info->result) {
                        if (isFailure) info->result->setFailed(report);
                        else info->result->setCompleted();
                        if (info->directive) CapabilityAgent::removeDirective(info->directive->getMessageId());
                    }
                }
            }
            shared_ptr<UserInactivityMonitor> UserInactivityMonitor::create(shared_ptr<MessageSenderInterface> messageSender,
                                                                            shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender,
                                                                            const milliseconds& sendPeriod) {
                if (!messageSender) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullMessageSender"));
                    return nullptr;
                }
                if (!exceptionEncounteredSender) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullExceptionEncounteredSender"));
                    return nullptr;
                }
                return shared_ptr<UserInactivityMonitor>(new UserInactivityMonitor(messageSender, exceptionEncounteredSender, sendPeriod));
            }
            UserInactivityMonitor::UserInactivityMonitor(shared_ptr<MessageSenderInterface> messageSender,
                                                         shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender,
                                                         const milliseconds& sendPeriod) : CapabilityAgent{USER_INACTIVITY_MONITOR_NAMESPACE, exceptionEncounteredSender},
                                                         RequiresShutdown{"UserInactivityMonitor"}, m_messageSender{messageSender},
                                                         m_lastTimeActive{steady_clock::now()}, m_sendPeriod{sendPeriod} {
                startTimer();
            }
            void UserInactivityMonitor::sendInactivityReport() {
                time_point<steady_clock> lastTimeActive;
                m_recentUpdateBlocked = false;
                {
                    lock_guard<mutex> timeLock(m_mutex);
                    lastTimeActive = m_lastTimeActive;
                }
                if (m_recentUpdateBlocked) {
                    lock_guard<mutex> timeLock(m_mutex);
                    m_lastTimeActive = steady_clock::now();
                    lastTimeActive = m_lastTimeActive;
                    return;
                }
                SizeType payloadKeySize = INACTIVITY_EVENT_PAYLOAD_KEY.length();
                const Pointer::Token payloadKey[] = {{INACTIVITY_EVENT_PAYLOAD_KEY.c_str(), payloadKeySize, kPointerInvalidIndex}};
                auto inactiveSeconds = duration_cast<seconds>(steady_clock::now() - lastTimeActive);
                auto inactiveHours = ((inactiveSeconds.count() + SECONDS_IN_HOUR / 2) / SECONDS_IN_HOUR);
                Pointer p{payloadKey, 1};
                Value _inactivityPayload{kObjectType};
                Pointer(payloadKey, 1).Set(_inactivityPayload, (int64_t)(inactiveHours * SECONDS_IN_HOUR));
                std::string inactivityPayloadString;
                jsonUtils::convertToValue(_inactivityPayload, &inactivityPayloadString);
                auto inactivityEvent = buildJsonEventString(INACTIVITY_EVENT_NAME, "", inactivityPayloadString);
                m_messageSender->sendMessage(make_shared<MessageRequest>(inactivityEvent.second));
                notifyObservers();
            }
            void UserInactivityMonitor::startTimer() {
                m_eventTimer.start(m_sendPeriod,Timer::PeriodType::ABSOLUTE, Timer::getForever(),
                              bind(&UserInactivityMonitor::sendInactivityReport, this));
            }
            DirectiveHandlerConfiguration UserInactivityMonitor::getConfiguration() const {
                return DirectiveHandlerConfiguration{{NamespaceAndName{USER_INACTIVITY_MONITOR_NAMESPACE, RESET_DIRECTIVE_NAME},
                                                      BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false)}};
            }
            void UserInactivityMonitor::handleDirectiveImmediately(shared_ptr<AVSDirective> directive) {
                handleDirective(make_shared<DirectiveInfo>(directive, nullptr));
            }
            void UserInactivityMonitor::preHandleDirective(shared_ptr<DirectiveInfo> info) {
            }
            void UserInactivityMonitor::handleDirective(shared_ptr<DirectiveInfo> info) {
                onUserActive();
                removeDirectiveGracefully(info);
            }
            void UserInactivityMonitor::cancelDirective(shared_ptr<DirectiveInfo> info) {
                if (!info->directive) {
                    removeDirectiveGracefully(info, true, "nullDirective");
                    ACSDK_ERROR(LX("cancelDirectiveFailed").d("reason", "nullDirectiveInDirectiveInfo"));
                    return;
                }
                removeDirective(info->directive->getMessageId());
            }
            void UserInactivityMonitor::onUserActive() {
                ACSDK_DEBUG5(LX(__func__));
                std::unique_lock<std::mutex> timeLock(m_mutex, std::defer_lock);
                m_eventTimer.stop();
                startTimer();
                if (timeLock.try_lock()) m_lastTimeActive = std::chrono::steady_clock::now();
                else m_recentUpdateBlocked = true;
            }
            seconds UserInactivityMonitor::timeSinceUserActivity() {
                unique_lock<mutex> lock(m_mutex);
                auto lastTimeActiveCopy = m_lastTimeActive;
                lock.unlock();
                return duration_cast<seconds>(steady_clock::now() - lastTimeActiveCopy);
            }
            void UserInactivityMonitor::addObserver(shared_ptr<UserInactivityMonitorObserverInterface> observer) {
                if (!observer) {
                    ACSDK_ERROR(LX("addObserverFailed").d("reason", "nullObserver"));
                    return;
                }
                lock_guard<mutex> lock(m_mutex);
                m_inactivityObservers.insert(observer);
            }
            void UserInactivityMonitor::removeObserver(shared_ptr<UserInactivityMonitorObserverInterface> observer) {
                if (!observer) {
                    ACSDK_ERROR(LX("removeObserverFailed").d("reason", "nullObserver"));
                    return;
                }
                lock_guard<mutex> lock(m_mutex);
                m_inactivityObservers.erase(observer);
            }
            void UserInactivityMonitor::notifyObservers() {
                unique_lock<mutex> lock(m_mutex);
                auto observers = m_inactivityObservers;
                lock.unlock();
                for (auto observer : observers) {
                    observer->onUserInactivityReportSent();
                }
            }
            void UserInactivityMonitor::doShutdown() {
                lock_guard<mutex> lock{m_mutex};
                m_inactivityObservers.clear();
            }
        }
    }
}