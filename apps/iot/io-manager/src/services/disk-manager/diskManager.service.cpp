#include "diskManager.service.h"
#include "Arduino.h"
#include <algorithm> // Add this line to include the <algorithm> header file

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
    String result = readString(keyAddress + MAX_KEY_LENGTH);
    return result;
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
            return i; // Return the address of the matching key
        }
    }
    return -1; // Key not found
}

int DiskManagerService::findEmptyAddress()
{
    for (int i = 0; i < EEPROM_SIZE; i += MAX_KEY_LENGTH + MAX_VALUE_LENGTH)
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
    if (address < 0 || address >= EEPROM_SIZE)
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
void DiskManagerService::purge()
{
    for (int i = 0; i < EEPROM_SIZE; i++)
    {
        EEPROM.write(i, 0);
    }
    EEPROM.end();
}