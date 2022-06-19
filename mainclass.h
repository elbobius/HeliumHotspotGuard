#ifndef MAINCLASS_H
#define MAINCLASS_H
#include <Arduino.h>
#include "wifiman.h"
#include "helium.h"
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#endif



void initEepromData();
void loadDataEeprom();
void saveDataEeprom();
void setupWebServer();
void loopWebServer();
void initRestart();
void turnOff();
void turnOn();
void doLoop();
void initMain();
void saveDataEeprom();
void writeIntIntoEEPROM(int , int );
int readIntFromEEPROM(int );
void activateRestart(unsigned long);
void handleRoot();
void handleSave();
void handleRestart();
