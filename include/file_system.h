#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <LittleFS.h>

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

#endif
