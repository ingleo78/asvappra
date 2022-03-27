#include <acl/Transport/MockMimeResponseSink.h>

namespace alexaClientSDK {
    namespace acl {
        namespace test {

            MockMimeResponseSink::MockMimeResponseSink(){};
            bool MockMimeResponseSink::onReceiveResponseCode(long responseCode) {
                return true;
            }
            bool MockMimeResponseSink::onReceiveHeaderLine(const std::string& line) {
                return true;
            }
            bool MockMimeResponseSink::onBeginMimePart(const std::multimap<std::string, std::string>& headers) {
                m_mimeCurrentContent.clear();
                return true;
            }
            avsCommon::utils::http2::HTTP2ReceiveDataStatus MockMimeResponseSink::onReceiveMimeData(
                const char* bytes,
                size_t size) {
                for (unsigned i = 0; i < size; i++) {
                    m_mimeCurrentContent.push_back(bytes[i]);
                }
                return avsCommon::utils::http2::HTTP2ReceiveDataStatus::SUCCESS;
            }
            bool MockMimeResponseSink::onEndMimePart() {
                m_mimeContents.push_back(m_mimeCurrentContent);
                return true;
            }
            avsCommon::utils::http2::HTTP2ReceiveDataStatus MockMimeResponseSink::onReceiveNonMimeData(
                const char* bytes,
                size_t size) {
                return avsCommon::utils::http2::HTTP2ReceiveDataStatus::SUCCESS;
            }
            void MockMimeResponseSink::onResponseFinished(avsCommon::utils::http2::HTTP2ResponseFinishedStatus status) {
            }
            std::vector<char> MockMimeResponseSink::getMimePart(unsigned part) {
                return m_mimeContents[part];
            }
            unsigned MockMimeResponseSink::getCountOfMimeParts() {
                return m_mimeContents.size();
            }
        }
    }
}