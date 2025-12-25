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

#ifndef HEADER_LEDDRIVER
#define HEADER_LEDDRIVER

#include <NeoPixelBus.h>
#include "LedState.h"
#include "LedStateSink.h"

class LedRingDriver: public LedStateSink {
public:
    void begin();
    void renderSolidHsv(const LedState& ledState);
    bool acceptsUpdate() override { return true; }
    void onStateCommitted(const LedState& state) override;
};

#endif
