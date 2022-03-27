#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_ENDPOINTS_MOCKENDPOINTBUILDER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_ENDPOINTS_MOCKENDPOINTBUILDER_H_

#include <gmock/gmock.h>
#include "EndpointBuilderInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace endpoints {
                namespace test {
                    using namespace std;
                    class MockEndpointBuilder : public EndpointBuilderInterface {
                    public:
                        MOCK_METHOD1(withDerivedEndpointId, EndpointBuilderInterface&(const string& suffix));
                        MOCK_METHOD1(withEndpointId, EndpointBuilderInterface&(const EndpointIdentifier& endpointId));
                        MOCK_METHOD1(withFriendlyName, EndpointBuilderInterface&(const string& friendlyName));
                        MOCK_METHOD1(withDescription, EndpointBuilderInterface&(const string& description));
                        MOCK_METHOD1(withManufacturerName, EndpointBuilderInterface&(const string& manufacturerName));
                        MOCK_METHOD1(withDisplayCategory, EndpointBuilderInterface&(const vector<string>& displayCategories));
                        MOCK_METHOD6(withAdditionalAttributes, EndpointBuilderInterface&(const string& manufacturer, const string& model,
                                     const string& serialNumber, const string& firmwareVersion, const string& softwareVersion, const string& customIdentifier));
                        MOCK_METHOD1(withConnections, EndpointBuilderInterface&(const vector<map<string, string>>& connections));
                        MOCK_METHOD1(withCookies, EndpointBuilderInterface&(const map<string, string>& cookies));
                        MOCK_METHOD0(build, unique_ptr<EndpointInterface>());
                    };
                }
            }
        }
    }
}

#endif  // ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_ENDPOINTS_MOCKENDPOINTBUILDER_H_
