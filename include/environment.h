#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <Arduino.h>
#include "globals.h"

String getEnvVar(const String& key);
void setEnvVar(const String& key, const String& value);
void unsetEnvVar(const String& key);
void loadEnvVars();
void saveEnvVars();

#endif