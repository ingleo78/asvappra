#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_DIAGNOSTICS_DIAGNOSTICSINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_DIAGNOSTICS_DIAGNOSTICSINTERFACE_H_

#include "../../../../../../../../AndroidSDK/ndk/21.3.6528147/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/c++/v1/memory"
#include "../../avs/attachment/AttachmentManagerInterface.h"
#include "../../sdkinterfaces/Diagnostics/AudioInjectorInterface.h"
#include "../../sdkinterfaces/Diagnostics/DevicePropertyAggregatorInterface.h"
#include "../../sdkinterfaces/Diagnostics/ProtocolTracerInterface.h"
#include "../../sdkinterfaces/DirectiveSequencerInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace diagnostics {
                using namespace std;
                using namespace avs::attachment;
                class DiagnosticsInterface {
                public:
                    virtual ~DiagnosticsInterface() = default;
                    virtual shared_ptr<DevicePropertyAggregatorInterface> getDevicePropertyAggregator();
                    virtual shared_ptr<ProtocolTracerInterface> getProtocolTracer() = 0;
                    virtual void setDiagnosticDependencies(shared_ptr<DirectiveSequencerInterface> sequencer,
                                                           shared_ptr<AttachmentManagerInterface> attachmentManager) = 0;
                    virtual shared_ptr<AudioInjectorInterface> getAudioInjector() = 0;
                };
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_DIAGNOSTICS_DIAGNOSTICSINTERFACE_H_
