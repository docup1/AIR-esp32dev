#include "file_system.h"
#include "utils.h"
#include "globals.h"

String outputFile = "";        // Инициализация переменной
bool outputRedirected = false; // Инициализация переменной
String currentDirectory = "/"; // Инициализация переменной

void initializeFS() {
    Serial.println("\nИнициализация LittleFS...");
    if (!LittleFS.begin(true)) {
        if (!LittleFS.format()) {
            Serial.println("Ошибка форматирования!");
            return;
        }
        if (!LittleFS.begin(true)) {
            Serial.println("Ошибка монтирования!");
            return;
        }
    }
    Serial.println("Файловая система готова\n");
}

void printTree(String path, int depth) {
    String prefix;
    for (int i = 0; i < depth; i++) prefix += "|   ";

    String targetPath = path.length() > 0 ? normalizePath(path) : currentDirectory;
    File dir = LittleFS.open(targetPath);

    if (!dir || !dir.isDirectory()) {
        writeOutput(prefix + "├── [Ошибка открытия]\n");
        return;
    }

    writeOutput(prefix + "└── " + String(dir.name()) + "\n");

    File file = dir.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            printTree(String(file.name()), depth + 1);
        } else {
            writeOutput(prefix + "    └── " + String(file.name()) + "\n");
        }
        file = dir.openNextFile();
    }
    dir.close();
}

void listFiles(String path) {
    String targetPath = path.length() > 0 ? normalizePath(path) : currentDirectory;
    File dir = LittleFS.open(targetPath);

    if (!dir || !dir.isDirectory()) {
        writeOutput("Директория не найдена!\n");
        return;
    }

    File file = dir.openNextFile();
    while (file) {
        String entry = String(file.name()) + "\t" + 
                      (file.isDirectory() ? "[DIR]" : String(file.size()) + " байт");
        writeOutput(entry + "\n");
        file = dir.openNextFile();
    }
    dir.close();
}



void createFile(String path) {
    String fullPath = normalizePath(path);
    File file = LittleFS.open(fullPath, FILE_WRITE);
    if (file) {
        writeOutput("Файл создан: " + fullPath + "\n");
        file.close();
    } else {
        writeOutput("Ошибка создания файла!\n");
    }
}

void catFile(String path) {
    String fullPath = normalizePath(path);
    File file = LittleFS.open(fullPath);
    if (!file) {
        writeOutput("Файл не найден!\n");
        return;
    }

    while (file.available()) {
        writeOutput(String((char)file.read()));
    }
    writeOutput("\n");
    file.close();
}

void deleteFile(String path) {
    String fullPath = normalizePath(path);
    if (LittleFS.remove(fullPath)) {
        writeOutput("Файл удален: " + fullPath + "\n");
    } else {
        writeOutput("Ошибка удаления!\n");
    }
}

void createDir(String path) {
    String fullPath = normalizePath(path);
    if (LittleFS.mkdir(fullPath)) {
        writeOutput("Директория создана: " + fullPath + "\n");
    } else {
        writeOutput("Ошибка создания директории!\n");
    }
}

void deleteDir(String path) {
    String fullPath = normalizePath(path);
    if (LittleFS.rmdir(fullPath)) {
        writeOutput("Директория удалена: " + fullPath + "\n");
    } else {
        writeOutput("Ошибка удаления директории!\n");
    }
}

void changeDir(String path) {
    if (path == "..") {
        int lastSlash = currentDirectory.lastIndexOf('/');
        if (lastSlash > 0) {
            currentDirectory = currentDirectory.substring(0, lastSlash);
        }
        writeOutput("Текущая директория: " + currentDirectory + "\n");
        return;
    }

    String newPath = normalizePath(path);
    File dir = LittleFS.open(newPath);
    if (dir && dir.isDirectory()) {
        currentDirectory = newPath;
        writeOutput("Текущая директория: " + currentDirectory + "\n");
    } else {
        writeOutput("Директория не существует!\n");
    }
    dir.close();
}

void copyFile(String args) {
    int spacePos = args.indexOf(' ');
    String sourcePath = normalizePath(args.substring(0, spacePos));
    String destPath = normalizePath(args.substring(spacePos + 1));

    File source = LittleFS.open(sourcePath, FILE_READ);
    if (!source) {
        writeOutput("Файл источника не найден!\n");
        return;
    }

    File dest = LittleFS.open(destPath, FILE_WRITE);
    if (!dest) {
        writeOutput("Ошибка создания файла назначения!\n");
        source.close();
        return;
    }

    while (source.available()) {
        dest.write(source.read());
    }

    writeOutput("Файл скопирован: " + sourcePath + " -> " + destPath + "\n");
    source.close();
    dest.close();
}

void moveFile(String args) {
    int spacePos = args.indexOf(' ');
    String sourcePath = normalizePath(args.substring(0, spacePos));
    String destPath = normalizePath(args.substring(spacePos + 1));

    if (LittleFS.rename(sourcePath, destPath)) {
        writeOutput("Файл перемещен: " + sourcePath + " -> " + destPath + "\n");
    } else {
        writeOutput("Ошибка перемещения файла!\n");
    }
}

void printFSInfo() {
    size_t total = LittleFS.totalBytes();
    size_t used = LittleFS.usedBytes();
    String info = "Файловая система:\n";
    info += "Всего: " + String(total) + " байт\n";
    info += "Использовано: " + String(used) + " байт\n";
    info += "Свободно: " + String(total - used) + " байт\n";
    writeOutput(info);
}

void printWorkingDir() {
    writeOutput("Текущая директория: " + currentDirectory + "\n");
}



void handleEcho(String args) {
    if (args.length() > 0) {
        writeOutput(args + "\n");
    } else {
        writeOutput("Ошибка: Нет текста для вывода!\n");
    }
}




