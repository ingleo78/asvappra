#ifndef ACSDKALERTS_ALARM_H_
#define ACSDKALERTS_ALARM_H_

#include <settings/DeviceSettingsManager.h>
#include "Alert.h"

namespace alexaClientSDK {
    namespace acsdkAlerts {
        using namespace std;
        using namespace settings;
        using namespace utils;
        class Alarm : public Alert {
        public:
            Alarm(function<pair<unique_ptr<istream>, const MediaType>()> defaultAudioFactory, function<pair<unique_ptr<istream>, const MediaType>()> shortAudioFactory,
                  shared_ptr<DeviceSettingsManager> settingsManager);
            std::string getTypeName() const override;
            static std::string getTypeNameStatic() {
                return "ALARM";
            }
        };
    }
}
#endif