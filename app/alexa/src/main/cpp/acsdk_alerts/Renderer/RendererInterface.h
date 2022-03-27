#ifndef ACSDKALERTS_RENDERER_RENDERERINTERFACE_H_
#define ACSDKALERTS_RENDERER_RENDERERINTERFACE_H_

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <util/MediaType.h>
#include "RendererObserverInterface.h"

namespace alexaClientSDK {
    namespace acsdkAlerts {
        namespace renderer {
            using namespace std;
            using namespace chrono;
            using namespace acsdkAlerts;
            using namespace avsCommon;
            using namespace renderer;
            using namespace utils;
            class RendererInterface {
            public:
                virtual ~RendererInterface() = default;
                virtual void start(shared_ptr<RendererObserverInterface> observer, function<pair<unique_ptr<istream>, const MediaType>()> audioFactory,
                                   bool volumeRampEnabled, const vector<string>& urls = vector<string>(), int loopCount = 0,
                                   milliseconds loopPause = milliseconds{0}, bool startWithPause = false) = 0;
                virtual void stop() = 0;
            };
        }
    }
}
#endif