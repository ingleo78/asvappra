#ifndef ALEXA_CLIENT_SDK_SAMPLEAPP_AUTHORIZATION_CBLAUTHDELEGATE_INCLUDE_CBLAUTHDELEGATE_CBLAUTHDELEGATE_H_
#define ALEXA_CLIENT_SDK_SAMPLEAPP_AUTHORIZATION_CBLAUTHDELEGATE_INCLUDE_CBLAUTHDELEGATE_CBLAUTHDELEGATE_H_

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_set>
#include <sdkinterfaces/AuthDelegateInterface.h>
#include <sdkinterfaces/AuthObserverInterface.h>
#include <configuration/ConfigurationNode.h>
#include <lib_curl_utils/HttpPostInterface.h>
#include <util/DeviceInfo.h>
#include <util/RetryTimer.h>
#include <registration_manager/CustomerDataHandler.h>
#include "CBLAuthDelegateConfiguration.h"
#include "CBLAuthDelegateStorageInterface.h"
#include "CBLAuthRequesterInterface.h"

namespace alexaClientSDK {
    namespace authorization {
        namespace cblAuthDelegate {
            using namespace std;
            using namespace chrono;
            using namespace avsCommon;
            using namespace sdkInterfaces;
            using namespace registrationManager;
            using namespace utils;
            using namespace configuration;
            using namespace libcurlUtils;
            class CBLAuthDelegate : public AuthDelegateInterface, public CustomerDataHandler {
            public:
                static shared_ptr<AuthDelegateInterface> createAuthDelegateInterface(const shared_ptr<ConfigurationNode>& configuration,
                                                                                     const shared_ptr<CustomerDataManager>& customerDataManager,
                                                                                     const shared_ptr<CBLAuthDelegateStorageInterface>& storage,
                                                                                     const shared_ptr<CBLAuthRequesterInterface>& authRequester,
                                                                                     unique_ptr<HttpPostInterface> httpPost,
                                                                                     const shared_ptr<DeviceInfo>& deviceInfo);
                static unique_ptr<CBLAuthDelegate> create(const ConfigurationNode& configuration, shared_ptr<CustomerDataManager> customerDataManager,
                                                          shared_ptr<CBLAuthDelegateStorageInterface> storage, shared_ptr<CBLAuthRequesterInterface> authRequester,
                                                          shared_ptr<HttpPostInterface> httpPost = nullptr, shared_ptr<DeviceInfo> deviceInfo = nullptr);
                CBLAuthDelegate(const CBLAuthDelegate& rhs) = delete;
                ~CBLAuthDelegate();
                CBLAuthDelegate& operator=(const CBLAuthDelegate& rhs) = delete;
                void addAuthObserver(shared_ptr<AuthObserverInterface> observer) override;
                void removeAuthObserver(shared_ptr<AuthObserverInterface> observer) override;
                string getAuthToken() override;
                void onAuthFailure(const string& token) override;
                void clearData() override;
            private:
                enum class FlowState {
                    STARTING,
                    REQUESTING_CODE_PAIR,
                    REQUESTING_TOKEN,
                    REFRESHING_TOKEN,
                    STOPPING
                };
                CBLAuthDelegate(
                    shared_ptr<CustomerDataManager> customerDataManager,
                    shared_ptr<CBLAuthDelegateStorageInterface> storage,
                    shared_ptr<CBLAuthRequesterInterface> authRequester,
                    shared_ptr<HttpPostInterface> httpPost);
                bool init(
                    const ConfigurationNode& configuration,
                    const shared_ptr<DeviceInfo>& deviceInfo);
                void stop();
                void handleAuthorizationFlow();
                FlowState handleStarting();
                FlowState handleRequestingCodePair();
                FlowState handleRequestingToken();
                FlowState handleRefreshingToken();
                FlowState handleStopping();
                HTTPResponse requestCodePair();
                HTTPResponse requestToken();
                HTTPResponse requestRefresh();
                AuthObserverInterface::Error receiveCodePairResponse(const HTTPResponse& response);
                AuthObserverInterface::Error receiveTokenResponse(const HTTPResponse& response, bool expiresImmediately);
                void setAuthState(avsCommon::sdkInterfaces::AuthObserverInterface::State newState);
                void setAuthError(avsCommon::sdkInterfaces::AuthObserverInterface::Error error);
                void setRefreshToken(const std::string& refreshToken);
                void clearRefreshToken();
                bool isStopping();
                shared_ptr<CBLAuthDelegateStorageInterface> m_storage;
                shared_ptr<HttpPostInterface> m_httpPost;
                shared_ptr<CBLAuthRequesterInterface> m_authRequester;
                unique_ptr<CBLAuthDelegateConfiguration> m_configuration;
                bool m_isStopping;
                mutex m_mutex;
                unordered_set<shared_ptr<AuthObserverInterface>> m_observers;
                string m_accessToken;
                AuthObserverInterface::State m_authState;
                AuthObserverInterface::Error m_authError;
                string m_deviceCode;
                string m_userCode;
                thread m_authorizationFlowThread;
                steady_clock::time_point m_codePairExpirationTime;
                steady_clock::time_point m_tokenExpirationTime;
                string m_refreshToken;
                condition_variable m_wake;
                steady_clock::time_point m_timeToRefresh;
                steady_clock::time_point m_requestTime;
                int m_retryCount;
                bool m_newRefreshToken;
                bool m_authFailureReported;
            };
        }
    }
}
#endif