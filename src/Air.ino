#include <LittleFS.h>
#include <FS.h>
#include <EEPROM.h>
#include <console.h>

#define BAUDRATE 115200

// =================== Основной скетч ===================
void setup() {
  Serial.begin(BAUDRATE);
  initializeFS();
  printHelp();
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.length() > 0) {
      handleCommand(input);
    }
  }
}
