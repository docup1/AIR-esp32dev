#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include "globals.h"
#include "environment.h" // Для getEnvVar

String normalizePath(String path);
bool checkArgs(String args, int required);
void writeOutput(const String &text);

#endif