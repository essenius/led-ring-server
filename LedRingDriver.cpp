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

#include "LedRingDriver.h"
#include "Utilities.h"

using utilities::scale_clamped;

// D5 is GPIO14
static constexpr uint8_t kGpioPort = D5;
static constexpr uint16_t kLedCount = 12;

// Tried FastLED first, but got interference issues with WiFi. NeoPixelBus is more stable.
// NeoEsp8266BitBang800KbpsMethod allows for a chosen GPIO port (unlike some other methods requiring the TX pin).

NeoPixelBus<NeoGrbFeature, NeoEsp8266BitBang800KbpsMethod> ledring(kLedCount, kGpioPort);

void LedRingDriver::begin() {
    ledring.Begin();
    ledring.Show();
}

void LedRingDriver::onStateCommitted(const LedState& state) {
    renderSolidHsv(state);
}

void LedRingDriver::renderSolidHsv(const LedState& ledState) {

    char colorBuffer[50];
    ledState.serializeHsv(colorBuffer, sizeof(colorBuffer));
    Serial.printf("In: %s; ", colorBuffer);
    HsbColor color(
        scale_clamped(ledState.hue, 0, 360, 0.0f, 1.0f),
        scale_clamped(ledState.saturation, 0, 100, 0.0f, 1.0f),
        scale_clamped(ledState.value, 0, 100, 0.0f, 1.0f));
    Serial.printf("Out: %.3f, %.3f, %.3f\n", color.H, color.S, color.B);
    for (uint16_t i = 0; i < kLedCount; i++) {
        ledring.SetPixelColor(i, color);
    }

    ledring.Show();
}
