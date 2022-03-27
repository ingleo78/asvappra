#ifndef ACSDKCORE_CORECOMPONENT_H_
#define ACSDKCORE_CORECOMPONENT_H_

#include <memory>
#include <avs/attachment/AttachmentManagerInterface.h>
#include <util/DeviceInfo.h>
#include <metrics/MetricRecorderInterface.h>
#include <timing/MultiTimer.h>
#include <sdkinterfaces/ContextManagerInterface.h>
#include <registration_manager/CustomerDataManager.h>
#include <shared/Component.h>

namespace alexaClientSDK {
    namespace acsdkCore {
        using namespace std;
        using namespace acsdkManufactory;
        using namespace acsdkShared;
        using namespace avsCommon;
        using namespace sdkInterfaces;
        using namespace utils;
        using namespace metrics;
        using namespace timing;
        using namespace registrationManager;
        Component<Import<shared_ptr<MetricRecorderInterface>>, Import<shared_ptr<MultiTimer>>, shared_ptr<ContextManagerInterface>,
                  shared_ptr<DeviceInfo>, shared_ptr<CustomerDataManager>> getComponent();
    }
}
#endif