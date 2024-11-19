#ifndef DISKMANAGER_SERVICE_H
#define DISKMANAGER_SERVICE_H

#include <Arduino.h>
#include <EEPROM.h>
#include <map>
#include <string>

class DiskManagerService
{
public:
    DiskManagerService();
    ~DiskManagerService();

    void initialize();
    void save(const String &key, const String &value);
    String read(const String &key);
    void remove(const String &key);
    void purge();
private:
    int findKeyAddress(const String &key);
    int findEmptyAddress();
    void writeString(int address, const String &data);
    String readString(int address);
    
    const int EEPROM_SIZE = 4096; // Adjust based on your EEPROM size
    const int MAX_KEY_LENGTH = 32;
    const int MAX_VALUE_LENGTH = 64;
};

#endif // DISKMANAGER_SERVICE_H