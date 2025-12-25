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
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <WiFiClient.h>

#include "FirmwareManager.h" 
#include "Utilities.h"

using utilities::snprintf_t;
using utilities::build_url;

void FirmwareManager::begin(WiFiClient* client, const char* baseUrl, const char* machineId) {
    _client = client;
    build_url(_baseUrl, sizeof(_baseUrl), baseUrl, machineId);
	  strlcat(_baseUrl, ".", sizeof(_baseUrl));
}

bool FirmwareManager::update(const char* version) {
	  Serial.printf("Updating firmware to %s\n", version);
	  ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);

	  char imageUrl[kBaseUrlSize];
	  build_url(imageUrl, sizeof(imageUrl), _baseUrl, version);
    Serial.printf("Fetching %s\n", imageUrl);

  	t_httpUpdate_return returnValue = ESPhttpUpdate.update(*_client, imageUrl);
  
  	switch(returnValue) {
    		case HTTP_UPDATE_OK:
      			// never returns
      			return true;
    		case HTTP_UPDATE_FAILED:
      			snprintf_t(_errorMessage, "%s (%d)", ESPhttpUpdate.getLastErrorString().c_str(), ESPhttpUpdate.getLastError());
      			Serial.printf("OTA update failed: %s\n", _errorMessage);
      			return false;
    
    		case HTTP_UPDATE_NO_UPDATES:            
      			// ignore, will not happen as it only occurs when the server returns 304 Not Modified
           
    		default:
      			// should not happen
    			return true;
  	}
}
