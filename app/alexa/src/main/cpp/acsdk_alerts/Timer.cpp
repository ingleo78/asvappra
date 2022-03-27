#include "Timer.h"

namespace alexaClientSDK {
    namespace acsdkAlerts {
        using namespace std;
        using namespace avsCommon;
        using namespace utils;
        using namespace settings;
        Timer::Timer(function<pair<unique_ptr<istream>, const MediaType>()> defaultAudioFactory, function<pair<unique_ptr<istream>, const MediaType>()> shortAudioFactory,
                     shared_ptr<DeviceSettingsManager> settingsManager) :
                Alert(defaultAudioFactory, shortAudioFactory, settingsManager) {
        }
        string Timer::getTypeName() const {
            return Timer::getTypeNameStatic();
        }
    }
}