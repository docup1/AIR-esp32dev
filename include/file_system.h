#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#pragma once

#include <LittleFS.h>
#include "globals.h"
#include <WString.h>
#include <FS.h>


bool findWifiInList(const String& ssid, String& password);
WifiConfig readWifiConfig();
void writeWifiConfig(const WifiConfig& config);
String formatSize(size_t bytes); 
void initializeFS();
void printTree(String path, int depth = 0);
void listFiles(String path);
void catFile(String path);
void createFile(String path);
void deleteFile(String path);
void createDir(String path);
void deleteDir(String path);
void changeDir(String path);
void copyFile(String args);
void moveFile(String args);
void printFSInfo();
void printWorkingDir();
void handleEcho(String args);
String normalizePath(String path);
void printLastLines(String path, int lines);
void writeToFile(String path, String content, const char* mode = FILE_WRITE);
void addWifiToList(const String& ssid, const String& password);
#endif