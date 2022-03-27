#ifndef ACSDKALERTS_STORAGE_ALERTSTORAGEINTERFACE_H_
#define ACSDKALERTS_STORAGE_ALERTSTORAGEINTERFACE_H_

#include <list>
#include <memory>
#include <string>
#include <vector>
#include <acsdk_alerts/Alert.h>
#include <settings/DeviceSettingsManager.h>

namespace alexaClientSDK {
    namespace acsdkAlerts {
        namespace storage {
            using namespace std;
            using namespace settings;
            class AlertStorageInterface {
            public:
                virtual ~AlertStorageInterface() = default;
                virtual bool createDatabase() = 0;
                virtual bool open() = 0;
                virtual void close() = 0;
                virtual bool store(shared_ptr<Alert> alert) = 0;
                virtual bool load(vector<shared_ptr<Alert>>* alertContainer, shared_ptr<DeviceSettingsManager> settingsManager) = 0;
                virtual bool modify(shared_ptr<Alert> alert) = 0;
                virtual bool erase(shared_ptr<Alert> alert) = 0;
                virtual bool bulkErase(const list<shared_ptr<Alert>>& alertList) = 0;
                virtual bool clearDatabase() = 0;
            };
        }
    }
}
#endif