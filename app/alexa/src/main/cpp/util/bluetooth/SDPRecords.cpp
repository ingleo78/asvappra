#include <sdkinterfaces/Bluetooth/Services/A2DPSinkInterface.h>
#include <sdkinterfaces/Bluetooth/Services/A2DPSourceInterface.h>
#include <sdkinterfaces/Bluetooth/Services/AVRCPControllerInterface.h>
#include <sdkinterfaces/Bluetooth/Services/AVRCPTargetInterface.h>
#include <sdkinterfaces/Bluetooth/Services/HFPInterface.h>
#include <sdkinterfaces/Bluetooth/Services/HIDInterface.h>
#include <sdkinterfaces/Bluetooth/Services/SPPInterface.h>
#include "SDPRecords.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace bluetooth {
                using namespace avsCommon::sdkInterfaces::bluetooth::services;
                SDPRecord::SDPRecord(const std::string& name, const std::string& uuid, const std::string& version) :
                        m_name{name},
                        m_uuid{uuid},
                        m_version{version} {
                }
                std::string SDPRecord::getName() const {
                    return m_name.data();
                }
                std::string SDPRecord::getUuid() const {
                    return m_uuid.data();
                }
                std::string SDPRecord::getVersion() const {
                    return m_version.data();
                }
                A2DPSourceRecord::A2DPSourceRecord(const std::string& version) : SDPRecord{A2DPSourceInterface::NAME, A2DPSourceInterface::UUID, version} {}
                A2DPSinkRecord::A2DPSinkRecord(const std::string& version) : SDPRecord{A2DPSinkInterface::NAME, A2DPSinkInterface::UUID, version} {}
                AVRCPTargetRecord::AVRCPTargetRecord(const std::string& version) : SDPRecord{AVRCPTargetInterface::NAME, AVRCPTargetInterface::UUID, version} {}
                AVRCPControllerRecord::AVRCPControllerRecord(const std::string& version) :
                        SDPRecord{AVRCPControllerInterface::NAME, AVRCPControllerInterface::UUID, version} {}
                HFPRecord::HFPRecord(const std::string& version) : SDPRecord{HFPInterface::NAME, HFPInterface::UUID, version} {}
                HIDRecord::HIDRecord(const std::string& version) : SDPRecord(HIDInterface::NAME, HIDInterface::UUID, version) {}
                SPPRecord::SPPRecord(const std::string& version) : SDPRecord(SPPInterface::NAME, SPPInterface::UUID, version) {}
            }
        }
    }
}
