#ifndef CNSOLE_H
#define CNSOLE_H

#include <WString.h>
#include <Arduino.h>

struct Command {
    String name;
    String args;
};

void initializeFS();

void handleCommand(String input);
void printHelp();
Command parseCommand(String input);

void handleShutdown();
void handleCompile(String args);
void handleReboot();
void handleStatus();
void handleScript(String args);

void handleShutdown();
void handleReboot();
void handleStatus();
void handleScript(String args);




#endif