#include "console.h"
#include "command_includes.h"
#include <map>
#include <functional>
#include <vm.h>
#include <EEPROM.h>

void initializeFS() {
  Serial.println("\nИнициализация LittleFS...");
  if (!LittleFS.begin(true)) {
    Serial.println("Ошибка монтирования, пробуем форматировать...");
    if (!LittleFS.format()) {
      Serial.println("Ошибка форматирования!");
      return;
    }
    if (!LittleFS.begin(true)) {
      Serial.println("Повторная ошибка монтирования!");
      return;
    }
  }
  EEPROM.begin(EEPROM_SIZE);
  loadEnvVars();  // Функция загрузки переменных окружения из EEPROM

  // Создание структуры каталогов
  const char* dirs[] = {
    "/system", "/system/outputs", "/config",
    "/utils", "/utils/scripts", "/utils/tools",
    "/home", "/system/systemdata.dat"
  };

  for (const char* dir : dirs) {
    if (!LittleFS.exists(dir)) {
      if (!LittleFS.mkdir(dir)) {
        Serial.println("Ошибка создания директории: " + String(dir));
      }
    }
  }

  // Создание базовых конфигурационных файлов
  const char* files[] = {
    "/system/board.conf",
    "/system/outputs/info.log",
    "/system/outputs/error.log",
    "/system/settings.conf",
    "/system/device_info.conf",
    "/config/wifi.conf",
    "/config/wifi_list.conf",
    "/config/port_init.conf",
    "/config/interface_init.conf"
  };

  for (const char* file : files) {
    if (!LittleFS.exists(file)) {
      fs::File f = LittleFS.open(file, FILE_WRITE);
      if (!f) {
        Serial.println("Ошибка создания файла: " + String(file));
      } else {
        f.close();
      }
    }
  }
  if (!LittleFS.exists("/config/wifi.conf")) {
    WifiConfig defaultConfig;
    defaultConfig.createMode = false;
    defaultConfig.ssid = "";
    defaultConfig.password = "";
    defaultConfig.channel = 1;
    writeWifiConfig(defaultConfig);
  }
  if (!LittleFS.exists("/system/systemdata.dat")) {
    fs::File f = LittleFS.open("/system/systemdata.dat", "w");
    f.close();
  }
  Serial.println("Файловая система готова\n");
}

Command parseCommand(String input) {
    Command cmd;
    int firstSpace = input.indexOf(' ');
    if (firstSpace == -1) {
        cmd.name = input;
        cmd.args = "";
    } else {
        cmd.name = input.substring(0, firstSpace);
        cmd.args = input.substring(firstSpace + 1);
    }
    return cmd;
}

