#include "diskManager.service.h"
#include <algorithm>

DiskManagerService *DiskManagerService::instance = nullptr;

DiskManagerService::DiskManagerService()
{
    instance = this;
}

DiskManagerService::~DiskManagerService()
{
    instance = nullptr;
}

DiskManagerService *DiskManagerService::getInstance()
{
    return instance;
}

void DiskManagerService::initialize()
{
    EEPROM.begin();
    reserveBlock(0x00, 128);
}

void DiskManagerService::save(const String &key, const String &value)
{
    int keyAddress = findKeyAddress(key);

    if (keyAddress == -1)
    { // Key not found
        keyAddress = findEmptyAddress();
        if (keyAddress == -1)
        { // No empty space
            Serial.println("Error: No space left in EEPROM.");
            return;
        }

        writeString(keyAddress, key); // Save the key
    }

    writeString(keyAddress + MAX_KEY_LENGTH, value); // Save the value
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

void DiskManagerService::purge()
{
    for (int i = RESERVED_END + 1; i < EEPROM_SIZE; i++) // Skip reserved area
    {
        EEPROM.write(i, 0);
    }
    EEPROM.end();
}

void DiskManagerService::reserveBlock(int startAddress, int size)
{
    // Mark addresses as reserved by writing a placeholder value (e.g., 0xFF)
    for (int i = startAddress; i < startAddress + size; i++)
    {
        if (i < EEPROM_SIZE)
        {
            EEPROM.write(i, 0xFF);
        }
    }
    EEPROM.end();
}

int DiskManagerService::findKeyAddress(const String &key)
{
    for (int i = RESERVED_END + 1; i < EEPROM_SIZE; i += MAX_KEY_LENGTH + MAX_VALUE_LENGTH)
    {
        String storedKey = readString(i);

        if (storedKey == key)
        {
            return i; // Return the address of the matching key
        }
    }
    return -1; // Key not found
}

int DiskManagerService::findEmptyAddress()
{
    for (int i = RESERVED_END + 1; i < EEPROM_SIZE; i += MAX_KEY_LENGTH + MAX_VALUE_LENGTH)
    {
        bool isEmpty = true;

        // Check if all bytes in this block are 0
        for (int j = 0; j < MAX_KEY_LENGTH; j++)
        {
            if (EEPROM.read(i + j) != 0)
            {
                isEmpty = false;
                break;
            }
        }

        if (isEmpty)
        {
            return i; // Return the starting address of the empty block
        }
    }
    return -1; // No empty address found
}

void DiskManagerService::writeString(int address, const String &data)
{
    if (address < RESERVED_END + 1 || address >= EEPROM_SIZE)
    {
        Serial.println("Error: Invalid EEPROM address: " + String(address));
        return;
    }

    unsigned int maxLength = MAX_KEY_LENGTH; // Adjust for value if needed
    int length = std::min(data.length(), maxLength);

    // Write each character
    for (int i = 0; i < length; i++)
    {
        EEPROM.write(address + i, data[i]);
    }

    // Null-terminate the string
    EEPROM.write(address + length, 0);
}

String DiskManagerService::readString(int address)
{
    String result = "";
    int maxLength = MAX_KEY_LENGTH + MAX_VALUE_LENGTH;

    // Read characters until null terminator or max length
    for (int i = 0; i < maxLength; i++)
    {
        char ch = EEPROM.read(address + i);
        if (ch == 0)
        { // Null-terminator found
            break;
        }
        result += ch;
    }

    return result;
}
