#include "command_includes.h"
#include <FS.h>
#include <EEPROM.h>

// =================== Функции работы с файловой системой (LittleFS) ===================



void printTree(String path, int depth) {
  String prefix = "";
  for (int i = 0; i < depth; i++) {
    prefix += "|   ";
  }

  // Если указан пустой путь, используем текущую директорию (currentDirectory из globals.h)
  String targetPath = (path.length() > 0) ? normalizePath(path) : currentDirectory;
  fs::File dir = LittleFS.open(targetPath);

  if (!dir || !dir.isDirectory()) {
    writeOutput(prefix + "├── [Ошибка открытия: " + targetPath + "]\n");
    return;
  }

  // Выводим имя текущей директории
  writeOutput(prefix + "└── " + String(dir.name()) + "\n");

  // Рекурсивный обход содержимого директории
  fs::File file = dir.openNextFile();
  while (file) {
    String entryName = String(file.name());
    if (file.isDirectory()) {
      printTree(entryName, depth + 1);
    } else {
      writeOutput(prefix + "    └── " + entryName + "\n");
    }
    file = dir.openNextFile();
  }
  dir.close();
}

void listFiles(String path) {
  String targetPath = (path.length() > 0) ? normalizePath(path) : currentDirectory;
  fs::File dir = LittleFS.open(targetPath);

  if (!dir || !dir.isDirectory()) {
    writeOutput("Директория не найдена!\n");
    return;
  }

  fs::File file = dir.openNextFile();
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
  fs::File file = LittleFS.open(fullPath, FILE_WRITE);
  if (file) {
    writeOutput("Файл создан: " + fullPath + "\n");
    file.close();
  } else {
    writeOutput("Ошибка создания файла!\n");
  }
}

void catFile(String path) {
  String fullPath = normalizePath(path);
  fs::File file = LittleFS.open(fullPath);
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
  fs::File dir = LittleFS.open(newPath);
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

  fs::File source = LittleFS.open(sourcePath, FILE_READ);
  if (!source) {
    writeOutput("Файл источника не найден!\n");
    return;
  }
  fs::File dest = LittleFS.open(destPath, FILE_WRITE);
  if (!dest) {
    writeOutput("Ошибка создания файла назначения!\n");
    source.close();
    return;
  }
  size_t bufSize = 512;
  uint8_t* buffer = new uint8_t[bufSize];
  while (source.available()) {
    size_t bytesRead = source.read(buffer, bufSize);
    dest.write(buffer, bytesRead);
  }
  delete[] buffer;
  source.close();
  dest.close();
  writeOutput("Файл скопирован: " + sourcePath + " -> " + destPath + "\n");
}

void moveFile(String args) {
  copyFile(args);
  int spacePos = args.indexOf(' ');
  String sourcePath = normalizePath(args.substring(0, spacePos));
  deleteFile(sourcePath);
}

String formatSize(size_t bytes) {
  const char* units[] = {"байт", "KB", "MB", "GB"};
  size_t unit = 0;
  double size = bytes;
  while (size >= 1024 && unit < 3) {
    size /= 1024;
    unit++;
  }
  return String(size, 2) + " " + units[unit];
}

void printFSInfo() {
  size_t total = LittleFS.totalBytes();
  size_t used = LittleFS.usedBytes();
  String info = "Файловая система:\n";
  info += "Всего: " + formatSize(total) + "\n";
  info += "Использовано: " + formatSize(used) + "\n";
  info += "Свободно: " + formatSize(total - used) + "\n";
  writeOutput(info);
}

void printWorkingDir() {
  writeOutput("Текущая директория: " + currentDirectory + "\n");
}

void handleEcho(String args) {
  if (args.length() == 0) {
    writeOutput("\n"); // Если аргументов нет, выводим пустую строку
    return;
  }
  int redirectPos = args.indexOf('>');
  String outputText = args;
  String outputFile = "";
  bool appendMode = false;
  if (redirectPos != -1) {
    outputText = args.substring(0, redirectPos);
    outputText.trim();
    String filePart = args.substring(redirectPos + 1);
    filePart.trim();
    if (filePart.startsWith(">")) {
      appendMode = true;
      filePart = filePart.substring(1);
      filePart.trim();
    }
    outputFile = filePart;
  }
  String result = "";
  for (size_t i = 0; i < outputText.length(); i++) {
    char c = outputText[i];
    if (c == '\\' && i + 1 < outputText.length()) {
      char nextChar = outputText[i + 1];
      switch (nextChar) {
        case 'n': result += '\n'; break;
        case 't': result += '\t'; break;
        case 'r': result += '\r'; break;
        case '\\': result += '\\'; break;
        case '$': result += '$'; break;
        default: result += c; result += nextChar; break;
      }
      i++;
    } else if (c == '$' && i + 1 < outputText.length()) {
      int endPos = i + 1;
      while (endPos < outputText.length() && 
             (isalnum(outputText[endPos]) || outputText[endPos] == '_')) {
        endPos++;
      }
      String varName = outputText.substring(i + 1, endPos);
      String varValue = getEnvVar(varName);
      result += varValue;
      i = endPos - 1;
    } else {
      result += c;
    }
  }
  if (outputFile.length() > 0) {
    String fullPath = normalizePath(outputFile);
    fs::File file = LittleFS.open(fullPath, appendMode ? FILE_APPEND : FILE_WRITE);
    if (file) {
      file.print(result);
      file.close();
    } else {
      writeOutput("Ошибка: Не удалось открыть файл для записи!\n");
    }
  } else {
    writeOutput(result + "\n");
  }
}

void writeToFile(String path, String content, const char* mode) {
  String fullPath = normalizePath(path);
  fs::File file = LittleFS.open(fullPath, mode);
  if (file) {
    file.print(content);
    file.close();
  } else {
    writeOutput("Ошибка записи в файл: " + fullPath + "\n");
  }
}





// =================== Конец интегрированного кода ===================

/*
Замечания:
– Функции writeOutput(), printHelp(), handleCommand(), normalizePath(), loadEnvVars(), 
  writeWifiConfig() и переменная currentDirectory должны быть определены в соответствующих файлах 
  (utils.h, commands.h, environment.h, globals.h) или заменены на ваши реализации.
– EEPROM_SIZE – размер EEPROM (например, 512), должен быть объявлен в globals.h или здесь.
– Этот скетч предназначен для работы с файловой системой LittleFS.
*/