#ifndef ALEXA_CLIENT_SDK_SPEECHENCODER_INCLUDE_SPEECHENCODER_ENCODERCONTEXT_H_
#define ALEXA_CLIENT_SDK_SPEECHENCODER_INCLUDE_SPEECHENCODER_ENCODERCONTEXT_H_

#include <memory>
#include <util/AudioFormat.h>

namespace alexaClientSDK {
    namespace speechencoder {
        class EncoderContext {
        public:
            virtual bool init(alexaClientSDK::avsCommon::utils::AudioFormat inputFormat) = 0;
            virtual size_t getInputFrameSize() = 0;
            virtual size_t getOutputFrameSize() = 0;
            virtual bool requiresFullyRead() = 0;
            virtual alexaClientSDK::avsCommon::utils::AudioFormat getAudioFormat() = 0;
            virtual std::string getAVSFormatName() = 0;
            virtual bool start() = 0;
            virtual ssize_t processSamples(void* samples, size_t numberOfWords, uint8_t* buffer) = 0;
            virtual void close() = 0;
            virtual ~EncoderContext() = default;
        };
    }
}

#endif  // ALEXA_CLIENT_SDK_SPEECHENCODER_INCLUDE_SPEECHENCODER_ENCODERCONTEXT_H_