#include <functional>
#include <logger/Logger.h>
#include "Utils/DiscoveryUtils.h"
#include "CapabilitiesDelegate.h"
#include "DiscoveryEventSender.h"
#include "PostConnectCapabilitiesPublisher.h"

namespace alexaClientSDK {
    namespace capabilitiesDelegate {
        using namespace logger;
        using namespace utils::configuration;
        using namespace capabilitiesDelegate::utils;
        static const string TAG("CapabilitiesDelegate");
        #define LX(event) LogEntry(TAG, event)
        static vector<EndpointIdentifier> getEndpointIdentifiers(
            const unordered_map<string, string>& endpoints) {
            vector<string> identifiers;
            if (!endpoints.empty()) {
                identifiers.reserve(endpoints.size());
                for (auto const& endpoint : endpoints) identifiers.push_back(endpoint.first);
            }
            return identifiers;
        }
        shared_ptr<CapabilitiesDelegate> CapabilitiesDelegate::create(
            const shared_ptr<AuthDelegateInterface>& authDelegate,
            const shared_ptr<CapabilitiesDelegateStorageInterface>& capabilitiesDelegateStorage,
            const shared_ptr<CustomerDataManager>& customerDataManager) {
            if (!authDelegate) { ACSDK_ERROR(LX("createFailed").d("reason", "nullAuthDelegate")); }
            else if (!capabilitiesDelegateStorage) { ACSDK_ERROR(LX("createFailed").d("reason", "nullCapabilitiesDelegateStorage")); }
            else if (!customerDataManager) { ACSDK_ERROR(LX("createFailed").d("reason", "nullCustomerDataManager")); }
            else {
                shared_ptr<CapabilitiesDelegate> instance(new CapabilitiesDelegate(authDelegate, capabilitiesDelegateStorage, customerDataManager));
                if (!(instance->init())) {
                    ACSDK_ERROR(LX("createFailed").d("reason", "CapabilitiesDelegateInitFailed"));
                    return nullptr;
                }
                return instance;
            }
            return nullptr;
        }
        CapabilitiesDelegate::CapabilitiesDelegate(const shared_ptr<AuthDelegateInterface>& authDelegate, const shared_ptr<CapabilitiesDelegateStorageInterface>& capabilitiesDelegateStorage,
                                                   const shared_ptr<CustomerDataManager>& customerDataManager) : RequiresShutdown{"CapabilitiesDelegate"},
                                                   CustomerDataHandler{customerDataManager}, m_capabilitiesState{CapabilitiesObserverInterface::State::UNINITIALIZED},
                                                   m_capabilitiesError{CapabilitiesObserverInterface::Error::UNINITIALIZED}, m_authDelegate{authDelegate},
                                                   m_capabilitiesDelegateStorage{capabilitiesDelegateStorage}, m_isConnected{false}, m_isShuttingDown{false} {}
        void CapabilitiesDelegate::doShutdown() {
            ACSDK_DEBUG5(LX(__func__));
            {
                lock_guard<mutex> lock(m_isShuttingDownMutex);
                m_isShuttingDown = true;
            }
            {
                lock_guard<mutex> lock(m_observerMutex);
                m_capabilitiesObservers.clear();
            }
            m_executor.shutdown();
            resetDiscoveryEventSender();
        }
        void CapabilitiesDelegate::clearData() {
            ACSDK_DEBUG5(LX(__func__));
            if (!m_capabilitiesDelegateStorage->clearDatabase()) { ACSDK_ERROR(LX("clearDataFailed").d("reason", "unable to clear database")); }
        }
        bool CapabilitiesDelegate::init() {
            const string errorEvent = "initFailed";
            string infoEvent = "CapabilitiesDelegateInit";
            if (!m_capabilitiesDelegateStorage->open()) {
                ACSDK_INFO(LX(__func__).m("Couldn't open database. Creating."));
                if (!m_capabilitiesDelegateStorage->createDatabase()) {
                    ACSDK_ERROR(LX("initFailed").m("Could not create database"));
                    return false;
                }
            }
            return true;
        }
        void CapabilitiesDelegate::addCapabilitiesObserver(shared_ptr<CapabilitiesObserverInterface> observer) {
            if (!observer) {
                ACSDK_ERROR(LX("addCapabilitiesObserverFailed").d("reason", "nullObserver"));
                return;
            }
            ACSDK_DEBUG5(LX("addCapabilitiesObserver").d("observer", observer.get()));
            {
                lock_guard<mutex> lock(m_observerMutex);
                if (!m_capabilitiesObservers.insert(observer).second) {
                    ACSDK_WARN(LX("addCapabilitiesObserverFailed").d("reason", "observerAlreadyAdded"));
                    return;
                }
            }
            observer->onCapabilitiesStateChange(m_capabilitiesState, m_capabilitiesError, vector<string>{},vector<string>{});
        }
        void CapabilitiesDelegate::removeCapabilitiesObserver(shared_ptr<CapabilitiesObserverInterface> observer) {
            if (!observer) {
                ACSDK_ERROR(LX("removeCapabilitiesObserverFailed").d("reason", "nullObserver"));
                return;
            }
            ACSDK_DEBUG5(LX("removeCapabilitiesObserver").d("observer", observer.get()));
            lock_guard<mutex> lock(m_observerMutex);
            if (m_capabilitiesObservers.erase(observer) == 0) { ACSDK_WARN(LX("removeCapabilitiesObserverFailed").d("reason", "observerNotAdded")); }
        }

