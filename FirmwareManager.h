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

// Since the device won't be in a place that's easily reachable, we let it update itself via OTA.
// We use the MQTT $fw/update property to request an update to a specific version.
// It looks for a specified url: https://base-url/path/device-name.version for the build image (if its current version isn't the same).

#ifndef HEADER_FIRMWARE_MANAGER
#define HEADER_FIRMWARE_MANAGER

#include <WiFiClient.h>

class FirmwareManager {
public:
    void begin(WiFiClient* client, const char* baseUrl, const char* machineId);
    bool update(const char* version);
    const char* errorMessage() const { return _errorMessage; }
private:
	  static constexpr int kErrorBufferSize = 255;
	  static constexpr int kBaseUrlSize = 100;
	  WiFiClient* _client = nullptr;
    char _baseUrl[kBaseUrlSize] = { 0 };
    char _errorMessage[kErrorBufferSize] = { 0 }; 
};
#endif
