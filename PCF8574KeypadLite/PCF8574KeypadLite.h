#ifndef PCF8574_KEYPAD_LITE_H
#define PCF8574_KEYPAD_LITE_H

#include <Arduino.h>
#include <Wire.h>

class PCF8574KeypadLite {
  public:
    PCF8574KeypadLite(uint8_t address);

    bool begin();
    char getKey();

    void setKeyMap(const char newKeyMap[4][4]);
    void setPins(const uint8_t rowPins[4], const uint8_t colPins[4]);
    void setDebounceTime(uint16_t debounceMs);

  private:
    uint8_t _address;

    uint8_t _rows[4] = {0, 1, 2, 3};
    uint8_t _cols[4] = {4, 5, 6, 7};

    char _keyMap[4][4] = {
      {'1', '2', '3', 'A'},
      {'4', '5', '6', 'B'},
      {'7', '8', '9', 'C'},
      {'*', '0', '#', 'D'}
    };

    uint16_t _debounceMs = 50;

    char _lastReadKey = 0;
    char _stableKey = 0;
    unsigned long _lastChangeTime = 0;

    bool writePCF(uint8_t data);
    uint8_t readPCF();
    char scanKey();
};

#endif