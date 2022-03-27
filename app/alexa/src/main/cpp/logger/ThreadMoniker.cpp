#include <atomic>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <thread>
#include <unordered_map>
#include "ThreadMoniker.h"

namespace alexaClientSDK {
    namespace avsCommon {
        namespace utils {
            namespace logger {
                static std::atomic<int> g_nextThreadMoniker(1);
                ThreadMoniker::ThreadMoniker(const std::string& moniker) : m_moniker{moniker.empty() ? generateMoniker() : moniker} { }
                std::string ThreadMoniker::generateMoniker() {
                    std::ostringstream stream;
                    stream << std::setw(3) << std::hex << std::right << g_nextThreadMoniker++;
                    return stream.str();
                }
                const ThreadMoniker& ThreadMoniker::getMonikerObjectFromMap(const std::string& moniker) {
                    static std::unordered_map<std::thread::id, ThreadMoniker> threadMonikers;
                    static std::unordered_map<std::string, std::thread::id> monikerThreads;
                    static std::mutex monikerMutex;
                    std::lock_guard<std::mutex> lock{monikerMutex};
                    auto id = std::this_thread::get_id();
                    auto entry = threadMonikers.find(id);
                    if (entry == threadMonikers.end()) {
                        auto oldEntry = monikerThreads.find(moniker);
                        if (oldEntry != monikerThreads.end()) threadMonikers.erase(oldEntry->second);
                        auto& object = threadMonikers.emplace(std::make_pair(id, ThreadMoniker(moniker))).first->second;
                        monikerThreads[object.m_moniker] = id;
                        return object;
                    }
                    return entry->second;
                }
                std::string ThreadMoniker::getThisThreadMoniker() {
                    return getMonikerObject().m_moniker;
                }
                void ThreadMoniker::setThisThreadMoniker(const std::string& moniker) {
                    getMonikerObject(moniker);
                }
                const ThreadMoniker& ThreadMoniker::getMonikerObject(const std::string& moniker) {
                    #ifdef _WIN32
                            return getMonikerObjectFromMap(moniker);
                    #else
                            static thread_local ThreadMoniker m_threadMoniker{moniker};
                            return m_threadMoniker;
                    #endif
                }
            }
        }
    }
}
