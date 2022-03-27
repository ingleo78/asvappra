#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_ENDPOINTS_ENDPOINTBUILDERINTERFACE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_INCLUDE_AVSCOMMON_SDKINTERFACES_ENDPOINTS_ENDPOINTBUILDERINTERFACE_H_

#include <memory>
#include <string>
#include <map>
#include <sdkinterfaces/Endpoints/EndpointInterface.h>
#include <sdkinterfaces/ModeController/ModeControllerAttributes.h>
#include <sdkinterfaces/ModeController/ModeControllerInterface.h>
#include <sdkinterfaces/PowerController/PowerControllerInterface.h>
#include <sdkinterfaces/RangeController/RangeControllerAttributes.h>
#include <sdkinterfaces/RangeController/RangeControllerInterface.h>
#include <sdkinterfaces/ToggleController/ToggleControllerAttributes.h>
#include <sdkinterfaces/ToggleController/ToggleControllerInterface.h>
#include <util/Optional.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace endpoints {
                using namespace std;
                using namespace avs;
                using namespace sdkInterfaces;
                using namespace utils;
                using namespace powerController;
                using namespace modeController;
                using namespace toggleController;
                using namespace rangeController;
                class EndpointBuilderInterface {
                public:
                    virtual ~EndpointBuilderInterface() = default;
                    virtual EndpointBuilderInterface& withDerivedEndpointId(const string& suffix);
                    virtual EndpointBuilderInterface& withEndpointId(const EndpointIdentifier& endpointId);
                    virtual EndpointBuilderInterface& withFriendlyName(const string& friendlyName);
                    virtual EndpointBuilderInterface& withDescription(const string& description);
                    virtual EndpointBuilderInterface& withManufacturerName(const string& manufacturerName);
                    virtual EndpointBuilderInterface& withDisplayCategory(const vector<std::string>& displayCategories);
                    virtual EndpointBuilderInterface& withAdditionalAttributes(const string& manufacturer, const string& model, const string& serialNumber,
                                                                               const string& firmwareVersion, const string& softwareVersion,
                                                                               const string& customIdentifier);
                    virtual EndpointBuilderInterface& withConnections(const vector<map<string, string>>& connections);
                    virtual EndpointBuilderInterface& withCookies(const map<string, string>& cookies);
                    virtual EndpointBuilderInterface& withPowerController(shared_ptr<PowerControllerInterface> powerController,
                                                                          bool isProactivelyReported, bool isRetrievable);
                    virtual EndpointBuilderInterface& withToggleController(shared_ptr<ToggleControllerInterface> toggleController, const string& instance,
                                                                           const ToggleControllerAttributes& toggleControllerAttributes, bool isProactivelyReported,
                                                                           bool isRetrievable, bool isNonControllable = false);
                    virtual EndpointBuilderInterface& withModeController(std::shared_ptr<ModeControllerInterface> modeController, const string& instance,
                                                                         const ModeControllerAttributes& modeControllerAttributes, bool isProactivelyReported,
                                                                         bool isRetrievable, bool isNonControllable = false);
                    virtual EndpointBuilderInterface& withRangeController(shared_ptr<RangeControllerInterface> rangeController, const string& instance,
                                                                          const RangeControllerAttributes& rangeControllerAttributes, bool isProactivelyReported,
                                                                          bool isRetrievable, bool isNonControllable = false) ;
                    virtual unique_ptr<EndpointInterface> build();
                };
            }
        }
    }
}
#endif