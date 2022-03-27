#ifndef ALEXA_CLIENT_SDK_REGISTRATIONMANAGER_INCLUDE_REGISTRATIONMANAGER_REGISTRATIONMANAGER_H_
#define ALEXA_CLIENT_SDK_REGISTRATIONMANAGER_INCLUDE_REGISTRATIONMANAGER_REGISTRATIONMANAGER_H_

#include <memory>
#include <unordered_set>
#include <sdkinterfaces/AuthDelegateInterface.h>
#include <sdkinterfaces/AVSConnectionManagerInterface.h>
#include <sdkinterfaces/DirectiveSequencerInterface.h>
#include "CustomerDataManager.h"
#include "RegistrationObserverInterface.h"

namespace alexaClientSDK {
    namespace registrationManager {
        using namespace std;
        using namespace avsCommon;
        using namespace sdkInterfaces;
        class RegistrationManager {
        public:
            RegistrationManager(shared_ptr<DirectiveSequencerInterface> directiveSequencer, shared_ptr<AVSConnectionManagerInterface> connectionManager,
                                shared_ptr<CustomerDataManager> dataManager);
            virtual ~RegistrationManager() = default;
            void logout();
            void addObserver(shared_ptr<RegistrationObserverInterface> observer);
            void removeObserver(shared_ptr<RegistrationObserverInterface> observer);
        private:
            void notifyObservers();
            shared_ptr<DirectiveSequencerInterface> m_directiveSequencer;
            shared_ptr<AVSConnectionManagerInterface> m_connectionManager;
            shared_ptr<CustomerDataManager> m_dataManager;
            mutex m_observersMutex;
            unordered_set<shared_ptr<RegistrationObserverInterface>> m_observers;
        };
    }
}
#endif