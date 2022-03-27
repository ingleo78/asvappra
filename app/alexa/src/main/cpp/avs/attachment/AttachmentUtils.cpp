#include <cstdlib>
#include <util/sds/InProcessSDS.h>
#include <util/Writer.h>
#include <logger/Logger.h>
#include "InProcessAttachmentReader.h"
#include "AttachmentUtils.h"

using namespace alexaClientSDK::avsCommon::utils::sds;

namespace alexaClientSDK {
    namespace avsCommon {
        namespace avs {
            namespace attachment {
                static const std::string TAG("AttachmentUtils");
                static const std::size_t MAX_READER_SIZE = 4 * 1024;
                #define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)
                std::unique_ptr<AttachmentReader> AttachmentUtils::createAttachmentReader(const std::vector<char>& srcBuffer) {
                    auto bufferSize = InProcessSDS::calculateBufferSize(srcBuffer.size());
                    auto buffer = std::make_shared<SharedDataStream<InProcessSDSTraits>>(bufferSize);
                    auto stream = InProcessSDS::create(make_shared<InProcessSDS::Buffer>(bufferSize));
                    if (!stream) {
                        ACSDK_ERROR(LX("createAttachmentReaderFailed").d("reason", "Failed to create stream"));
                        return nullptr;
                    }
                    auto writer = stream->createWriter(SharedDataStream<InProcessSDS>::Writer::Policy::BLOCKING);
                    if (!writer) {
                        ACSDK_ERROR(LX("createAttachmentReaderFailed").d("reason", "Failed to create writer"));
                        return nullptr;
                    }
                    if (srcBuffer.size() > MAX_READER_SIZE) {
                        ACSDK_WARN(LX("createAttachmentReaderExceptionSize").d("size", srcBuffer.size()));
                    }
                    auto numWritten = writer->write(srcBuffer.data(), srcBuffer.size());
                    if (numWritten < 0 || static_cast<size_t>(numWritten) != srcBuffer.size()) {
                        ACSDK_ERROR(LX("createAttachmentReaderFailed").d("reason", "writing failed").d("buffer size", srcBuffer.size())
                                    .d("numWritten", numWritten));
                        return nullptr;
                    }
                    auto attachmentReader = InProcessAttachmentReader::create(InProcessSDS::Reader::Policy::BLOCKING, std::move(stream));
                    return std::move(attachmentReader);
                }
            }
        }
    }
}