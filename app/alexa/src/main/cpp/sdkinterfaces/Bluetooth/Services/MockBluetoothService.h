#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_BLUETOOTH_SERVICES_MOCKBLUETOOTHSERVICE_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_BLUETOOTH_SERVICES_MOCKBLUETOOTHSERVICE_H_

#include <gmock/gmock.h>
#include "BluetoothServiceInterface.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace sdkInterfaces {
            namespace bluetooth {
                namespace services {
                    namespace test {
                        class MockBluetoothService : public BluetoothServiceInterface {
                        public:
                            void setup() override;
                            void cleanup() override;
                            std::shared_ptr<SDPRecordInterface> getRecord() override;
                            MockBluetoothService(std::shared_ptr<SDPRecordInterface> record);
                        protected:
                            std::shared_ptr<SDPRecordInterface> m_record;
                        };
                        inline std::shared_ptr<SDPRecordInterface> MockBluetoothService::getRecord() {
                            return m_record;
                        }
                        MockBluetoothService::MockBluetoothService(std::shared_ptr<SDPRecordInterface> record) : m_record{record} {}
                        void MockBluetoothService::setup() {/*no-op*/}
                        void MockBluetoothService::cleanup() {/*no op*/}
                    }
                }
            }
        }
    }
}
#endif