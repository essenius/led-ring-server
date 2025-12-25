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

#ifndef HEADER_UTILITIES
#define HEADER_UTILITIES

#include <cstdio>
#include <cstring>
#include <algorithm>

namespace utilities {

    using log_callback = void(*)(const char*);

    extern log_callback logger;

    inline void set_logger(const log_callback log) { logger = log; }

    template <typename... Arguments>
    bool snprintf_t(char* buffer, const size_t size, const char* const format, Arguments ... arguments) {
        const int usedSize = snprintf(buffer, size, format, arguments...);
        if (usedSize >= 0 && static_cast<size_t>(usedSize) < size) return true;
        if (logger) {
            char message[64];
            if (usedSize < 0) {
                snprintf(message, sizeof(message), "snprintf error (returned %d)", usedSize);
            }
            else {
                snprintf(message, sizeof(message), "snprintf truncated (wanted %d, max %zu)", usedSize, size);
            }
            logger(message);
        }
        if (size > 0) {
            buffer[size - 1] = '\0';
        }
        return false;
    }

    template <size_t BufferSize, typename... Arguments>
    bool snprintf_t(char(&buffer)[BufferSize], const char* const format, Arguments ... arguments) {
        return snprintf_t(buffer, BufferSize, format, arguments...);
    }

    int clamp(int value, int min, int max);
    void build_url(char* buffer, size_t size, const char* base, const char* child);
    void build_topic(char* buffer, size_t size, const char* base, const char* sub1, const char* sub2 = nullptr);

    // thread safe alternative for strtok
    char* next_token(char** start, int delimiter);

    int scale_clamped(int value, int inMin, int inMax, int outMin, int outMax);
    float scale_clamped(int value, int inMin, int inMax, float outMin, float outMax);

};

#endif
