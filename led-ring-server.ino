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

#include "secrets.h"
// Contains the configuration data and secrets we don't want to share in GitHub. Contents:
//   #ifndef SECRETS_H
//   #define SECRETS_H
//   constexpr auto kConfigSsid = "wifi-ssid";
//   constexpr auto kConfigWifiPassword = "wifi-password";
//   constexpr auto kConfigDeviceName = "name-of-this-device";
//   constexpr auto kConfigMqttBroker = "name-of-mqtt-broker";
//   static const int   ConfigMqttPort = mqtt-broker-port;
//   constexpr auto kConfigMqttUser = "mqtt-user";
//   constexpr auto kConfigMqttPassword = "mqtt-user-password";
//   constexpr auto kConfigBaseFirmwareUrl = "URL-of-OTA-images-with-trailing-slash/";
//   constexpr char kConfigRootCaCertificate[] PROGMEM = R"rootca(
//   -----BEGIN CERTIFICATE-----
//   Encoded Root CA certificate
//   -----END CERTIFICATE-----
//   )rootca";
//   #endif

#include "Controller.h"
#include "WifiDriver.h"
#include "MqttDriver.h"
#include "FirmwareManager.h"
#include "LedRingDriver.h"
#include "Persistence.h"
#include "Utilities.h"

using utilities::set_logger;

namespace {
	  constexpr auto kName = "LedRingController";
    constexpr auto kVersion = "0.0.7";
    LedState desired_led_state;
    
    LedRingDriver led_ring_driver;
    Persistence persistence;
    FirmwareManager firmware_manager;
    WifiDriver wifi_driver;
    MqttDriver mqtt_driver;
    Controller controller(&led_ring_driver, &firmware_manager, &mqtt_driver, kVersion); 

    constexpr unsigned long kControllerInterval = 50; // ms
    constexpr unsigned long kNetworkCheckInterval = 500; // ms

    unsigned long last_controller_update = 0;
    unsigned long last_network_check = 0;
}

void setup() {
    Serial.begin(115200);
    set_logger([](const char* msg){ Serial.println(msg); });
    delay(250);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    Serial.printf("\nStarting %s %s\n", kName, kVersion);
    controller.addStateSink(&led_ring_driver);
    controller.addStateSink(&persistence);
    controller.addStateSink(&mqtt_driver);
    persistence.begin();
    desired_led_state = *persistence.get();
    Serial.printf("Desired state: %d, %d,%d @ %d\n", desired_led_state.hue, desired_led_state.saturation, desired_led_state.value, desired_led_state.mode);
    controller.beginLed(desired_led_state);
    Serial.printf("Switched on leds");
    
    if (!wifi_driver.begin()) {
        Serial.println("Could not connect to WiFi. Rebooting...");
        ESP.restart();
    }
    wifi_driver.printStatus();

    Serial.printf("Initiating firmware manager...\n");
    firmware_manager.begin(wifi_driver.client(), kConfigBaseFirmwareUrl, wifi_driver.macAddress()); 
    
    Serial.println("Connecting to MQTT...");
    mqtt_driver.begin(wifi_driver.client(), kConfigDeviceName);
    if (!mqtt_driver.isConnected()) {
        Serial.println("Could not connect to MQTT broker. Rebooting...");
        ESP.restart();
    };
    controller.listenToMqtt();
    mqtt_driver.publishDeviceProperty(kMacAddressProperty, wifi_driver.macAddress());
    mqtt_driver.publishDeviceProperty(kIpAddressProperty, wifi_driver.ipAddress());
    mqtt_driver.publishFirmwareProperty(kNameProperty, kName);
    mqtt_driver.publishFirmwareProperty(kVersionProperty, kVersion);
    
    Serial.println("Starting loop");
    digitalWrite(LED_BUILTIN, HIGH);

}

void loop() {
      const unsigned long now = millis();

    // Run controller loop at a regular interval
    if (now - last_controller_update >= kControllerInterval) {
        controller.loop();
        last_controller_update = now;
    }

    // Periodically check network status
    if (now - last_network_check >= kNetworkCheckInterval) {
        wifi_driver.loop();
        mqtt_driver.loop(); 
        last_network_check = now;
    }

    // Let the background tasks run
    delay(1);
}
