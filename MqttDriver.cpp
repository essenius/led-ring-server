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
#include <PubSubClient.h>
#include "LedState.h"
#include "MqttDriver.h"
#include "Utilities.h"
#include "secrets.h"

using utilities::snprintf_t;
using utilities::build_topic;
using utilities::next_token;

PubSubClient mqttClient;

void MqttDriver::begin(Client* client, const char* clientName) {
    mqttClient.setClient(*client);
    _clientName = clientName;
    mqttClient.setBufferSize(512);
    mqttClient.setServer(kConfigMqttBroker, kConfigMqttPort);
    mqttClient.setCallback([this](const char* topic, const uint8_t* payload, const unsigned int length) {
        this->mqttCallback(topic, payload, length);
    });
    if (!connect()) {
        Serial.printf("Could not connect to MQTT broker: state %d\n", mqttClient.state());     
    }
}

bool MqttDriver::connect() {
    if (isConnected()) return true;
  
    snprintf_t(_topicBuffer, kBaseTopicTemplate, _clientName, kStateProperty);
    bool connectionSucceeded;
    if (strlen(kConfigMqttUser) == 0) {
        connectionSucceeded = mqttClient.connect(kConfigDeviceName, _topicBuffer, kWillQos, kRetainWill, kStateLost);
    } else {
        connectionSucceeded = mqttClient.connect(kConfigDeviceName, kConfigMqttUser, kConfigMqttPassword, _topicBuffer, kWillQos, kRetainWill, kStateLost);
    }
    if (!connectionSucceeded) {
        Serial.println("Could not connect to MQTT broker");
        return false;
    }
    if (announceDevice()) {
        subscribeSetters();
        return true;
    } 
    Serial.println("Could not announce device on MQTT");
    return false; 
}

void MqttDriver::disconnect() {
    if (isConnected()) {
        setState(kStateDisconnected);
        mqttClient.disconnect();    
    }
}

bool MqttDriver::isConnected() {
    return mqttClient.connected();
}

bool MqttDriver::loop() {
    if (!mqttClient.connected()) connect();
    return mqttClient.loop();
}

void MqttDriver::onStateCommitted(const LedState& state) {
    char buffer[kColorBufferSize]; 
    if (state.serializeHsv(buffer, sizeof(buffer))) {
        Serial.printf("Publishing color %s\n", buffer);
        publishLedProperty(kColorProperty, buffer);  
    }

    if (state.serializeMode(buffer, sizeof(buffer))) {
        publishLedProperty(kModeProperty, buffer);  
    }
}

void MqttDriver::publishDeviceProperty(const char* propertyName, const char* payload) {
    publishProperty(kDeviceNode, propertyName, payload);
}

void MqttDriver::publishLedProperty(const char* property, const char* payload) {
    publishProperty(kLedNode, property, payload);
}

void MqttDriver::publishFirmwareProperty(const char* property, const char* payload) {
    publishProperty(kFirmwareNode, property, payload);
}

void MqttDriver::publishProperty(const char* node, const char* property, const char* payload) {
    char path[kBaseTopicBufferSize];
    strlcpy(path, node, sizeof(path));
    strlcat(path, "/", sizeof(path));
    strlcat(path, property, sizeof(path));
    Serial.printf("Publishing %s to %s\n", payload, path);
    publishEntity(_clientName, path, payload);
}

void MqttDriver::setState(const char* state) {
    publishEntity(_clientName, kStateProperty, state);  
}

// *** private methods ***

