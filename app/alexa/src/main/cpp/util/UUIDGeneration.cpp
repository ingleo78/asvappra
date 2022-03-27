#include <random>
#include <chrono>
#include <sstream>
#include <string>
#include <iomanip>
#include <cmath>
#include <mutex>
#include <climits>
#include <algorithm>
#include <functional>
#include <logger/Logger.h>
#include <uuid_generation/UUIDGeneration.h>

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace uuidGeneration {
                using namespace std;
                using namespace chrono;
                static const std::string TAG("UUIDGeneration");
                #define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)
                static const uint8_t UUID_VERSION_VALUE = 4 << 4;
                static const size_t UUID_VARIANT_VALUE = 2 << 6;
                static const string SEPARATOR("-");
                static const size_t MAX_NUM_REPLACEMENT_BITS = CHAR_BIT;
                static const size_t BITS_IN_HEX_DIGIT = 4;
                static bool g_seedNeeded = true;
                static mutex g_mutex;
                static string g_salt("default");
                static const double ENTROPY_THRESHOLD = 600;
                static const int ENTROPY_REPEAT_THRESHOLD = 16;
                void setSalt(const string& newSalt) {
                    unique_lock<std::mutex> lock(g_mutex);
                    g_salt = newSalt;
                    g_seedNeeded = true;
                }
                static const string generateHexWithReplacement(independent_bits_engine<default_random_engine, CHAR_BIT, uint16_t>& ibe,
                                                               unsigned int numDigits, uint8_t replacementBits, uint16_t numReplacementBits) {
                    if (numReplacementBits > MAX_NUM_REPLACEMENT_BITS) {
                        ACSDK_ERROR(LX("generateHexWithReplacementFailed").d("reason", "replacingMoreBitsThanProvided")
                                          .d("numReplacementBitsLimit", MAX_NUM_REPLACEMENT_BITS).d("numReplacementBits", numReplacementBits));
                        return "";
                    }
                    if (numReplacementBits > (numDigits * BITS_IN_HEX_DIGIT)) {
                        ACSDK_ERROR(LX("generateHexWithReplacementFailed").d("reason", "replacingMoreBitsThanGenerated")
                                          .d("numDigitsInBits", numDigits * BITS_IN_HEX_DIGIT).d("numReplacementBits", numReplacementBits));
                        return "";
                    }
                    const size_t arrayLen = (numDigits + 1) / 2;
                    vector<uint16_t> shorts(arrayLen);
                    generate(shorts.begin(), shorts.end(), ref(ibe));
                    vector<uint8_t> bytes(arrayLen);
                    for (size_t i = 0; i < arrayLen; i++) {
                        bytes[i] = static_cast<uint8_t>(shorts[i]);
                    }
                    bytes.at(0) &= (0xff >> numReplacementBits);
                    replacementBits &= (0xff << (MAX_NUM_REPLACEMENT_BITS - numReplacementBits));
                    bytes.at(0) |= replacementBits;
                    ostringstream oss;
                    for (const auto& byte : bytes) {
                        oss << hex << std::setfill('0') << setw(2) << static_cast<int>(byte);
                    }
                    string bytesText = oss.str();
                    bytesText.resize(numDigits);
                    return bytesText;
                }
                static const string generateHex(independent_bits_engine<default_random_engine, CHAR_BIT, uint16_t>& ibe, unsigned int numDigits) {
                    return generateHexWithReplacement(ibe, numDigits, 0, 0);
                }
                const string generateUUID() {
                    static independent_bits_engine<default_random_engine, CHAR_BIT, uint16_t> ibe;
                    unique_lock<mutex> lock(g_mutex);
                    static int consistentEntropyReports = 0;
                    static double priorEntropyResult = 0;
                    if (g_seedNeeded) {
                        random_device rd;
                        double currentEntropy = rd.entropy();
                        if (fabs(currentEntropy - priorEntropyResult) < numeric_limits<double>::epsilon()) ++consistentEntropyReports;
                        else consistentEntropyReports = 0;
                        priorEntropyResult = currentEntropy;
                        long randomTimeComponent = rd() + duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
                        string fullSeed = g_salt + to_string(randomTimeComponent);
                        seed_seq seed(fullSeed.begin(), fullSeed.end());
                        ibe.seed(seed);
                        if (currentEntropy > ENTROPY_THRESHOLD) g_seedNeeded = false;
                        else {
                            ACSDK_WARN(LX("low entropy on call to generate UUID").d("current entropy", currentEntropy));
                        }
                        if (consistentEntropyReports > ENTROPY_REPEAT_THRESHOLD) {
                            g_seedNeeded = false;
                            ACSDK_WARN(LX("multiple repeat values for entropy")
                                           .d("current entropy", currentEntropy)
                                           .d("consistent entropy reports", consistentEntropyReports));
                        }
                    }
                    ostringstream uuidText;
                    uuidText << generateHex(ibe, 8) << SEPARATOR << generateHex(ibe, 4) << SEPARATOR
                             << generateHexWithReplacement(ibe, 4, UUID_VERSION_VALUE, 4) << SEPARATOR
                             << generateHexWithReplacement(ibe, 4, UUID_VARIANT_VALUE, 2) << SEPARATOR
                             << generateHex(ibe, 12);

                    lock.unlock();
                    return uuidText.str();
                }
            }
        }
    }
}
