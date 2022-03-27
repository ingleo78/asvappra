#ifndef ACSDKBLUETOOTH_BLUETOOTHSTORAGEINTERFACE_H_
#define ACSDKBLUETOOTH_BLUETOOTHSTORAGEINTERFACE_H_

#include <unordered_map>
#include <list>
#include <storage/SQLiteDatabase.h>
#include <storage/SQLiteStatement.h>
#include <storage/SQLiteUtils.h>

namespace alexaClientSDK {
    namespace acsdkBluetooth {
        using namespace std;
        class BluetoothStorageInterface {
        public:
            virtual ~BluetoothStorageInterface() = default;
            virtual bool createDatabase() = 0;
            virtual bool open() = 0;
            virtual void close() = 0;
            virtual bool clear() = 0;
            virtual bool getMac(const string& uuid, string* mac) = 0;
            virtual bool getUuid(const string& mac, string* uuid) = 0;
            virtual bool getCategory(const string& uuid, string* category) = 0;
            virtual bool getMacToUuid(unordered_map<string, string>* macToUuid) = 0;
            virtual bool getMacToCategory(unordered_map<string, string>* macToCategory) = 0;
            virtual bool getUuidToMac(unordered_map<string, string>* uuidToMac) = 0;
            virtual bool getUuidToCategory(unordered_map<string, string>* uuidToCategory) = 0;
            virtual bool getOrderedMac(bool ascending, std::list<std::string>* macs) = 0;
            virtual bool insertByMac(const string& mac, const string& uuid, bool overwrite) = 0;
            virtual bool updateByCategory(const string& uuid, const string& category) = 0;
            virtual bool remove(const string& mac) = 0;
        };
    }
}
#endif