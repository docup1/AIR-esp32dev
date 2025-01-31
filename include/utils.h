#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

String normalizePath(String path);
bool checkArgs(String args, int required);
void writeOutput(const String &text);

#endif
