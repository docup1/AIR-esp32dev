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

#endif