#ifndef ALEXA_CLIENT_SDK_CAPTIONS_IMPLEMENTATION_INCLUDE_CAPTIONS_SYSTEMCLOCKDELAY_H_
#define ALEXA_CLIENT_SDK_CAPTIONS_IMPLEMENTATION_INCLUDE_CAPTIONS_SYSTEMCLOCKDELAY_H_

#include "DelayInterface.h"

namespace alexaClientSDK {
    namespace captions {
        using namespace std;
        using namespace chrono;
        class SystemClockDelay : public DelayInterface {
        public:
            void delay(milliseconds milliseconds) override;
        };
    }
}
#endif