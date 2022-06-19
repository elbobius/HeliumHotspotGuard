#ifndef MY_HELIUM_h
#define MY_HELIUM_h
  #include <Arduino.h>
  #include <ESP8266HTTPClient.h>
  #include <WiFiClientSecureBearSSL.h>
  #include <WiFiClientSecureBearSSL.h>
#endif

class HeliumHotspot{
  private:
  
  public:
    HeliumHotspot(void);
    long getCurrentBlock(void);
    long getLastActiveBlock(String);
};
