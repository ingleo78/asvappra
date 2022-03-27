#include <fstream>
#include <logger/Logger.h>
#include "DiagnosticsUtils.h"

namespace alexaClientSDK {
    namespace diagnostics {
        namespace utils {
            static const string TAG("DiagnosticsUtils");
            #define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)
            static constexpr unsigned int NUM_BITS_PER_SAMPLE = 16;
            static constexpr unsigned int NUM_OF_AUDIO_CHANNELS = 1;
            static constexpr unsigned int SAMPLES_PER_SECOND = 16000;
            static constexpr unsigned int LPCM_AUDIO_FORMAT = 1;
            bool validateAudioFormat(const WavFileHeader& wavFileHeader) {
                if (wavFileHeader.bitsPerSample != NUM_BITS_PER_SAMPLE) {
                    ACSDK_ERROR(LX("validateAudioFormatFailed").d("reason", "only 16 bits per sample supported"));
                    return false;
                }
                if (wavFileHeader.numberOfChannels != NUM_OF_AUDIO_CHANNELS) {
                    ACSDK_ERROR(LX("validateAudioFormatFailed").d("reason", "only 1 audio channel supported"));
                    return false;
                }
                if (wavFileHeader.samplesPerSec != SAMPLES_PER_SECOND) {
                    ACSDK_ERROR(LX("validateAudioFormatFailed").d("reason", "only 16000 samples per second supported"));
                    return false;
                }
                if (wavFileHeader.audioFormat != LPCM_AUDIO_FORMAT) {
                    ACSDK_ERROR(LX("validateAudioFormatFailed").d("reason", "only LPCM supported"));
                    return false;
                }
                return true;
            }
            bool readWavFileToBuffer(const string& absoluteFilePath, vector<uint16_t>* audioBuffer) {
                if (!audioBuffer) {
                    ACSDK_ERROR(LX("readAudioFileFailed").d("reason", "nullptrAudioBuffer"));
                    return false;
                }
                ifstream inputFile(absoluteFilePath, ios::binary);
                if (!inputFile.good()) {
                    ACSDK_ERROR(LX("readAudioFileFailed").d("reason", "unable to open file"));
                    return false;
                }
                inputFile.seekg(0, std::ios::end);
                int fileLengthInBytes = inputFile.tellg();
                const int wavFileHeaderSize = sizeof(WavFileHeader);
                if (fileLengthInBytes <= wavFileHeaderSize) {
                    ACSDK_ERROR(LX("readAudioFileFailed").d("reason", "file size less than RIFF header"));
                    return false;
                }
                inputFile.seekg(0, std::ios::beg);
                WavFileHeader wavFileHeader;
                inputFile.read(reinterpret_cast<char*>(&wavFileHeader), wavFileHeaderSize);
                if (static_cast<size_t>(inputFile.gcount()) != wavFileHeaderSize) {
                    ACSDK_ERROR(LX("readAudioFileFailed").d("reason", "failed reading header"));
                    return false;
                }
                int numSamples = (fileLengthInBytes - wavFileHeaderSize) / sizeof(uint16_t);
                audioBuffer->resize(numSamples, 0);
                inputFile.read(reinterpret_cast<char*>(&(audioBuffer->at(0))), numSamples * sizeof(uint16_t));
                if (static_cast<size_t>(inputFile.gcount()) != (numSamples * sizeof(uint16_t))) {
                    ACSDK_ERROR(LX("readAudioFileFailed").d("reason", "failed reading audio data"));
                    return false;
                }
                return true;
            }
        }
    }
}