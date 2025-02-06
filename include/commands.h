#ifndef COMMANDS_H
#define COMMANDS_H

#include <WString.h>
#include <Arduino.h>
#include "globals.h"

struct Command {
    String name;
    String args;
};

void handleWifi(String args);
void handleWifiMode(String args);
void handleWifiCreate(String args);
void handleWifiConnect(String args);
void handleWifiInfo();
void handleCommand(String input);
void printHelp();
Command parseCommand(String input);
void handleSetEnv(String args);
void handleGetEnv(String args);
void handleUnsetEnv(String args);
void handlePrintEnv();
void handleShutdown();
void handleCompile(String args);
void handleReboot();
void handleStatus();
void handleScript(String args);
void handleInfoLog();
void handleErrLog();
void handleClearLog(String type);
void handleShutdown();
void handleReboot();
void handleStatus();
void handleScript(String args);
void handleInfoLog();
void handleErrLog();
void handleClearLog(String type);
void handleWifiList();
void handleWifiRemove(String args);
bool isWifiExists(const String& ssid);


#endif