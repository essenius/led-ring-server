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

// This class implements the homie convention (https://homieiot.github.io/specification/) 
// to allow home automation systems like OpenHAB to autodetect it.

#ifndef HEADER_MQTTDRIVER
#define HEADER_MQTTDRIVER

#include <functional>
#include <Client.h>

#include "LedState.h"
#include "LedStateSink.h"

using MqttPropertyCallback = std::function<void(const char* node, const char* property, const char* payload)>;

// the constants we need outside the class as well

constexpr auto kMacAddressProperty = "mac-address";
constexpr auto kIpAddressProperty = "ip-address";

constexpr auto kLedNode = "led";
constexpr auto kFirmwareNode = "$fw";
constexpr auto kColorProperty = "color";
constexpr auto kModeProperty = "mode";

constexpr auto kStateInit = "init";
constexpr auto kStateReady = "ready";
constexpr auto kStateDisconnected = "disconnected";
constexpr auto kStateLost = "lost";

constexpr auto kNameProperty = "name";
constexpr auto kVersionProperty = "version";
constexpr auto kStatusProperty = "status";
constexpr auto kUpdateProperty = "update";
constexpr auto kErrorProperty = "error";

class MqttDriver : public LedStateSink {
public:
    void begin(Client* client, const char* clientName);
    bool connect();
    void disconnect();
    bool isConnected();
    void onStateCommitted(const LedState& state) override;
    bool acceptsUpdate() override { return isConnected(); }
    bool loop();
    void publishDeviceProperty(const char* propertyName, const char* payload);
    void publishLedProperty(const char* property, const char* payload);
    void publishFirmwareProperty(const char* property, const char* payload);
    void publishProperty(const char* node, const char* property, const char* payload);
    void setState(const char* state);
    void setReceivedPropertyCallback(MqttPropertyCallback cb) { _propertyCallback = cb; }

private:
    static constexpr auto kTopicBufferSize = 255;
    static constexpr auto kBaseTopicBufferSize = 200;   // just node/property
    static constexpr auto kColorBufferSize = 20;        // should be plenty for 3 uints and 2 commas
    static constexpr auto kPayloadBufferSize = 100;     // longest is the $properties list

    static constexpr auto kBaseTopicTemplate = "homie/%s/%s";
    static constexpr auto kIntegerType = "integer";
    static constexpr auto kStringType = "string";
    static constexpr auto kColorType = "color";

    static constexpr int kWillQos = 1;
    static constexpr bool kRetainWill = true;
    static constexpr bool kRetainMessage = true;
    static constexpr auto kStateProperty = "$state";
    static constexpr auto kDeviceNode = "device";

    static constexpr auto kByteFormat = "0-255";
    static constexpr auto kColorHsvFormat = "hsv";

    const char* _clientName = nullptr;
    bool _wasAnnounced = false;
    char _topicBuffer[kTopicBufferSize] = { };
    MqttPropertyCallback _propertyCallback = nullptr;

    bool announceDevice();
    void announceNode(const char* baseTopic, const char* name, const char* properties);
    void announceProperty(const char* baseTopic, const char* name, const char* dataType, const char* format, bool settable);
    void mqttCallback(const char* topic, const uint8_t* payload, unsigned int length);
    bool publishEntity(const char* baseTopic, const char* entity, const char* payload);
    void subscribeSetters();
    static bool tryParseTopic(char* copyTopic, const char*& node, const char*& property, bool& isSetter);
};

#endif
