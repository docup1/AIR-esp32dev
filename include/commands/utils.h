#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include <LittleFS.h>

String normalizePath(String path);
bool checkArgs(String args, int required);
void writeOutput(const String &text);
void printLastLines(String path, int lines);

extern fs::File outputFile;
extern bool outputRedirected;
extern String currentDirectory;

#endif