        void CapabilitiesDelegate::setCapabilitiesState(const CapabilitiesObserverInterface::State newCapabilitiesState, const CapabilitiesObserverInterface::Error newCapabilitiesError,
                                                        const vector<EndpointIdentifier>& addOrUpdateReportEndpoints, const vector<EndpointIdentifier>& deleteReportEndpoints) {
            ACSDK_DEBUG5(LX("setCapabilitiesState").d("newCapabilitiesState", newCapabilitiesState));
            unordered_set<shared_ptr<CapabilitiesObserverInterface>> capabilitiesObservers;
            {
                lock_guard<mutex> lock(m_observerMutex);
                capabilitiesObservers = m_capabilitiesObservers;
                m_capabilitiesState = newCapabilitiesState;
                m_capabilitiesError = newCapabilitiesError;
            }
            if (!capabilitiesObservers.empty()) {
                ACSDK_DEBUG9(LX("callingOnCapabilitiesStateChange").d("state", m_capabilitiesState).d("error", m_capabilitiesError));
                for (auto& observer : capabilitiesObservers) {
                    observer->onCapabilitiesStateChange(newCapabilitiesState, newCapabilitiesError, addOrUpdateReportEndpoints, deleteReportEndpoints);
                }
            }
        }
        void CapabilitiesDelegate::setMessageSender(const shared_ptr<MessageSenderInterface>& messageSender) {
            ACSDK_DEBUG5(LX(__func__));
            if (!messageSender) {
                ACSDK_ERROR(LX("setMessageSenderFailed").d("reason", "Null messageSender"));
                return;
            }
            lock_guard<mutex> lock(m_messageSenderMutex);
            m_messageSender = messageSender;
        }
        void CapabilitiesDelegate::invalidateCapabilities() {
            ACSDK_DEBUG5(LX(__func__));
            if (!m_capabilitiesDelegateStorage->clearDatabase()) {
                ACSDK_ERROR(LX("invalidateCapabilitiesFailed").d("reason", "unable to clear database"));
            }
        }
        bool CapabilitiesDelegate::addOrUpdateEndpoint(const AVSDiscoveryEndpointAttributes& endpointAttributes, const vector<CapabilityConfiguration>& capabilities) {
            ACSDK_DEBUG5(LX(__func__));
            if (!validateEndpointAttributes(endpointAttributes)) {
                ACSDK_ERROR(LX("addOrUpdateEndpointFailed").d("reason", "invalidAVSDiscoveryEndpointAttributes"));
                return false;
            }
            if (capabilities.empty()) {
                ACSDK_ERROR(LX("addOrUpdateEndpointFailed").d("reason", "invalidCapabilities"));
                return false;
            }
            for (auto& capability : capabilities) {
                if (!validateCapabilityConfiguration(capability)) {
                    ACSDK_ERROR(LX("addOrUpdateEndpointFailed").d("reason", "invalidCapabilityConfiguration").d("capability", capability.interfaceName));
                    return false;
                }
            }
            string endpointId = endpointAttributes.endpointId;
            {
                lock_guard<mutex> lock{m_endpointsMutex};
                auto it = m_deleteEndpoints.pending.find(endpointId);
                if (m_deleteEndpoints.pending.end() != it) {
                    ACSDK_ERROR(LX("addOrUpdateEndpointFailed").d("reason", "already pending deletion").sensitive("endpointId", endpointId));
                    return false;
                }
                it = m_addOrUpdateEndpoints.pending.find(endpointId);
                if (m_addOrUpdateEndpoints.pending.end() != it) {
                    ACSDK_ERROR(LX("addOrUpdateEndpointFailed").d("reason", "already pending addOrUpdate").sensitive("endpointId", endpointId));
                    return false;
                }
                string endpointConfigJson = getEndpointConfigJson(endpointAttributes, capabilities);
                m_addOrUpdateEndpoints.pending.insert(make_pair(endpointId, endpointConfigJson));
            }
            if (!m_currentDiscoveryEventSender) m_executor.submit([this] { executeSendPendingEndpoints(); });
            return true;
        }
        bool CapabilitiesDelegate::deleteEndpoint(const AVSDiscoveryEndpointAttributes& endpointAttributes, const vector<CapabilityConfiguration>& capabilities) {
            ACSDK_DEBUG5(LX(__func__));
            if (!validateEndpointAttributes(endpointAttributes)) {
                ACSDK_ERROR(LX("deleteEndpointFailed").d("reason", "invalidAVSDiscoveryEndpointAttributes"));
                return false;
            }
            if (capabilities.empty()) {
                ACSDK_ERROR(LX("deleteEndpointFailed").d("reason", "invalidCapabilities"));
                return false;
            }
            for (auto& capability : capabilities) {
                if (!validateCapabilityConfiguration(capability)) {
                    ACSDK_ERROR(LX("deleteEndpointFailed").d("reason", "invalidCapabilityConfiguration").d("capability", capability.interfaceName));
                    return false;
                }
            }
            string endpointId = endpointAttributes.endpointId;
            {
                lock_guard<std::mutex> lock{m_endpointsMutex};
                auto it = m_addOrUpdateEndpoints.pending.find(endpointId);
                if (m_addOrUpdateEndpoints.pending.end() != it) {
                    ACSDK_ERROR(LX(__func__).d("deleteEndpointFailed", "already pending registration").sensitive("endpointId", endpointId));
                    return false;
                }
                it = m_endpoints.find(endpointId);
                if (m_endpoints.end() == it) {
                    ACSDK_ERROR(LX("deleteEndpointFailed").d("reason", "endpoint not registered").sensitive("endpointId", endpointId));
                    return false;
                }
                it = m_deleteEndpoints.pending.find(endpointId);
                if (m_deleteEndpoints.pending.end() != it) {
                    ACSDK_ERROR(LX("deleteEndpointFailed").d("reason", "already pending deletion").sensitive("endpointId", endpointId));
                    return false;
                }
                string endpointConfigJson = getEndpointConfigJson(endpointAttributes, capabilities);
                m_deleteEndpoints.pending.insert(make_pair(endpointId, endpointConfigJson));
            }
            if (!m_currentDiscoveryEventSender) m_executor.submit([this] { executeSendPendingEndpoints(); });
            return true;
        }
        void CapabilitiesDelegate::executeSendPendingEndpoints() {
            ACSDK_DEBUG5(LX(__func__));
            if (isShuttingDown()) {
                ACSDK_DEBUG5(LX(__func__).d("Skipped", "Shutting down"));
                return;
            }
            if (!m_isConnected) {
                ACSDK_DEBUG5(LX(__func__).d("Deferred", "Not connected"));
                return;
            }
            if (m_currentDiscoveryEventSender) {
                ACSDK_DEBUG5(LX(__func__).d("Deferred", "Discovery events already in-flight"));
                return;
            }
            if (m_addOrUpdateEndpoints.pending.empty() && m_deleteEndpoints.pending.empty()) {
                ACSDK_DEBUG5(LX(__func__).d("Skipped", "No endpoints to register or delete"));
                return;
            }
            unordered_map<string, string> addOrUpdateEndpointsToSend;
            unordered_map<string, string> deleteEndpointsToSend;
            {
                lock_guard<mutex> lock{m_endpointsMutex};
                m_addOrUpdateEndpoints.inFlight = m_addOrUpdateEndpoints.pending;
                addOrUpdateEndpointsToSend = m_addOrUpdateEndpoints.inFlight;
                m_addOrUpdateEndpoints.pending.clear();
                m_deleteEndpoints.inFlight = m_deleteEndpoints.pending;
                deleteEndpointsToSend = m_deleteEndpoints.inFlight;
                m_deleteEndpoints.pending.clear();
            }
            ACSDK_DEBUG5(LX(__func__).d("num endpoints to add", addOrUpdateEndpointsToSend.size()).d("num endpoints to delete", deleteEndpointsToSend.size()));
            auto discoveryEventSender = DiscoveryEventSender::create(addOrUpdateEndpointsToSend, deleteEndpointsToSend, m_authDelegate);
            if (!discoveryEventSender) {
                ACSDK_ERROR(LX("failedExecuteSendPendingEndpoints").d("reason", "failed to create DiscoveryEventSender"));
                moveInFlightEndpointsToPending();
                setCapabilitiesState(CapabilitiesObserverInterface::State::FATAL_ERROR,CapabilitiesObserverInterface::Error::UNKNOWN_ERROR,
                                     getEndpointIdentifiers(addOrUpdateEndpointsToSend), getEndpointIdentifiers(deleteEndpointsToSend));
                return;
            }
            addDiscoveryEventSender(discoveryEventSender);
            discoveryEventSender->sendDiscoveryEvents(m_messageSender);
        }
        void CapabilitiesDelegate::onAlexaEventProcessedReceived(const string& eventCorrelationToken) {
            ACSDK_DEBUG5(LX(__func__));
            shared_ptr<DiscoveryEventSenderInterface> currentEventSender;
            {
                lock_guard<mutex> lock{m_currentDiscoveryEventSenderMutex};
                currentEventSender = m_currentDiscoveryEventSender;
            }
            if (currentEventSender) currentEventSender->onAlexaEventProcessedReceived(eventCorrelationToken);
            else { ACSDK_ERROR(LX(__func__).m("invalidDiscoveryEventSender")); }
        }
        shared_ptr<PostConnectOperationInterface> CapabilitiesDelegate::createPostConnectOperation() {
            ACSDK_DEBUG5(LX(__func__));
            resetDiscoveryEventSender();
            unordered_map<string, string> originalPendingAddOrUpdateEndpoints;
            unordered_map<string, string> addOrUpdateEndpointsToSend;
            unordered_map<string, string> deleteEndpointsToSend;
            {
                lock_guard<mutex> lock{m_endpointsMutex};
                moveInFlightEndpointsToPendingLocked();
                originalPendingAddOrUpdateEndpoints = m_addOrUpdateEndpoints.pending;
                unordered_map<string, string> storedEndpointConfig;
                if (!m_capabilitiesDelegateStorage->load(&storedEndpointConfig)) {
                    ACSDK_ERROR(LX("createPostConnectOperationFailed").m("Could not load previous config from database."));
                    return nullptr;
                }
                ACSDK_DEBUG5(LX(__func__).d("num endpoints stored", storedEndpointConfig.size()));
                if (storedEndpointConfig.empty()) {
                    for (auto& endpoint : m_endpoints) {
                        auto endpointId = endpoint.first;
                        auto it = m_deleteEndpoints.pending.find(endpointId);
                        if (m_deleteEndpoints.pending.end() != it) continue;
                        it = m_addOrUpdateEndpoints.pending.find(endpointId);
                        if (m_addOrUpdateEndpoints.pending.end() == it) m_addOrUpdateEndpoints.pending[endpointId] = endpoint.second;
                    }
                } else {
                    filterUnchangedPendingAddOrUpdateEndpointsLocked(&storedEndpointConfig);
                    addStaleEndpointsToPendingDeleteLocked(&storedEndpointConfig);
                }
                m_addOrUpdateEndpoints.inFlight = m_addOrUpdateEndpoints.pending;
                addOrUpdateEndpointsToSend = m_addOrUpdateEndpoints.inFlight;
                m_addOrUpdateEndpoints.pending.clear();
                m_deleteEndpoints.inFlight = m_deleteEndpoints.pending;
                deleteEndpointsToSend = m_deleteEndpoints.inFlight;
                m_deleteEndpoints.pending.clear();
            }
            if (addOrUpdateEndpointsToSend.empty() && !originalPendingAddOrUpdateEndpoints.empty()) {
                setCapabilitiesState(CapabilitiesObserverInterface::State::SUCCESS,CapabilitiesObserverInterface::Error::SUCCESS,
                                     getEndpointIdentifiers(originalPendingAddOrUpdateEndpoints),vector<string>{});
            }
            if (addOrUpdateEndpointsToSend.empty() && deleteEndpointsToSend.empty()) {
                ACSDK_DEBUG5(LX(__func__).m("No change in Capabilities, skipping post connect step"));
                return nullptr;
            }
            ACSDK_DEBUG5(LX(__func__).d("num endpoints to add", addOrUpdateEndpointsToSend.size()).d("num endpoints to delete", deleteEndpointsToSend.size()));
            shared_ptr<DiscoveryEventSenderInterface> currentEventSender = DiscoveryEventSender::create(addOrUpdateEndpointsToSend, deleteEndpointsToSend, m_authDelegate);
            if (currentEventSender) addDiscoveryEventSender(currentEventSender);
            else {
                ACSDK_ERROR(LX("createPostConnectOperationFailed").m("Could not create DiscoveryEventSender."));
                return nullptr;
            }
            auto instance = PostConnectCapabilitiesPublisher::create(currentEventSender);
            if (!instance) resetDiscoveryEventSender();
            return instance;
        }
        void CapabilitiesDelegate::onDiscoveryCompleted(const unordered_map<string, string>& addOrUpdateReportEndpoints, const unordered_map<string, string>& deleteReportEndpoints) {
            ACSDK_DEBUG5(LX(__func__));
            if (m_addOrUpdateEndpoints.inFlight != addOrUpdateReportEndpoints ||
                m_deleteEndpoints.inFlight != deleteReportEndpoints) {
                ACSDK_WARN(LX(__func__).m("Cached in-flight endpoints do not match endpoints registered to AVS"));
            }
            auto addOrUpdateReportEndpointIdentifiers = getEndpointIdentifiers(addOrUpdateReportEndpoints);
            auto deleteReportEndpointIdentifiers = getEndpointIdentifiers(deleteReportEndpoints);
            if (!updateEndpointConfigInStorage(addOrUpdateReportEndpoints, deleteReportEndpoints)) {
                ACSDK_ERROR(LX("publishCapabilitiesFailed").d("reason", "failed to save endpointConfig to database"));
                setCapabilitiesState(CapabilitiesObserverInterface::State::FATAL_ERROR,CapabilitiesObserverInterface::Error::UNKNOWN_ERROR,
                                     addOrUpdateReportEndpointIdentifiers, deleteReportEndpointIdentifiers);
                return;
            }
            moveInFlightEndpointsToRegisteredEndpoints();
            resetDiscoveryEventSender();
            setCapabilitiesState(CapabilitiesObserverInterface::State::SUCCESS,CapabilitiesObserverInterface::Error::SUCCESS,
                                 addOrUpdateReportEndpointIdentifiers, deleteReportEndpointIdentifiers);
            m_executor.submit([this] { executeSendPendingEndpoints(); });
        }
        void CapabilitiesDelegate::onDiscoveryFailure(MessageRequestObserverInterface::Status status) {
            ACSDK_DEBUG5(LX(__func__));
            if (status == MessageRequestObserverInterface::Status::SUCCESS_ACCEPTED) {
                ACSDK_ERROR(LX(__func__).d("reason", "invalidStatus").d("status", status));
                return;
            }
            ACSDK_ERROR(LX(__func__).d("reason", status));
            auto addOrUpdateReportEndpointIdentifiers = getEndpointIdentifiers(m_addOrUpdateEndpoints.inFlight);
            auto deleteReportEndpointIdentifiers = getEndpointIdentifiers(m_deleteEndpoints.inFlight);
            switch(status) {
                case MessageRequestObserverInterface::Status::INVALID_AUTH:
                    m_addOrUpdateEndpoints.inFlight.clear();
                    m_deleteEndpoints.inFlight.clear();
                    resetDiscoveryEventSender();
                    setCapabilitiesState(CapabilitiesObserverInterface::State::FATAL_ERROR,CapabilitiesObserverInterface::Error::FORBIDDEN,
                                         addOrUpdateReportEndpointIdentifiers, deleteReportEndpointIdentifiers);
                    break;
                case MessageRequestObserverInterface::Status::BAD_REQUEST:
                    m_addOrUpdateEndpoints.inFlight.clear();
                    m_deleteEndpoints.inFlight.clear();
                    resetDiscoveryEventSender();
                    setCapabilitiesState(CapabilitiesObserverInterface::State::FATAL_ERROR,CapabilitiesObserverInterface::Error::BAD_REQUEST,
                                         addOrUpdateReportEndpointIdentifiers, deleteReportEndpointIdentifiers);
                    break;
                case MessageRequestObserverInterface::Status::SERVER_INTERNAL_ERROR_V2:
                    if (isShuttingDown()) resetDiscoveryEventSender();
                    setCapabilitiesState(CapabilitiesObserverInterface::State::RETRIABLE_ERROR,CapabilitiesObserverInterface::Error::SERVER_INTERNAL_ERROR,
                                         addOrUpdateReportEndpointIdentifiers, deleteReportEndpointIdentifiers);
                    break;
                default:
                    if (isShuttingDown()) resetDiscoveryEventSender();
                    setCapabilitiesState(CapabilitiesObserverInterface::State::RETRIABLE_ERROR,CapabilitiesObserverInterface::Error::UNKNOWN_ERROR,
                                         addOrUpdateReportEndpointIdentifiers, deleteReportEndpointIdentifiers);
                    break;
            }
        }
        bool CapabilitiesDelegate::updateEndpointConfigInStorage(const unordered_map<string, string>& addOrUpdateReportEndpoints,
                                                                 const unordered_map<string, string>& deleteReportEndpoints) {
            ACSDK_DEBUG5(LX(__func__));
            if (!m_capabilitiesDelegateStorage) {
                ACSDK_ERROR(LX("updateEndpointConfigInStorageLockedFailed").d("reason", "invalidStorage"));
                return false;
            }
            if (!m_capabilitiesDelegateStorage->store(addOrUpdateReportEndpoints)) {
                ACSDK_ERROR(LX("updateStorageFailed").d("reason", "storeFailed"));
                return false;
            }
            if (!m_capabilitiesDelegateStorage->erase(deleteReportEndpoints)) {
                ACSDK_ERROR(LX("updateStorageFailed").d("reason", "eraseFailed"));
                return false;
            }
            return true;
        }
        void CapabilitiesDelegate::addDiscoveryEventSender(const shared_ptr<DiscoveryEventSenderInterface>& discoveryEventSender) {
            ACSDK_DEBUG5(LX(__func__));
            if (!discoveryEventSender) {
                ACSDK_ERROR(LX("addDiscoveryEventSenderFailed").d("reason", "invalid sender"));
                return;
            }
            resetDiscoveryEventSender();
            lock_guard<mutex> lock{m_currentDiscoveryEventSenderMutex};
            discoveryEventSender->addDiscoveryStatusObserver(shared_from_this());
            m_currentDiscoveryEventSender = discoveryEventSender;
        }
        void CapabilitiesDelegate::resetDiscoveryEventSender() {
            ACSDK_DEBUG5(LX(__func__));
            shared_ptr<DiscoveryEventSenderInterface> currentDiscoveryEventSender;
            {
                lock_guard<mutex> lock{m_currentDiscoveryEventSenderMutex};
                currentDiscoveryEventSender = m_currentDiscoveryEventSender;
                m_currentDiscoveryEventSender.reset();
            }
            if (currentDiscoveryEventSender) {
                currentDiscoveryEventSender->removeDiscoveryStatusObserver(shared_from_this());
                currentDiscoveryEventSender->stop();
            }
        }
        void CapabilitiesDelegate::onAVSGatewayChanged(const string& avsGateway) {
            ACSDK_DEBUG5(LX(__func__));
            invalidateCapabilities();
        }
        void CapabilitiesDelegate::onConnectionStatusChanged(const ConnectionStatusObserverInterface::Status status, const ConnectionStatusObserverInterface::ChangedReason reason) {
            ACSDK_DEBUG5(LX(__func__).d("connectionStatus", status));
            {
                lock_guard<mutex> lock(m_isConnectedMutex);
                m_isConnected = (ConnectionStatusObserverInterface::Status::CONNECTED == status);
            }
            if (ConnectionStatusObserverInterface::Status::CONNECTED == status) m_executor.submit([this] { executeSendPendingEndpoints(); });
        }
        void CapabilitiesDelegate::moveInFlightEndpointsToRegisteredEndpoints() {
            lock_guard<mutex> lock(m_endpointsMutex);
            for (const auto& inFlight : m_addOrUpdateEndpoints.inFlight) m_endpoints[inFlight.first] = inFlight.second;
            for (const auto& inFlight : m_deleteEndpoints.inFlight) m_endpoints.erase(inFlight.first);
            m_addOrUpdateEndpoints.inFlight.clear();
            m_deleteEndpoints.inFlight.clear();
        }
        void CapabilitiesDelegate::addStaleEndpointsToPendingDeleteLocked(
            unordered_map<string, string>* storedEndpointConfig) {
            ACSDK_DEBUG5(LX(__func__));
            if (!storedEndpointConfig) {
                ACSDK_ERROR(LX("findEndpointsToDeleteLockedFailed").d("reason", "invalidStoredEndpointConfig"));
                return;
            }
            for (auto& it : *storedEndpointConfig) {
                if (m_endpoints.end() == m_endpoints.find(it.first) &&
                    m_addOrUpdateEndpoints.pending.end() == m_addOrUpdateEndpoints.pending.find(it.first)) {
                    ACSDK_DEBUG9(LX(__func__).d("step", "endpoint included in deleteReport").sensitive("endpointId", it.first));
                    m_deleteEndpoints.pending.insert({it.first, getDeleteReportEndpointConfigJson(it.first)});
                }
            }
        }
        void CapabilitiesDelegate::filterUnchangedPendingAddOrUpdateEndpointsLocked(unordered_map<string, string>* storedEndpointConfig) {
            ACSDK_DEBUG5(LX(__func__));
            if (!storedEndpointConfig) {
                ACSDK_ERROR(LX("filterUnchangedPendingAddOrUpdateEndpointsLockedFailed").d("reason", "invalidStoredEndpointConfig"));
                return;
            }
            unordered_map<string, string> addOrUpdateEndpointIdToConfigPairs = m_addOrUpdateEndpoints.pending;
            for (auto& endpointIdToConfigPair : addOrUpdateEndpointIdToConfigPairs) {
                auto storedEndpointConfigId = storedEndpointConfig->find(endpointIdToConfigPair.first);
                if (storedEndpointConfig->end() != storedEndpointConfigId) {
                    if (endpointIdToConfigPair.second == storedEndpointConfigId->second) {
                        ACSDK_DEBUG9(LX(__func__).d("step", "endpoint not be included in addOrUpdateReport").sensitive("endpointId", endpointIdToConfigPair.first));
                        m_addOrUpdateEndpoints.pending.erase(endpointIdToConfigPair.first);
                        m_endpoints[endpointIdToConfigPair.first] = endpointIdToConfigPair.second;
                        storedEndpointConfig->erase(endpointIdToConfigPair.first);
                    } else {
                        ACSDK_DEBUG9(LX(__func__).d("step", "endpoint included in addOrUpdateReport").d("reason", "configuration changed")
                            .sensitive("endpointId", endpointIdToConfigPair.first));
                    }
                } else {
                    ACSDK_DEBUG9(LX(__func__).d("step", "endpoint included in addOrUpdateReport").d("reason", "new").sensitive("endpointId", endpointIdToConfigPair.first));
                }
            }
        }
        void CapabilitiesDelegate::moveInFlightEndpointsToPendingLocked() {
            ACSDK_DEBUG5(LX(__func__));
            for (auto& inFlightEndpointIdToConfigPair : m_addOrUpdateEndpoints.inFlight) {
                if (m_addOrUpdateEndpoints.pending.end() !=
                    m_addOrUpdateEndpoints.pending.find(inFlightEndpointIdToConfigPair.first)) {
                    ACSDK_ERROR(LX(__func__).d("moveInFlightEndpointToPendingFailed", "Unexpected duplicate endpointId in pending")
                        .sensitive("endpointId", inFlightEndpointIdToConfigPair.first));
                } else {
                    ACSDK_DEBUG9(LX(__func__).d("step", "willRetryInFlightAddOrUpdateEndpoint").sensitive("endpointId", inFlightEndpointIdToConfigPair.first)
                        .sensitive("configuration", inFlightEndpointIdToConfigPair.second));
                    m_addOrUpdateEndpoints.pending.insert(inFlightEndpointIdToConfigPair);
                }
            }
            for (auto& inFlightEndpointIdToConfigPair : m_deleteEndpoints.inFlight) {
                auto inFlightEndpointIdToConfigPairId = m_deleteEndpoints.pending.find(inFlightEndpointIdToConfigPair.first);
                if (m_deleteEndpoints.pending.end() != inFlightEndpointIdToConfigPairId) {
                    ACSDK_ERROR(LX(__func__).d("moveInFlightEndpointToPendingFailed", "Unexpected duplicate endpointId in pending")
                        .sensitive("endpointId", inFlightEndpointIdToConfigPair.first));
                } else {
                    ACSDK_DEBUG9(LX(__func__).d("step", "willRetryDeletingInFlightEndpoint").sensitive("endpointId", inFlightEndpointIdToConfigPair.first));
                    m_deleteEndpoints.pending.insert(inFlightEndpointIdToConfigPair);
                }
            }
            m_addOrUpdateEndpoints.inFlight.clear();
            m_deleteEndpoints.inFlight.clear();
        }
        void CapabilitiesDelegate::moveInFlightEndpointsToPending() {
            ACSDK_DEBUG5(LX(__func__));
            lock_guard<mutex> lock(m_endpointsMutex);
            moveInFlightEndpointsToPendingLocked();
        }
        bool CapabilitiesDelegate::isShuttingDown() {
            lock_guard<mutex> lock(m_isShuttingDownMutex);
            return m_isShuttingDown;
        }
    }
}