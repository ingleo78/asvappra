#ifndef ACSDKALERTS_TIMER_H_
#define ACSDKALERTS_TIMER_H_

#include "Alert.h"

namespace alexaClientSDK {
    namespace acsdkAlerts {
        using namespace std;
        using namespace avsCommon;
        using namespace utils;
        using namespace settings;
        class Timer : public Alert {
        public:
            Timer(function<pair<std::unique_ptr<istream>, const MediaType>()> defaultAudioFactory, function<std::pair<unique_ptr<istream>, const MediaType>()> shortAudioFactory,
                  shared_ptr<DeviceSettingsManager> settingsManager);
            string getTypeName() const override;
            static string getTypeNameStatic() {
                return "TIMER";
            }
        };
    }
}
#endif