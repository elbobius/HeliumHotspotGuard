#include "mainclass.h"

void setup() {
  Serial.begin(115200);
  initMain();
}

void loop() {
  doLoop();
}
