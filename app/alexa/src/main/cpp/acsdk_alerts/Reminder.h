#ifndef ACSDKALERTS_REMINDER_H_
#define ACSDKALERTS_REMINDER_H_

#include "Alert.h"

namespace alexaClientSDK {
    namespace acsdkAlerts {
        using namespace std;
        using namespace settings;
        using namespace avsCommon::utils;
        class Reminder : public Alert {
        public:
            Reminder(function<pair<unique_ptr<istream>, const MediaType>()> defaultAudioFactory, function<pair<unique_ptr<istream>, const MediaType>()> shortAudioFactory,
                     shared_ptr<DeviceSettingsManager> settingsManager);
            string getTypeName() const override;
            static string getTypeNameStatic() {
                return "REMINDER";
            }
        };
    }
}
#endif