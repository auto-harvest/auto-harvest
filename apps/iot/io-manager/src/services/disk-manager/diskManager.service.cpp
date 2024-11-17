#include "diskManager.service.h"

DiskManagerService::DiskManagerService()
{
}

DiskManagerService::~DiskManagerService()
{
}

void DiskManagerService::initialize()
{
    EEPROM.begin();
}

void DiskManagerService::save(const String &key, const String &value)
{
    int keyAddress = findKeyAddress(key);
    if (keyAddress == -1)
    {
        keyAddress = findEmptyAddress();
        writeString(keyAddress, key);
    }
    writeString(keyAddress + MAX_KEY_LENGTH, value);
    EEPROM.end();
}

String DiskManagerService::read(const String &key)
{
    int keyAddress = findKeyAddress(key);
    if (keyAddress == -1)
    {
        return "";
    }
    return readString(keyAddress + MAX_KEY_LENGTH);
}

void DiskManagerService::remove(const String &key)
{
    int keyAddress = findKeyAddress(key);
    if (keyAddress != -1)
    {
        for (int i = 0; i < MAX_KEY_LENGTH + MAX_VALUE_LENGTH; i++)
        {
            EEPROM.write(keyAddress + i, 0);
        }
        EEPROM.end();
    }
}

int DiskManagerService::findKeyAddress(const String &key)
{
    for (int i = 0; i < EEPROM_SIZE; i += MAX_KEY_LENGTH + MAX_VALUE_LENGTH)
    {
        String storedKey = readString(i);
        if (storedKey == key)
        {
            return i;
        }
    }
    return -1;
}

int DiskManagerService::findEmptyAddress()
{
    for (int i = 0; i < EEPROM_SIZE; i += MAX_KEY_LENGTH + MAX_VALUE_LENGTH)
    {
        if (EEPROM.read(i) == 0)
        {
            return i;
        }
    }
    return -1;
}

void DiskManagerService::writeString(int address, const String &data)
{
    for (int i = 0; i < data.length(); i++)
    {
        EEPROM.write(address + i, data[i]);
    }
    EEPROM.write(address + data.length(), 0); // Null-terminate the string
}

String DiskManagerService::readString(int address)
{
    String result = "";
    char ch = EEPROM.read(address);
    while (ch != 0 && result.length() < MAX_KEY_LENGTH + MAX_VALUE_LENGTH)
    {
        result += ch;
        address++;
        ch = EEPROM.read(address);
    }
    return result;
}