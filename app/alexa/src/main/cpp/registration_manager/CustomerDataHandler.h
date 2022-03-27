#ifndef ALEXA_CLIENT_SDK_REGISTRATIONMANAGER_INCLUDE_REGISTRATIONMANAGER_CUSTOMERDATAHANDLER_H_
#define ALEXA_CLIENT_SDK_REGISTRATIONMANAGER_INCLUDE_REGISTRATIONMANAGER_CUSTOMERDATAHANDLER_H_

#include <memory>

namespace alexaClientSDK {
    namespace registrationManager {
        class CustomerDataManager;
        class CustomerDataHandler {
        public:
            CustomerDataHandler(std::shared_ptr<CustomerDataManager> customerDataManager);
            virtual ~CustomerDataHandler();
            virtual void clearData();
        private:
            const std::shared_ptr<CustomerDataManager> m_dataManager;
        };
    }
}
#endif