// Copyright 2021-2025 Rik Essenius
// 
//   Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
//   except in compliance with the License. You may obtain a copy of the License at
// 
//       http://www.apache.org/licenses/LICENSE-2.0
// 
//    Unless required by applicable law or agreed to in writing, software distributed under the License
//    is distributed on an "AS IS" BASIS WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and limitations under the License.

#ifndef HEADER_CONTROLLER
#define HEADER_CONTROLLER

#include "LedRingDriver.h"
#include "FirmwareManager.h"
#include "LedState.h"
#include "LedStateSink.h"
#include "MqttDriver.h"

struct SinkEntry {
    LedStateSink* sink;
    bool pending;
};

class Controller {
public:
    Controller(LedRingDriver* ledDriver, FirmwareManager* fwManager, MqttDriver* mqtt, const char* version);
    static constexpr uint8_t kMaxSinks = 3;
    void addStateSink(LedStateSink* sink);
    void beginLed(const LedState& ledState);
    void listenToMqtt();
    // Called by MqttDriver when a property setter message arrives
    void handleMqttMessage(const char* node, const char* property, const char* payload);

    // Called periodically in loop to handle any pending tasks
    void loop();

 private:
    static constexpr int kFirmwareVersionBufferSize = 50;
    static constexpr bool kResetOtaRequest = true;
    static constexpr auto kOtaStatusIdle = "idle";
    static constexpr auto kOtaStatusPending = "pending";
    static constexpr auto kOtaStatusUpdating = "updating";
    static constexpr auto kOtaStatusFailed = "failed";
    static constexpr auto kOtaStatusCurrent = "current";
    void commitNewState();
    static bool commitSingleSink(SinkEntry* entry, const LedState& ledState);
    void processFirmwareProperty(const char* property, const char* payload);
    void processLedProperty(const char* property, const char* payload);
    void processOtaRequest();
    void processPendingSinks();
    void setOtaStatus(const char* status, const char* error = "");

    uint8_t _sinkCount = 0;
    SinkEntry _sinks[kMaxSinks] = { };
    LedRingDriver* _ledDriver;
    FirmwareManager* _fwManager;
    const char* _currentFirmwareVersion;
    char _firmwareVersionRequested[kFirmwareVersionBufferSize] = { 0 };
    MqttDriver* _mqtt;
    
    LedState _committedState = {};
    LedState _newState = {};
};

#endif
