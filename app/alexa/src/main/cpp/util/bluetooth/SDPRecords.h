#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_BLUETOOTH_SDPRECORDS_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_BLUETOOTH_SDPRECORDS_H_

#include <string>
#include <sdkinterfaces/Bluetooth/Services/SDPRecordInterface.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace bluetooth {
                class SDPRecord : public sdkInterfaces::bluetooth::services::SDPRecordInterface {
                public:
                    SDPRecord(const std::string &name, const std::string &uuid, const std::string &version);
                    std::string getName() const override;
                    std::string getUuid() const override;
                    std::string getVersion() const override;
                protected:
                    const std::string_view m_name;
                    const std::string_view m_uuid;
                    const std::string_view m_version;
                };
                class A2DPSourceRecord : public SDPRecord {
                public:
                    A2DPSourceRecord(const std::string &version);
                };
                class A2DPSinkRecord : public SDPRecord {
                public:
                    A2DPSinkRecord(const std::string &version);
                };
                class AVRCPTargetRecord : public SDPRecord {
                public:
                    AVRCPTargetRecord(const std::string &version);
                };
                class AVRCPControllerRecord : public SDPRecord {
                public:
                    AVRCPControllerRecord(const std::string &version);
                };
                class HFPRecord : public SDPRecord {
                public:
                    HFPRecord(const std::string &version);
                };
                class HIDRecord : public SDPRecord {
                public:
                    HIDRecord(const std::string &version);
                };
                class SPPRecord : public SDPRecord {
                public:
                    SPPRecord(const std::string &version);
                };
            }
        }
    }
}
#endif