#include <functional>
#include <logger/Logger.h>
#include "AVSGatewayManager.h"
#include "PostConnectVerifyGatewaySender.h"

namespace alexaClientSDK {
    namespace avsGatewayManager {
        using namespace std;
        using namespace avsCommon;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace configuration;
        using namespace logger;
        using namespace registrationManager;
        using namespace storage;
        static const string TAG("AVSGatewayManager");
        #define LX(event) LogEntry(TAG, event)
        static const string AVS_GATEWAY_MANAGER_ROOT_KEY = "avsGatewayManager";
        static const string AVS_GATEWAY = "avsGateway";
        static const string DEFAULT_AVS_GATEWAY = "https://alexa.na.gateway.devices.a2z.com";
        shared_ptr<AVSGatewayManager> AVSGatewayManager::create(shared_ptr<AVSGatewayManagerStorageInterface> avsGatewayManagerStorage,
                                                                shared_ptr<CustomerDataManager> customerDataManager, const ConfigurationNode& configurationRoot) {
            ACSDK_DEBUG5(LX(__func__));
            if (!avsGatewayManagerStorage) { ACSDK_ERROR(LX("createFailed").d("reason", "nullAvsGatewayManagerStorage")); }
            else {
                string avsGateway;
                auto avsGatewayManagerConfig = configurationRoot[AVS_GATEWAY_MANAGER_ROOT_KEY];
                if (avsGatewayManagerConfig.getString(AVS_GATEWAY, &avsGateway, DEFAULT_AVS_GATEWAY)) {
                    ACSDK_DEBUG5(LX(__func__).d("default AVS Gateway from config", avsGateway));
                }
                if (avsGateway.empty()) avsGateway = DEFAULT_AVS_GATEWAY;
                auto avsGatewayManager = shared_ptr<AVSGatewayManager>(new AVSGatewayManager(avsGatewayManagerStorage, customerDataManager, avsGateway));
                if (avsGatewayManager->init()) return avsGatewayManager;
                else { ACSDK_ERROR(LX("createFailed").d("reason", "initializationFailed")); }
            }
            return nullptr;
        }
        AVSGatewayManager::AVSGatewayManager(shared_ptr<AVSGatewayManagerStorageInterface> avsGatewayManagerStorage, shared_ptr<CustomerDataManager> customerDataManager,
                                             const string& defaultAVSGateway) : CustomerDataHandler{customerDataManager}, m_avsGatewayStorage{avsGatewayManagerStorage},
                                             m_currentState{defaultAVSGateway, false} {}
        AVSGatewayManager::~AVSGatewayManager() {
            ACSDK_DEBUG5(LX(__func__));
            lock_guard<std::mutex> lock{m_mutex};
            m_observers.clear();
        }
        shared_ptr<PostConnectOperationInterface> AVSGatewayManager::createPostConnectOperation() {
            lock_guard<std::mutex> m_lock{m_mutex};
            ACSDK_DEBUG5(LX(__func__));
            if (!m_currentState.isVerified) {
                auto callback = bind(&AVSGatewayManager::onGatewayVerified, this, placeholders::_1);
                m_currentVerifyGatewaySender = PostConnectVerifyGatewaySender::create(callback);
                return m_currentVerifyGatewaySender;
            }
            ACSDK_DEBUG5(LX(__func__).m("Gateway already verified, skipping gateway verification step"));
            return nullptr;
        }
        bool AVSGatewayManager::init() {
            lock_guard<std::mutex> m_lock{m_mutex};
            if (!m_avsGatewayStorage->init()) {
                ACSDK_ERROR(LX("initFailed").d("reason", "unable to initialize gateway storage."));
                return false;
            }
            if (!m_avsGatewayStorage->loadState(&m_currentState)) {
                ACSDK_ERROR(LX("initFailed").d("reason", "unable to load gateway verification state"));
                return false;
            }
            ACSDK_DEBUG5(LX("init").d("avsGateway", m_currentState.avsGatewayURL));
            return true;
        }
        bool AVSGatewayManager::setAVSGatewayAssigner(std::shared_ptr<AVSGatewayAssignerInterface> avsGatewayAssigner) {
            ACSDK_DEBUG5(LX(__func__));
            lock_guard<mutex> m_lock{m_mutex};
            if (!avsGatewayAssigner) {
                ACSDK_ERROR(LX("setAVSGatewayAssignerFailed").d("reason", "nullAvsGatewayAssigner"));
                return false;
            }
            m_avsGatewayAssigner = avsGatewayAssigner;
            m_avsGatewayAssigner->setAVSGateway(m_currentState.avsGatewayURL);
            return true;
        }
        string AVSGatewayManager::getGatewayURL() const {
            lock_guard<mutex> lock{m_mutex};
            return m_currentState.avsGatewayURL;
        }
        bool AVSGatewayManager::setGatewayURL(const string& avsGatewayURL) {
            ACSDK_DEBUG5(LX(__func__).d("avsGateway", avsGatewayURL));
            if (avsGatewayURL.empty()) {
                ACSDK_ERROR(LX("setGatewayURLFailed").d("reason", "empty avsGatewayURL"));
                return false;
            };
            unordered_set<shared_ptr<AVSGatewayObserverInterface>> observersCopy;
            shared_ptr<AVSGatewayAssignerInterface> avsGatewayAssignerCopy;
            {
                lock_guard<mutex> lock{m_mutex};
                if (avsGatewayURL != m_currentState.avsGatewayURL) {
                    m_currentState = GatewayVerifyState{avsGatewayURL, false};
                    saveStateLocked();
                    observersCopy = m_observers;
                    avsGatewayAssignerCopy = m_avsGatewayAssigner;
                } else {
                    ACSDK_ERROR(LX("setGatewayURLFailed").d("reason", "no change in URL"));
                    return false;
                }
            }
            if (avsGatewayAssignerCopy) avsGatewayAssignerCopy->setAVSGateway(avsGatewayURL);
            else { ACSDK_WARN(LX("setGatewayURLWARN").d("reason", "invalid AVSGatewayAssigner")); }
            if (!observersCopy.empty()) {
                for (auto& observer : observersCopy) observer->onAVSGatewayChanged(avsGatewayURL);
            }
            return true;
        }
        void AVSGatewayManager::onGatewayVerified(const shared_ptr<PostConnectOperationInterface>& verifyGatewaySender) {
            ACSDK_DEBUG5(LX(__func__));
            lock_guard<mutex> lock{m_mutex};
            if (m_currentVerifyGatewaySender == verifyGatewaySender && !m_currentState.isVerified) {
                m_currentState.isVerified = true;
                saveStateLocked();
            }
        }
        bool AVSGatewayManager::saveStateLocked() {
            ACSDK_DEBUG5(LX(__func__).d("gateway", m_currentState.avsGatewayURL).d("isVerified", m_currentState.isVerified));
            if (!m_avsGatewayStorage->saveState(m_currentState)) {
                ACSDK_ERROR(LX("saveStateLockedFailed").d("reason", "unable to save to database"));
                return false;
            }
            return true;
        }
        void AVSGatewayManager::addObserver(std::shared_ptr<AVSGatewayObserverInterface> observer) {
            ACSDK_DEBUG5(LX(__func__));
            if (!observer) {
                ACSDK_ERROR(LX("addObserverFailed").d("reason", "nullObserver"));
                return;
            }
            {
                lock_guard<mutex> lock{m_mutex};
                if (!m_observers.insert(observer).second) {
                    ACSDK_ERROR(LX("addObserverFailed").d("reason", "observer already added!"));
                    return;
                }
            }
        }
        void AVSGatewayManager::removeObserver(shared_ptr<AVSGatewayObserverInterface> observer) {
            ACSDK_DEBUG5(LX(__func__));
            if (!observer) {
                ACSDK_ERROR(LX("addObserverFailed").d("reason", "nullObserver"));
                return;
            }
            {
                lock_guard<mutex> lock{m_mutex};
                if (!m_observers.erase(observer)) {
                    ACSDK_ERROR(LX("removeObserverFailed").d("reason", "observer not found"));
                    return;
                }
            }
        }
        void AVSGatewayManager::clearData() {
            ACSDK_DEBUG5(LX(__func__));
            lock_guard<mutex> lock{m_mutex};
            m_avsGatewayStorage->clear();
        }
    }
}