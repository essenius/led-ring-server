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


#include "Utilities.h"

namespace utilities {

    log_callback logger = nullptr;

    void build_topic(char* buffer, size_t size, const char* base, const char* sub1, const char* sub2) {
        strlcpy(buffer, base, size);
        strlcat(buffer, "/", size);
        strlcat(buffer, sub1, size);
        if (sub2) {
            strlcat(buffer, "/", size);
            strlcat(buffer, sub2, size);
        }
    }

    int clamp(int value, const int min, const int max) {
        if (min == max) return min;

        value = std::max(value, min);
        value = std::min(value, max);

        return value;
    }

    int scale_clamped(int value, const int inMin, const int inMax, const int outMin, const int outMax) {

        value = clamp(value, inMin, inMax);
        if (inMin == outMin && inMax == outMax) return value;

        // casting to long to prevent overflow during multiplication
        return static_cast<int>(outMin + static_cast<long>(outMax - outMin) * (value - inMin) / (inMax - inMin));
    }

    float scale_clamped(const int value, const int inMin, const int inMax, const float outMin, const float outMax) {

    const auto inValue = clamp(value, inMin, inMax);
    const auto inRange = static_cast<float>(inMax - inMin);
    const auto valueRange = static_cast<float>(inValue - inMin);
        return outMin + (outMax - outMin) * valueRange / inRange;
    }

    char* next_token(char** start, const int delimiter) {
        char* token = *start;
        char* nextDelimiter = token != nullptr ? strchr(token, delimiter) : nullptr;
        if (nextDelimiter == nullptr) {
            *start = nullptr;
        }
        else {
            *nextDelimiter = '\0';
            *start = nextDelimiter + 1;
        }
        return token;
    }

    void build_url(char* buffer, size_t size, const char* base, const char* child) {
        strlcpy(buffer, base, size);
        strlcat(buffer, child, size);
    }
}
