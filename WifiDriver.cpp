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
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "WifiDriver.h"
#include "Utilities.h"
#include "secrets.h"

#include <cstdint>
#include <cstring>

using utilities::snprintf_t;

constexpr auto kLocalHostIp = "127.0.0.1";

namespace {
    BearSSL::WiFiClientSecure wifi_client;
    BearSSL::X509List ca_cert(kConfigRootCaCertificate);
}

bool WifiDriver::begin() {
    WiFi.mode(WIFI_STA);
    wifi_client.setTrustAnchors(&ca_cert);
    if (!WiFi.hostname(kConfigDeviceName)) {
        Serial.println("Could not set host name");
    }
    WiFi.begin(kConfigSsid, kConfigWifiPassword);
    Serial.print("Connecting");
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        attempts++;
        Serial.print(".");
    }
    return isConnected();
}

WiFiClient* WifiDriver::client() {
    return &wifi_client;
}

bool WifiDriver::isConnected() { 
    return WiFi.status() == WL_CONNECTED; 
}

void WifiDriver::reconnect() {
    if (isConnected()) return;
    WiFi.reconnect();
}

const char* WifiDriver::macAddress() {
    if (strlen(_macAddress) > 0) return _macAddress;
    uint8_t mac[6];
    WiFi.macAddress(mac);
    snprintf_t(_macAddress, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]); 
    return _macAddress; 
}

const char* WifiDriver::ipAddress() {
    if (strlen(_ipAddress) > 0) return _ipAddress;
    // make sure we don't capture the ip address when we're not connected
    if (!isConnected()) return kLocalHostIp;
    IPAddress ip = WiFi.localIP();
    snprintf_t(_ipAddress, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
    return _ipAddress;
}

void WifiDriver::printStatus() {
    Serial.printf("\nConnected to SSID: %s, IP: %s, name: %s, Mac address: %s\n", WiFi.SSID(), ipAddress(), WiFi.hostname().c_str(), macAddress() );
}
