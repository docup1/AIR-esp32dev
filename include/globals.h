#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <LittleFS.h>
#include <EEPROM.h>

#define EEPROM_SIZE 512
#define MAX_ENV_VARS 20

struct EnvVar {
    String key;
    String value;
};
struct WifiConfig {
    bool createMode;
    String ssid;
    String password;
    int channel;
};
extern fs::File outputFile;
extern bool outputRedirected;
extern String currentDirectory;
extern EnvVar envVars[MAX_ENV_VARS];
extern int envVarCount;

#endif