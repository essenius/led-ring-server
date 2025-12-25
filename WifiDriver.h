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

// This class connects to Wifi and prepares for using TLS connections
// The assumption is that you have your own Root CA certificate that has signed the certificates of the devices and the hosts you use.
// This way, you can use TLS without swithching on the insecure flag.
// The interface hides the TLS complexity by exposing a normal WiFiClient that MQTTDriver and FirmwareManager can use.
// So should you e.g. want to use normal HTTP instead, all you need to change is this class.

#ifndef HEADER_WIFIDRIVER
#define HEADER_WIFIDRIVER

#include "WiFiClient.h"

class WifiDriver {
public:
    bool begin();
    WiFiClient* client();
    const char* macAddress();
    const char* ipAddress();
    void printStatus();
    void loop() { if (!isConnected()) reconnect(); }

    bool isConnected();
    void reconnect();
private:
    static constexpr int kMacAddressSize = 14;
    static constexpr int kIpAddressSize = 16;
    char _macAddress[kMacAddressSize] = {0};
    char _ipAddress[kIpAddressSize] = {0};
    bool _isConnected = false;
};
#endif
