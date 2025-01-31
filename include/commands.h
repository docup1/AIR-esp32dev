#ifndef COMMANDS_H
#define COMMANDS_H

#include <Arduino.h>

struct Command {
    String name;
    String args;
};

void handleCommand(String input);
void printHelp();
Command parseCommand(String input);

#endif
