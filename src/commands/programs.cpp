#include "commands/programs.h"
#include "Arduino.h"
#include "console.h"
#include "commands/utils.h"
#include "commands/environment.h"
#include "vm.h"

void handleScript(String args) {
    String path = normalizePath(args);
    fs::File file = LittleFS.open(path);
    
    if (!file) {
        writeOutput("Скрипт не найден!\n");
        return;
    }
    
    while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim();
        if (line.length() > 0) {
            handleCommand(line);
        }
    }
    file.close();
}

void handleCompile(String args) {
    // Ожидается: compile <output_file> <bytecode>
    int firstSpace = args.indexOf(' ');
    if (firstSpace == -1) {
        writeOutput("Использование: compile <output_file> <bytecode>\n");
        return;
    }

    String filename = args.substring(0, firstSpace);
    String bytecodeText = args.substring(firstSpace + 1);

    // Преобразуем текстовое представление в бинарный массив
    uint8_t buffer[MEM_SIZE];
    size_t bufferSize = 0;

    int start = 0;
    while (start < bytecodeText.length() && bufferSize < MEM_SIZE) {
        // Пропускаем разделители: пробелы, запятые, переводы строки
        while (start < bytecodeText.length() && 
              (bytecodeText[start] == ' ' || bytecodeText[start] == ',' ||
               bytecodeText[start] == '\n' || bytecodeText[start] == '\r')) {
            start++;
        }
        if (start >= bytecodeText.length()) break;

        // Находим конец токена
        int end = start;
        while (end < bytecodeText.length() && 
              (bytecodeText[end] != ' ' && bytecodeText[end] != ',' &&
               bytecodeText[end] != '\n' && bytecodeText[end] != '\r')) {
            end++;
        }
        String token = bytecodeText.substring(start, end);
        token.trim();
        if (token.length() > 0) {
            // Если токен начинается с "0x" или "0X", парсим как шестнадцатиричное число
            if (token.startsWith("0x") || token.startsWith("0X")) {
                buffer[bufferSize++] = strtol(token.c_str(), NULL, 16);
            }
            // Если токен начинается с одинарной кавычки, то берём следующий символ как байт
            else if (token.charAt(0) == '\'' && token.length() >= 2) {
                buffer[bufferSize++] = token.charAt(1);
            }
            // Иначе пытаемся разобрать токен как шестнадцатиричное число без префикса
            else {
                buffer[bufferSize++] = strtol(token.c_str(), NULL, 16);
            }
        }
        start = end + 1;
    }

    // Сохраняем бинарный файл
    String fullPath = normalizePath(filename);
    File file = LittleFS.open(fullPath, "w");
    if (!file) {
        writeOutput("Ошибка создания файла: " + fullPath + "\n");
        return;
    }
    file.write(buffer, bufferSize);
    file.close();

    writeOutput("Файл создан: " + fullPath + "\n");
    writeOutput("Размер: " + String(bufferSize) + " байт\n");
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

