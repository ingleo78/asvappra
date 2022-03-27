#include <thread>
#include <algorithm>
#include "SystemClockDelay.h"

namespace alexaClientSDK {
    namespace captions {
        void SystemClockDelay::delay(milliseconds milliseconds) {
            auto duration = chrono::milliseconds(max(milliseconds::zero().count(), milliseconds.count()));
            this_thread::sleep_for(duration);
        }
    }
}