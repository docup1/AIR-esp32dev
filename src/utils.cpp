#include "utils.h"
#include <LittleFS.h>

extern String currentDirectory;

String normalizePath(String path) {
    if (path.startsWith("/")) {
        return path;
    }
    return currentDirectory + "/" + path;
}

bool checkArgs(String args, int required) {
    int argCount = 0;
    for (size_t i = 0; i < args.length(); ++i) {
        if (args.charAt(i) == ' ') argCount++;
    }
    argCount++; // To count the last argument

    if (argCount < required) {
        Serial.println("Недостаточно аргументов!");
        return false;
    }
    return true;
}

void writeOutput(const String &text) {
    Serial.print(text);
}
