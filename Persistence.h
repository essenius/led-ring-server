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

#ifndef HEADER_PERSISTENCE
#define HEADER_PERSISTENCE

#include "LedState.h"
#include "LedStateSink.h"
#include <cstdint>

struct PersistedLedState {
    uint16_t magicNumber; // initialized to 0 which is different from the magic number
    LedState ledState;
};

class Persistence: public LedStateSink {
public:
    void  begin();
    void onStateCommitted(const LedState& state) override { put(&state); }
    bool acceptsUpdate() override { return true; }
    const LedState* get();
    bool put(const LedState* ledState);
    bool update();

private:
    static constexpr uint8_t kSaveSize = sizeof(PersistedLedState);
    static constexpr uint16_t kMagicNumber = 0xBABE;
    static constexpr unsigned long kMinSaveInterval = 1000; // 1 second

    PersistedLedState _state = {};
    LedState _pendingState = {};          
    bool _putPending = false;
    unsigned long _lastSaveTime = 0;  
};

#endif
