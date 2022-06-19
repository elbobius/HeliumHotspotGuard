#include "mainclass.h"
#define MAXADDRESBYTE 54
struct HotspotSettings {
  char minerAddress[MAXADDRESBYTE];
  unsigned int maxBlockDif;
  unsigned int durationOff;
  unsigned int durationNextReboot;
} loadedSettings;

const int RELAY_PIN = 12;
HeliumHotspot heliumHotspot;
unsigned long previousMillisReboot = 0;        // will store last time hotspot was rebooted
bool rebootState = 0;
unsigned long previousMillis = 0;        // will store last time hotspot blocks was updated
const long interval = 10000;           // interval at which hotspot blocks are updated (milliseconds)
const char* host = "HeliumGuard";

ESP8266WebServer server(80);

void initMain() {
  initRestart();
  setupWifi();
  setupWebServer();
  initEepromData();
  loadDataEeprom();
}
void doLoop() {
  // put your main code here, to run repeatedly:
  loopWifi();
  loopWebServer();
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    turnOff();
  }
  turnOn();
}

void initEepromData() {
  EEPROM.begin(256);
}

void loadDataEeprom() {
  Serial.println("EEPROM good mem: " +String(EEPROM.read(0)));
  if (EEPROM.read(0) == 12) {
    EEPROM.get(1,loadedSettings);
    Serial.println("-- saved settings loaded --");
  }
  else {
    strcpy(loadedSettings.minerAddress, "112fupssDgN194c6iExPYJntxtux7bLcZ8R79icDhHjqBTmizKN4");
    loadedSettings.maxBlockDif = 600;
    loadedSettings.durationOff = 10;
    loadedSettings.durationNextReboot = 720;
    saveDataEeprom();
    Serial.println("-- default settings loaded --");
  }
  Serial.println("minerAddress = " + String(loadedSettings.minerAddress));
  Serial.println("maxBlockDif = " + String(loadedSettings.maxBlockDif));
  Serial.println("durationOff = " + String(loadedSettings.durationOff));
  Serial.println("durationNextReboot = " + String(loadedSettings.durationNextReboot));
  
  
}

void saveDataEeprom() {
  EEPROM.write(0, 12);
  EEPROM.put(1,loadedSettings);
  EEPROM.commit();
}

void writeIntIntoEEPROM(int address, int number)
{
  EEPROM.write(address, number >> 8);
  EEPROM.write(address + 1, number & 0xFF);
}

int readIntFromEEPROM(int address)
{
  byte byte1 = EEPROM.read(address);
  byte byte2 = EEPROM.read(address + 1);
  return (byte1 << 8) + byte2;
}

void handleRoot() {
  server.sendHeader("Connection", "close");
  const String serverIndex = "<style type='text/css'>textarea.html-text-box{background-color:ffffff;background-repeat:no-repeat;background-attachment:fixed;border-width:1;border-style:solid;border-color:cccccc;font-family:Arial;font-size:8pt;color:000000}input.html-text-box{background-color:ffffff;font-family:Arial;font-size:8pt;color:000000}</style><body><h1>Helium Hotspot Guard</h1><form action='/action_page'>Hotspot address<br><input type='text' name='address' value='" + String(loadedSettings.minerAddress) + "'><br>Max. block difference<br><input type='text' name='blockdif' value='" + String(loadedSettings.maxBlockDif) + "'><br>Off duration [seconds]<br><input type='text' name='duration' value='" + String(loadedSettings.durationOff) + "'><br>Time between restart: [min]<br><input type='text' name='restartcycle' value='" + String(loadedSettings.durationNextReboot) + "'><br><br><input type='submit' value='Submit'><br></form><form action='/restart'><br>Manual restart hotspot<br><input type='submit' value='Restart'></form></body><br>Firmware update<br><form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'> <input type='submit' value='Update'></form>";
  char Buf[1024];
  serverIndex.toCharArray(Buf, 1024);
  server.send(200, "text/html", Buf);
}

void handleSave() {
  strcpy(loadedSettings.minerAddress, server.arg("address").c_str());
  loadedSettings.maxBlockDif = server.arg("blockdif").toInt();
  loadedSettings.durationOff = server.arg("duration").toInt();
  loadedSettings.durationNextReboot = server.arg("restartcycle").toInt();
  Serial.println("Text received. Contents: ");
  Serial.println("miner address: " + String(loadedSettings.minerAddress));
  Serial.println("block difference:" + String(loadedSettings.maxBlockDif));
  Serial.println("off duration:" + String(loadedSettings.durationOff));
  Serial.println("off interval:" + String(loadedSettings.durationNextReboot));
  saveDataEeprom();
  handleRoot();
}
void handleRestart() {
  activateRestart(millis());
  handleRoot();
}


void setupWebServer() {
  MDNS.begin(host);
  server.on("/", handleRoot);
  server.on("/action_page", handleSave);
  server.on("/restart", handleRestart);
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.setDebugOutput(true);
      WiFiUDP::stopAll();
      Serial.printf("Update: %s\n", upload.filename.c_str());
      uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
      if (!Update.begin(maxSketchSpace)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
      Serial.setDebugOutput(false);
    }
    yield();
  });
  server.begin();
  MDNS.addService("http", "tcp", 80);

  Serial.printf("Ready! Open http://%s.local in your browser\n", host);

}

void loopWebServer(void) {
  server.handleClient();
  MDNS.update();
}

void initRestart() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);
}

void turnOff() {
  unsigned long curBlock = heliumHotspot.getCurrentBlock();
  unsigned long minerBlock = heliumHotspot.getLastActiveBlock(loadedSettings.minerAddress);
  Serial.println("current block = " + String(curBlock));
  Serial.println("miner block = " + String(minerBlock));
  Serial.println("latest activity " + String(curBlock - minerBlock) + " blocks behind");
  Serial.println("max block difference before restart:" + String(loadedSettings.maxBlockDif));
  unsigned long currentMillis = millis();
  if ((curBlock - minerBlock > loadedSettings.maxBlockDif) && (currentMillis - previousMillisReboot >= loadedSettings.durationNextReboot * 60000)) {
    activateRestart(currentMillis);
  }
}

void activateRestart(unsigned long currentMillis) {
  Serial.println("RESTART ACTIVATED");
  digitalWrite(RELAY_PIN, LOW);
  rebootState = 1;
  previousMillisReboot = currentMillis;
}

void turnOn() {
  unsigned long currentMillis = millis();
  if ((currentMillis >=  previousMillisReboot + loadedSettings.durationOff * 1000) && (rebootState == 1)) {
    Serial.println("RESTART DONE");
    digitalWrite(RELAY_PIN, HIGH);
    rebootState = 0;
  }
}
