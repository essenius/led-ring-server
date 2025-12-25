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

#include "LedState.h"
#include "Utilities.h"

using utilities::snprintf_t;

bool LedState::operator==(const LedState& other) const {
    return hue == other.hue && saturation == other.saturation &&
        value == other.value && mode == other.mode;
}

bool LedState::operator!=(const LedState& other) const {
    return !(*this == other);
}

bool LedState::isValid() const {
    return hue <= 360;
    // saturation and value are uint8_t so all values are valid
}

bool LedState::serializeHsv(char* buffer, const size_t size) const {
    return snprintf_t(buffer, size, "%u,%u,%u", hue, saturation, value);
}

bool LedState::serializeMode(char* buffer, const size_t size) const {
    return snprintf_t(buffer, size, "%d", mode);
}

void LedState::setDefault(LedState& state) {
    state.hue = 41;
    state.saturation = 73;
    state.value = 100;
    state.mode = 0;
}
