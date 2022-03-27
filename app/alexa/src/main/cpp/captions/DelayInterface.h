#ifndef ALEXA_CLIENT_SDK_CAPTIONS_IMPLEMENTATION_INCLUDE_CAPTIONS_DELAYINTERFACE_H_
#define ALEXA_CLIENT_SDK_CAPTIONS_IMPLEMENTATION_INCLUDE_CAPTIONS_DELAYINTERFACE_H_

#include <chrono>

namespace alexaClientSDK {
    namespace captions {
        class DelayInterface {
        public:
            virtual ~DelayInterface() = default;
            virtual void delay(std::chrono::milliseconds milliseconds);
        };
    }
}
#endif