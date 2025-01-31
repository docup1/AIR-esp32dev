#include <LittleFS.h>
#include "commands.h"
#include "file_system.h"
#include "utils.h"

#define BAUDRATE 115200

void setup() {
    Serial.begin(BAUDRATE);
    while (!Serial);
    
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
