// Copyright 2025 Rik Essenius
// 
//   Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
//   except in compliance with the License. You may obtain a copy of the License at
// 
//       http://www.apache.org/licenses/LICENSE-2.0
// 
//    Unless required by applicable law or agreed to in writing, software distributed under the License
//    is distributed on an "AS IS" BASIS WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and limitations under the License.

#include "Persistence.h"
#include <EEPROM.h>
#include <ESP.h>

void Persistence::begin() {
    // set last save time to before the save interval so a new put saves immediately
    _lastSaveTime = millis() - kMinSaveInterval - 1;
    EEPROM.begin(kSaveSize);

    EEPROM.get(0, _state);
    Serial.printf("Read from to EEPROM: %04x h=%d\n", _state.magicNumber, _state.ledState.hue);

    if (_state.magicNumber != kMagicNumber || !_state.ledState.isValid()) {
        LedState::setDefault(_state.ledState); 
        put(&_state.ledState);
    }
}

const LedState* Persistence::get() {
  // if begin wasn't called yet, do it first 
    if (_state.magicNumber != kMagicNumber) begin();
    return &_state.ledState;
}

bool Persistence::put(const LedState* state) {
    if (_state.ledState == *state) return true;
    _pendingState = *state;
    _putPending = true;
    return update();
}

bool Persistence::update() {
    if (!_putPending) return true;
    const unsigned long now = millis();

    if (now - _lastSaveTime >= kMinSaveInterval) {
        _state.magicNumber = kMagicNumber;
        _state.ledState = _pendingState;
        Serial.printf("Writing to EEPROM: %04x h=%d\n", _state.magicNumber, _state.ledState.hue);
        EEPROM.put(0, _state);
        EEPROM.commit();
        _lastSaveTime = now;
        _putPending = false;
        return true;
    }
    return false;
}
