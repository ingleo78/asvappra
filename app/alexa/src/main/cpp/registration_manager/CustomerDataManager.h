#ifndef ALEXA_CLIENT_SDK_REGISTRATIONMANAGER_INCLUDE_REGISTRATIONMANAGER_CUSTOMERDATAMANAGER_H_
#define ALEXA_CLIENT_SDK_REGISTRATIONMANAGER_INCLUDE_REGISTRATIONMANAGER_CUSTOMERDATAMANAGER_H_

#include <list>
#include <mutex>
#include <unordered_set>
#include "CustomerDataHandler.h"

namespace alexaClientSDK {
    namespace registrationManager {
        using namespace std;
        class CustomerDataManager {
        public:
            virtual ~CustomerDataManager();
            void addDataHandler(CustomerDataHandler* handler);
            void removeDataHandler(CustomerDataHandler* handler);
            void clearData();
        private:
            unordered_set<CustomerDataHandler*> m_dataHandlers;
            mutex m_mutex;
        };
    }
}
#endif