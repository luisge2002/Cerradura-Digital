#include "PCF8574KeypadLite.h"

PCF8574KeypadLite::PCF8574KeypadLite(uint8_t address) {
  _address = address;
}

bool PCF8574KeypadLite::begin() {
  Wire.beginTransmission(_address);
  if (Wire.endTransmission() != 0) {
    return false;
  }

  writePCF(0xFF);
  return true;
}

void PCF8574KeypadLite::setKeyMap(const char newKeyMap[4][4]) {
  for (uint8_t r = 0; r < 4; r++) {
    for (uint8_t c = 0; c < 4; c++) {
      _keyMap[r][c] = newKeyMap[r][c];
    }
  }
}

void PCF8574KeypadLite::setPins(const uint8_t rowPins[4], const uint8_t colPins[4]) {
  for (uint8_t i = 0; i < 4; i++) {
    _rows[i] = rowPins[i];
    _cols[i] = colPins[i];
  }
}

void PCF8574KeypadLite::setDebounceTime(uint16_t debounceMs) {
  _debounceMs = debounceMs;
}

bool PCF8574KeypadLite::writePCF(uint8_t data) {
  Wire.beginTransmission(_address);
  Wire.write(data);
  return Wire.endTransmission() == 0;
}

uint8_t PCF8574KeypadLite::readPCF() {
  Wire.requestFrom(_address, (uint8_t)1);

  if (Wire.available()) {
    return Wire.read();
  }

  return 0xFF;
}

char PCF8574KeypadLite::scanKey() {
  for (uint8_t r = 0; r < 4; r++) {
    uint8_t outputData = 0xFF;

    bitClear(outputData, _rows[r]);

    writePCF(outputData);
    delayMicroseconds(300);

    uint8_t inputData = readPCF();

    for (uint8_t c = 0; c < 4; c++) {
      if (bitRead(inputData, _cols[c]) == 0) {
        writePCF(0xFF);
        return _keyMap[r][c];
      }
    }
  }

  writePCF(0xFF);
  return 0;
}

char PCF8574KeypadLite::getKey() {
  char currentKey = scanKey();

  if (currentKey != _lastReadKey) {
    _lastChangeTime = millis();
    _lastReadKey = currentKey;
  }

  if ((millis() - _lastChangeTime) > _debounceMs) {
    if (currentKey != _stableKey) {
      _stableKey = currentKey;

      if (_stableKey != 0) {
        return _stableKey;
      }
    }
  }

  return 0;
}