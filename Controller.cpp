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

#include <ESP.h>
#include "Controller.h"
#include "Utilities.h"

using utilities::clamp;

Controller::Controller(LedRingDriver* ledDriver, FirmwareManager* fwManager, MqttDriver* mqtt, const char* version)
    : _ledDriver(ledDriver), _fwManager(fwManager), _currentFirmwareVersion(version), _mqtt(mqtt) {}

void Controller::addStateSink(LedStateSink* sink) {
    if (_sinkCount < kMaxSinks) {
        _sinks[_sinkCount++] = {.sink = sink, .pending = true };
    } 
}

void Controller::beginLed(const LedState& ledState) {
    _ledDriver->begin();
    _newState = ledState;
    commitNewState();
}

void Controller::listenToMqtt() {
    _mqtt->setReceivedPropertyCallback([this](const char* node, const char* property, const char* payload) {
        this->handleMqttMessage(node, property, payload);
    });
    Serial.println("Setting OTA state Idle");
    setOtaStatus(kOtaStatusIdle);
}

void Controller::loop() {
    processPendingSinks();
    _mqtt->loop();
    
    if (_newState != _committedState) {
        Serial.printf("Committing new state %d, %d, %d\n", _newState.hue, _newState.saturation, _newState.value);
        commitNewState();
    }
    if (strlen(_firmwareVersionRequested) > 0) {
        Serial.printf("Processing OTA request for %s\n", _firmwareVersionRequested);
        processOtaRequest();
    }
}

// *** private methods ***

// this will set the leds, commit to flash and publish to mqtt
void Controller::commitNewState() {
    if (_committedState == _newState) return;

    for (auto& entry : _sinks) {
          Serial.println("Committing to sink");
          if (!commitSingleSink(&entry, _newState)) {
            Serial.print("Setting pending");
            entry.pending = true;
        }
    }
    _committedState = _newState;
    Serial.printf("_sinks[2].pending = %d\n", _sinks[2].pending);
}

bool Controller::commitSingleSink(SinkEntry* entry, const LedState& ledState) {
    if (entry->sink->acceptsUpdate()) {
        Serial.println("Accepts update");
        entry->sink->onStateCommitted(ledState);
        entry->pending = false;
        return true;
    }
    Serial.print("Does not accept update");
    return false;
}

// callback 
void Controller::handleMqttMessage(const char* node, const char* property, const char* payload) {
    if (!node) return;
    Serial.printf("Handing Mqtt message node=%s property=%s payload=%s\n", node, property, payload);
    if (strcmp(node, kLedNode) == 0) {
        Serial.printf("Processing led property %s\n", property);
        processLedProperty(property, payload);
    } else if (strcmp(node, kFirmwareNode) == 0) {
        Serial.printf("Processing fw property %s\n", property);
        processFirmwareProperty(property, payload);
    }
}

void Controller::processFirmwareProperty(const char* property, const char* payload) {
    if (!property) return;
    if (strcmp(property, kUpdateProperty) == 0) {
        // copy over the version as an indication there is work to be done
        strlcpy(_firmwareVersionRequested, payload, sizeof(_firmwareVersionRequested));
        setOtaStatus(kOtaStatusPending);
        // publish back the reqested version to MQTT (topic without /set) to indicate work in progress
        _mqtt->publishFirmwareProperty(kUpdateProperty, payload);
    }
}

void Controller::processLedProperty(const char* property, const char* payload) {
    // Only HSV externally
    if (strcmp(property, kColorProperty) == 0) {
        int h, s, v;
        if (sscanf(payload, "%d,%d,%d", &h, &s, &v) == 3) {
            _newState.hue = clamp(h, 0, 360);
            _newState.saturation = clamp(s, 0, 100);
            _newState.value = clamp(v, 0, 100);
        }
    } else if (strcmp(property, "mode") == 0) {
        const long mode = strtol(payload, nullptr, 10);
        _newState.mode = clamp(static_cast<uint8_t>(mode), 0, 255);
    }
}

void Controller::processOtaRequest() {
    Serial.printf("Processing OTA request '%s' (now '')", _firmwareVersionRequested, _currentFirmwareVersion);
    if (_firmwareVersionRequested && strcmp(_firmwareVersionRequested, _currentFirmwareVersion) == 0) {
        // Already current, so reset request
        Serial.println("Already current. Setting OTA state Idle");
        setOtaStatus(kOtaStatusIdle, "Already current");
        return;
    }

    setOtaStatus(kOtaStatusUpdating);
    if (!_fwManager->update(_firmwareVersionRequested)) {
        // The update failed, reset request
        // It might have marked the state lost, so mark ready again
        _mqtt->setState(kStateReady);
        setOtaStatus(kOtaStatusFailed, _fwManager->errorMessage());
        return;
    }
    
    // should there be no update but no error either, we reset to idle
    setOtaStatus(kOtaStatusIdle);
    // Does not return if successful (reboots)
}

void Controller::processPendingSinks() {
    
    for (auto& entry : _sinks) {
      if (entry.pending) {
          commitSingleSink(&entry, _committedState);
      }
  }
}

void Controller::setOtaStatus(const char* status, const char* error) {
    _mqtt->publishFirmwareProperty(kStatusProperty, status);
    if (strcmp(status, kOtaStatusIdle) == 0 || strcmp(status, kOtaStatusFailed) == 0) {
        Serial.printf("resetting OTA request (status %s)\n", status);
        _firmwareVersionRequested[0] = 0;
        _mqtt->publishFirmwareProperty(kUpdateProperty, "");
    }
    _mqtt->publishFirmwareProperty(kErrorProperty, error);
}