bool MqttDriver::announceDevice() {
    if (_wasAnnounced) return true;
    char baseTopic[kBaseTopicBufferSize];
    char payload[kPayloadBufferSize];

    // homie
    if (!publishEntity(_clientName, "$homie", "4.0")) return false;
    setState(kStateInit);

    // $name and $nodes
    publishEntity(_clientName, "$name", _clientName);
    snprintf_t(payload, "%s,%s", kDeviceNode, kLedNode, kFirmwareNode);
    publishEntity(_clientName, "$nodes", payload);

    // Device node
    snprintf_t(payload, "%s,%s", kMacAddressProperty, kIpAddressProperty);
    build_topic(baseTopic, sizeof(baseTopic), _clientName, kDeviceNode);
    announceNode(baseTopic, kDeviceNode, payload);
    
    // Device properties
    build_topic(baseTopic, sizeof(baseTopic), _clientName, kDeviceNode, kMacAddressProperty);
    announceProperty(baseTopic, kMacAddressProperty, kStringType, "", false);
    build_topic(baseTopic, sizeof(baseTopic), _clientName, kDeviceNode, kIpAddressProperty);
    announceProperty(baseTopic, kIpAddressProperty, kStringType, "", false);

    // LED node
    build_topic(baseTopic, sizeof(baseTopic), _clientName, kLedNode);
    snprintf_t(payload, "%s,%s", kColorProperty, kModeProperty);
    announceNode(baseTopic, kLedNode, payload);

    // LED properties
    build_topic(baseTopic, sizeof(baseTopic), _clientName, kLedNode, kColorProperty);
    announceProperty(baseTopic, kColorProperty, kColorType, kColorHsvFormat, true); 

    build_topic(baseTopic, sizeof(baseTopic), _clientName, kLedNode, kModeProperty);
    announceProperty(baseTopic, kModeProperty, kIntegerType, kByteFormat, true);

    // Firmware node
    build_topic(baseTopic, sizeof(baseTopic), _clientName, kFirmwareNode);
    snprintf_t(payload, "%s,%s,%s,%s", kNameProperty, kVersionProperty, kStatusProperty, kUpdateProperty, kErrorProperty);
    announceNode(baseTopic, kFirmwareNode, payload);
    
    // Firmware properties
    build_topic(baseTopic, sizeof(baseTopic), _clientName, kFirmwareNode, kNameProperty);
    announceProperty(baseTopic, kNameProperty, kStringType, "", false); 

    build_topic(baseTopic, sizeof(baseTopic), _clientName, kFirmwareNode, kVersionProperty);
    announceProperty(baseTopic, kVersionProperty, kStringType, "", false); 

    build_topic(baseTopic, sizeof(baseTopic), _clientName, kFirmwareNode, kStatusProperty);
    announceProperty(baseTopic, kStatusProperty, kStringType, "", false); 

    build_topic(baseTopic, sizeof(baseTopic), _clientName, kFirmwareNode, kUpdateProperty);
    announceProperty(baseTopic, kStatusProperty, kStringType, "", true); 

    build_topic(baseTopic, sizeof(baseTopic), _clientName, kFirmwareNode, kErrorProperty);
    announceProperty(baseTopic, kErrorProperty, kStringType, "", false); 

    setState(kStateReady);
    _wasAnnounced = true;
    return true;
}

void MqttDriver::announceNode(const char* baseTopic, const char* name, const char* properties) {
    publishEntity(baseTopic, "$name", name);
    publishEntity(baseTopic, "$properties", properties);
}

void MqttDriver::announceProperty(const char* baseTopic, const char* name, const char* dataType, const char* format, bool settable) {
    publishEntity(baseTopic, "$name", name);
    publishEntity(baseTopic, "$datatype", dataType);
    if (strlen(format) > 0) {
        publishEntity(baseTopic, "$format", format);
    }
    if (settable) {
        publishEntity(baseTopic, "$settable", "true");
    }
}

void MqttDriver::mqttCallback(const char* topic, const uint8_t* payload, const unsigned length) {
    // ignore messages with an empty payload or a payload too long to be uint8_t.
    Serial.printf("Called with topic %s", topic);
    if (length == 0 || length > kPayloadBufferSize) return;

    char topicCopy[kTopicBufferSize];
    strlcpy(topicCopy, topic, sizeof(topicCopy));
    const char* node;
    const char* property;
    bool isSetter;
    if (!tryParseTopic(topicCopy, node, property, isSetter)) return;

    char payloadStr[kPayloadBufferSize];
    for (unsigned int i = 0; i < length; i++) {
        payloadStr[i] = static_cast<char>(payload[i]);
    }
    payloadStr[length] = 0;

    Serial.printf(", payload %s\n", payloadStr);

    if (_propertyCallback) {
        _propertyCallback(node, property, payloadStr);
    } 
}

bool MqttDriver::publishEntity(const char* baseTopic, const char* entity, const char* payload) {
    if (!mqttClient.connected() && !connect()) return false;

    if (!snprintf_t(_topicBuffer, kBaseTopicTemplate, baseTopic, entity)) return false;
    return mqttClient.publish(_topicBuffer, payload, kRetainMessage);
}

void MqttDriver::subscribeSetters() {
    char topic[kTopicBufferSize];

    const char* properties[] = { kColorProperty, kModeProperty };
    for (const char* property : properties) {
        if (snprintf_t(topic, "homie/%s/%s/%s/set", _clientName, kLedNode, property)) {
            mqttClient.subscribe(topic);
        }
    }
    if (snprintf_t(topic, "homie/%s/$fw/update/set", _clientName)) {
        mqttClient.subscribe(topic);
    }    
}

bool MqttDriver::tryParseTopic(char* copyTopic, const char*& node, const char*& property, bool& isSetter) {
    constexpr int delimiter = '/';
	// only pass messages like homie/device/node/property[/set]
	// ignore the first two, as the subscription ensures they are correct
	if (next_token(&copyTopic, delimiter) == nullptr) return false; 
    if (next_token(&copyTopic, delimiter) == nullptr) return false; 
    node = next_token(&copyTopic, delimiter);
    if (node == nullptr) return false;
    property = next_token(&copyTopic, delimiter);
    if (property == nullptr) return false;
    const auto set = next_token(&copyTopic, delimiter);
    isSetter = set && strcmp(set, "set") == 0;
    return true;
}
