#include "helium.h"
#include "ArduinoJson.h"
HeliumHotspot::HeliumHotspot(void) {
  
}
long HeliumHotspot::getCurrentBlock(void) {
  std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
   //client->setFingerprint(fingerprint);
  // Or, if you happy to ignore the SSL certificate, then use the following line instead:
  client->setInsecure();

  HTTPClient https;
  String payload;

   if (https.begin(*client, "https://api.helium.io/v1/blocks/height" )) {  // HTTPS
    // start connection and send HTTP header
    int httpCode = https.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        payload = https.getString();
        Serial.println(payload);
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);
        return doc["data"]["height"];
      }
    } else {
      Serial.printf("[HTTPS] GET https://api.helium.io/v1/hotspots/ failed, error: %s\n", https.errorToString(httpCode).c_str());
    }
    https.end();
  } else {
    Serial.printf("[HTTPS] Unable to connect\n");
  }
  return 0;
}

long HeliumHotspot::getLastActiveBlock(String HotspotId) {
  std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);

  //client->setFingerprint(fingerprint);
  // Or, if you happy to ignore the SSL certificate, then use the following line instead:
  client->setInsecure();

  HTTPClient https;
  String payload;
  Serial.println("get block for hotspot: " + HotspotId);
  if (https.begin(*client, "https://api.helium.io/v1/hotspots/"+ HotspotId )) {  // HTTPS
    // start connection and send HTTP header
    int httpCode = https.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        payload = https.getString();
        Serial.println(payload);
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);
        return doc["data"]["last_change_block"];
      }
    } else {
      Serial.printf("[HTTPS] GET https://api.helium.io/v1/hotspots/ failed, error: %s\n", https.errorToString(httpCode).c_str());
    }
    https.end();
  } else {
    Serial.printf("[HTTPS] Unable to connect\n");
  }
  return 0;
}
