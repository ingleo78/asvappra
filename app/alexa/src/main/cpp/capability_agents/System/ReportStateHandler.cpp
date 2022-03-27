#include <util/error/FinallyGuard.h>
#include <logger/Logger.h>
#include <json/JSONUtils.h>
#include <settings/SettingEventSender.h>
#include "ReportStateHandler.h"

namespace alexaClientSDK {
    namespace capabilityAgents {
        namespace system {
            using namespace logger;
            using namespace error;
            using namespace settings;
            using namespace storage;
            static const string TAG("ReportStateHandler");
            #define LX(event) LogEntry(TAG, event)
            static const string REPORT_STATE_NAMESPACE = "System";
            static const string REPORT_STATE_DIRECTIVE = "ReportState";
            static const string PENDING_REPORT_STATE_KEY = "pendingReportState";
            static const string REPORT_STATE_COMPONENT_NAME = "ReportStateHandler";
            static const string REPORT_STATE_TABLE = "ReportStateTable";
            static const string PENDING_REPORT_VALUE = "true";
            static const SettingEventMetadata REPORT_STATE_METADATA{REPORT_STATE_NAMESPACE,string(),string("StateReport"),
                                                                    string("states")};
            static bool initializeDataBase(MiscStorageInterface& storage) {
                if (!storage.isOpened() && !storage.open() && !storage.isOpened()) {
                    ACSDK_DEBUG3(LX(__func__).m("Couldn't open misc database. Creating."));
                    if (!storage.createDatabase()) {
                        ACSDK_ERROR(LX("initializeDatabaseFailed").d("reason", "Could not create misc database."));
                        return false;
                    }
                }
                bool tableExists = false;
                if (!storage.tableExists(REPORT_STATE_COMPONENT_NAME, REPORT_STATE_TABLE, &tableExists)) {
                    ACSDK_ERROR(LX("initializeDatabaseFailed").d("reason","Could not get Capabilities table information misc database."));
                    return false;
                }
                if (!tableExists) {
                    ACSDK_DEBUG3(LX(__func__).m("Table doesn't exist in misc database. Creating new."));
                    if (!storage.createTable(REPORT_STATE_COMPONENT_NAME, REPORT_STATE_TABLE,MiscStorageInterface::KeyType::STRING_KEY,
                        MiscStorageInterface::ValueType::STRING_VALUE)) {
                        ACSDK_ERROR(LX("initializeDatabaseFailed").d("reason", "Cannot create table").d("component", REPORT_STATE_COMPONENT_NAME)
                            .d("table", REPORT_STATE_TABLE));
                        return false;
                    }
                }
                return true;
            }
            unique_ptr<ReportStateHandler> ReportStateHandler::create(shared_ptr<CustomerDataManager> dataManager,
                                                                      shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender,
                                                                      shared_ptr<AVSConnectionManagerInterface> connectionManager,
                                                                      shared_ptr<MessageSenderInterface> messageSender,
                                                                      shared_ptr<MiscStorageInterface> storage,
                                                                      const vector<StateReportGenerator>& generators) {
                if (!dataManager) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullDataManager"));
                    return nullptr;
                }
                if (!exceptionEncounteredSender) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullExceptionEncountered"));
                    return nullptr;
                }
                if (!storage) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "nullStorage"));
                    return nullptr;
                }
                if (generators.empty()) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "noReportGenerator"));
                    return nullptr;
                }
                if (!initializeDataBase(*storage)) return nullptr;
                auto eventSender = settings::SettingEventSender::create(REPORT_STATE_METADATA, std::move(messageSender));
                bool pendingReport = false;
                if (!storage->tableEntryExists(REPORT_STATE_COMPONENT_NAME, REPORT_STATE_TABLE, PENDING_REPORT_STATE_KEY, &pendingReport)) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "cannotAccessStorage"));
                    return nullptr;
                }
                auto handler = unique_ptr<ReportStateHandler>(new ReportStateHandler(move(dataManager), move(exceptionEncounteredSender),
                                                              move(connectionManager), move(storage), move(eventSender), generators,
                                                              pendingReport));
                handler->initialize();
                return handler;
            }
            DirectiveHandlerConfiguration ReportStateHandler::getConfiguration() const {
                return DirectiveHandlerConfiguration{{NamespaceAndName{REPORT_STATE_NAMESPACE, REPORT_STATE_DIRECTIVE},
                                                      BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false)}};
            }
            void ReportStateHandler::handleDirectiveImmediately(shared_ptr<AVSDirective> directive) {
                if (!directive) {
                    ACSDK_ERROR(LX("handleDirectiveImmediatelyFailed").d("reason", "nullDirective"));
                    return;
                }
                m_executor.submit([this, directive] { handleReportState(*directive); });
            }
            void ReportStateHandler::handleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) {
                if (!info) {
                    ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "nullDirectiveInfo"));
                    return;
                }
                if (!info->directive) {
                    ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "nullDirective"));
                    return;
                }
                m_executor.submit([this, info] {
                    auto ok = handleReportState(*(info->directive));
                    if (info->result) {
                        if (ok) info->result->setCompleted();
                        else info->result->setFailed("HandleReportStateFailed");
                    }
                });
            }
            void ReportStateHandler::preHandleDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) {}
            void ReportStateHandler::cancelDirective(shared_ptr<CapabilityAgent::DirectiveInfo> info) {}
            bool ReportStateHandler::handleReportState(const AVSDirective& info) {
                ACSDK_DEBUG5(LX(__func__));
                FinallyGuard finally{[this, &info] { removeDirective(info.getMessageId()); }};
                if (info.getName() != REPORT_STATE_DIRECTIVE) {
                    ACSDK_ERROR(LX("handleReportStateFailed").d("reason", "unexpectedDirective").d("directive", info.getName()));
                    return false;
                }
                m_pendingReport = true;
                m_storage->put(REPORT_STATE_COMPONENT_NAME, REPORT_STATE_TABLE, PENDING_REPORT_STATE_KEY, PENDING_REPORT_VALUE);
                sendReportState();
                return true;
            }
            ReportStateHandler::ReportStateHandler(shared_ptr<CustomerDataManager> dataManager, shared_ptr<ExceptionEncounteredSenderInterface> exceptionEncounteredSender,
                                                   shared_ptr<AVSConnectionManagerInterface> connectionManager, shared_ptr<MiscStorageInterface> storage,
                                                   unique_ptr<SettingEventSenderInterface> eventSender, const vector<StateReportGenerator>& generators,
                                                   bool pendingReport) : CapabilityAgent{REPORT_STATE_NAMESPACE, move(exceptionEncounteredSender)},
                                                   CustomerDataHandler{move(dataManager)}, m_connectionManager{move(connectionManager)},
                                                   m_storage{move(storage)}, m_generators{generators}, m_eventSender{move(eventSender)},
                                                   m_pendingReport{pendingReport} {}
            void ReportStateHandler::initialize() {
                lock_guard<std::mutex> lock(m_stateMutex);
                m_connectionObserver = SettingConnectionObserver::create([this](bool isConnected) {
                    if (isConnected) {
                        lock_guard<mutex> lock(m_stateMutex);
                        m_executor.submit([this] { sendReportState(); });
                    }
                });
                m_connectionManager->addConnectionStatusObserver(m_connectionObserver);
            }
            ReportStateHandler::~ReportStateHandler() {
                m_executor.shutdown();
                m_connectionManager->removeConnectionStatusObserver(m_connectionObserver);
            }
            void ReportStateHandler::sendReportState() {
                lock_guard<mutex> lock(m_stateMutex);
                ACSDK_DEBUG5(LX(__func__).d("pendingReport", m_pendingReport));
                if (m_pendingReport) {
                    JsonGenerator jsonGenerator;
                    set<string> states;
                    for (auto& generator : m_generators) {
                        auto report = generator.generateReport();
                        states.insert(report.begin(), report.end());
                    }
                    jsonGenerator.addMembersArray(REPORT_STATE_METADATA.settingName, states);
                    auto statesJson = jsonGenerator.toString();
                    if (!m_eventSender->sendStateReportEvent(statesJson).get()) {
                        ACSDK_ERROR(LX("sendReportEventFailed").sensitive("state", statesJson));
                        return;
                    }
                #ifdef ACSDK_EMIT_SENSITIVE_LOGS
                    ACSDK_DEBUG5(LX(__func__).sensitive("state", statesJson));
                #endif
                    m_pendingReport = false;
                    m_storage->remove(REPORT_STATE_COMPONENT_NAME, REPORT_STATE_TABLE, PENDING_REPORT_STATE_KEY);
                }
            }
            void ReportStateHandler::clearData() {
                lock_guard<mutex> lock(m_stateMutex);
                m_storage->clearTable(REPORT_STATE_COMPONENT_NAME, REPORT_STATE_TABLE);
                m_storage->deleteTable(REPORT_STATE_COMPONENT_NAME, REPORT_STATE_TABLE);
            }
            void ReportStateHandler::addStateReportGenerator(const StateReportGenerator& generator) {
                lock_guard<mutex> lock(m_stateMutex);
                m_generators.push_back(generator);
            }
        }
    }
}