void handleCommand(String input) {
    int redirectPos = input.indexOf(">");
    String outputFilename;
    int mode = 0;

    if (redirectPos != -1) {
        if (input.charAt(redirectPos + 1) == '>') {
            mode = 2;
            outputFilename = input.substring(redirectPos + 2);
        } else {
            mode = 1;
            outputFilename = input.substring(redirectPos + 1);
        }
        input = input.substring(0, redirectPos);
        outputFilename.trim();
        input.trim();

        if (outputFilename.length() > 0) {
            String fullPath = normalizePath(outputFilename);
            outputFile = LittleFS.open(fullPath, (mode == 1) ? FILE_WRITE : FILE_APPEND);
            outputRedirected = true;
        }
    }

    Command cmd = parseCommand(input);

    // Карта команд с функциями-обработчиками
    std::map<String, std::function<void(String)>> commandHandlers = {
        {"ls", [](String args) { listFiles(args); }},
        {"cat", [](String args) { if (checkArgs(args, 1)) catFile(args); }},
        {"touch", [](String args) { if (checkArgs(args, 1)) createFile(args); }},
        {"echo", [](String args) { handleEcho(args); }},
        {"rm", [](String args) { if (checkArgs(args, 1)) deleteFile(args); }},
        {"mkdir", [](String args) { if (checkArgs(args, 1)) createDir(args); }},
        {"rmdir", [](String args) { if (checkArgs(args, 1)) deleteDir(args); }},
        {"cd", [](String args) { if (checkArgs(args, 1)) changeDir(args); }},
        {"pwd", [](String) { printWorkingDir(); }},
        {"tree", [](String args) { printTree(args); }},
        {"info", [](String) { printFSInfo(); }},
        {"cp", [](String args) { if (checkArgs(args, 2)) copyFile(args); }},
        {"mv", [](String args) { if (checkArgs(args, 2)) moveFile(args); }},
        {"setenv", [](String args) { handleSetEnv(args); }},
        {"getenv", [](String args) { handleGetEnv(args); }},
        {"unsetenv", [](String args) { handleUnsetEnv(args); }},
        {"printenv", [](String) { handlePrintEnv(); }},
        {"shutdown", [](String) { handleShutdown(); }},
        {"reboot", [](String) { handleReboot(); }},
        {"status", [](String) { handleStatus(); }},
        {"skript", [](String args) { handleScript(args); }},
        {"run", [](String args) { handleRun(args); }},
        {"infolog", [](String) { handleInfoLog(); }},
        {"errlog", [](String) { handleErrLog(); }},
        {"clear", [](String args) { handleClearLog("all"); }},
        {"clearinfolog", [](String) { handleClearLog("info"); }},
        {"clearerrlog", [](String) { handleClearLog("error"); }},
        {"wifi", [](String args) { handleWifi(args); }},
        {"wifimode", [](String args) { handleWifiMode(args); }},
        {"wificreate", [](String args) { handleWifiCreate(args); }},
        {"wificonnect", [](String args) { handleWifiConnect(args); }},
        {"wifiinfo", [](String) { handleWifiInfo(); }},
        {"wifilist", [](String) { handleWifiList(); }},
        {"wifiremove", [](String args) { handleWifiRemove(args); }},
        {"wificonnect", [](String args) { handleWifiConnect(args); }},
        {"compile", [](String args) { handleCompile(args); }},
        {"help", [](String) { printHelp(); }}

    };

    if (commandHandlers.find(cmd.name) != commandHandlers.end()) {
        commandHandlers[cmd.name](cmd.args);
    } else {
        writeOutput("Неизвестная команда\n");
    }

    if (outputRedirected && outputFile) {
        outputFile.close();
        outputRedirected = false;
    }
}

void printHelp() {
    String helpText = "Доступные команды:\n";
    helpText += "help - эта справка\n";
    helpText += "ls [path] - список файлов\n";
    helpText += "cat <file> - показать содержимое\n";
    helpText += "touch <file> - создать файл\n";
    helpText += "echo <text> > file - записать в файл\n";
    helpText += "rm <file> - удалить файл\n";
    helpText += "mkdir <dir> - создать директорию\n";
    helpText += "rmdir <dir> - удалить директорию\n";
    helpText += "cd <dir> - сменить директорию\n";
    helpText += "pwd - текущая директория\n";
    helpText += "tree [path] - дерево каталогов\n";
    helpText += "info - информация о ФС\n";
    helpText += "cp <src> <dst> - копировать\n";
    helpText += "mv <src> <dst> - переместить\n";
    helpText += "setenv <key> <value> - установить переменную\n";
    helpText += "getenv <key> - получить переменную\n";
    helpText += "unsetenv <key> - удалить переменную\n";
    helpText += "printenv - все переменные\n";
    helpText += "shutdown - Выключение\n";
    helpText += "reboot - Перезагрузка\n";
    helpText += "status - Состояние системы\n";
    helpText += "skript <file> - Выполнить скрипт\n";
    helpText += "run <file> - Запуск программы\n";
    helpText += "infolog/errlog - Просмотр логов\n";
    helpText += "clear* - Очистка логов\n";
    helpText += "wifi <ssid> <pass> - Добавить сеть в список\n";
    helpText += "wifilist - Список сетей\n";
    helpText += "wifiremove <ssid> - Удалить сеть\n";
    helpText += "wificonnect <ssid> - Подключиться к сети из списка\n";
    helpText += "wifimode <create|connect> - Режим работы WiFi\n";
    helpText += "wificreate <ssid> <pass> [channel] - Настроить точку доступа\n";
    helpText += "wificonnect <ssid> <pass> - Настроить подключение\n";
    helpText += "wifiinfo - Показать текущие настройки\n";
    helpText += "compile <file> <bytecode> - Создать бинарный файл из текстового байт-кода\n";
    writeOutput(helpText);
}






void handleShutdown() {
    writeOutput("Система выключается...\n");
    // ESP.deepSleep(0);
}

void handleReboot() {
    writeOutput("Перезагрузка системы...\n");
    ESP.restart();
}

void handleStatus() {
    String status = "Состояние системы:\n";
    status += "Версия ПО: 1.0\n";
    status += "IP адрес: 192.168.1.1\n";
    writeOutput(status);
}

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
