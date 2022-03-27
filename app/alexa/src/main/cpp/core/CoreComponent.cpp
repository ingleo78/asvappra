#include <shared/ComponentAccumulator.h>
#include <shared/ConstructorAdapter.h>
#include <shared/SharedComponent.h>
#include <context_manager/ContextManager.h>
#include "CoreComponent.h"

namespace alexaClientSDK {
    namespace acsdkCore {
        using namespace contextManager;
        Component<Import<shared_ptr<MetricRecorderInterface>>, Import<shared_ptr<MultiTimer>>, shared_ptr<ContextManagerInterface>,
                  shared_ptr<DeviceInfo>, shared_ptr<CustomerDataManager>> getComponent() {
            return ComponentAccumulator<>() .addComponent(getComponent()).addRetainedFactory(ContextManager::createContextManagerInterface)
                   .addRetainedFactory(DeviceInfo::createFromConfiguration).addRetainedFactory(ConstructorAdapter<CustomerDataManager>::get());
        }
    }
}