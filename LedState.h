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

#ifndef HEADER_LEDSTATE
#define HEADER_LEDSTATE

#include <cstdint>
#include <cstddef>

struct LedState {
    uint16_t hue;
    uint8_t saturation;
    uint8_t value;
    uint8_t mode;   // will be an enum later, 0 = static, 1 = animation, etc.

    bool operator==(const LedState& other) const;
    bool operator!=(const LedState& other) const;
    bool isValid() const;
    bool serializeHsv(char* buffer, const size_t size) const;
    bool serializeMode(char* buffer, const size_t size) const;
    static void setDefault(LedState& state);
};

#endif
