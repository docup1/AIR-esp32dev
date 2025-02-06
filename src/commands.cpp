#include "commands.h"
#include "file_system.h"
#include "utils.h"
#include "globals.h"
#include <map>
#include <functional>
#include <vm.h>
#include <commands/handlerun.h>
fs::File outputFile;
bool outputRedirected = false;
String currentDirectory = "/";
EnvVar envVars[MAX_ENV_VARS];
int envVarCount = 0;

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

void handleSetEnv(String args) {
    int spacePos = args.indexOf(' ');
    if (spacePos == -1) {
        writeOutput("Использование: setenv <key> <value>\n");
        return;
    }
    String key = args.substring(0, spacePos);
    String value = args.substring(spacePos + 1);
    setEnvVar(key, value);
}

void handleGetEnv(String args) {
    String value = getEnvVar(args);
    writeOutput(value + "\n");
}

void handleUnsetEnv(String args) {
    unsetEnvVar(args);
}

void handlePrintEnv() {
    for (int i = 0; i < envVarCount; i++) {
        writeOutput(envVars[i].key + "=" + envVars[i].value + "\n");
    }
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



void handleInfoLog() {
    printLastLines("/system/outputs/info.log", 6);
}

void handleErrLog() {
    printLastLines("/system/outputs/error.log", 6);
}

void handleClearLog(String type) {
    if (type == "info" || type == "all") {
        LittleFS.remove("/system/outputs/info.log");
        createFile("/system/outputs/info.log");
    }
    if (type == "error" || type == "all") {
        LittleFS.remove("/system/outputs/error.log");
        createFile("/system/outputs/error.log");
    }
    writeOutput("Логи очищены\n");
}
void handleWifi(String args) {
    int spacePos = args.indexOf(' ');
    if (spacePos == -1) {
        writeOutput("Использование: wifi <SSID> <password>\n");
        return;
    }

    String ssid = args.substring(0, spacePos);
    String password = args.substring(spacePos + 1);

    // Проверяем, существует ли уже такая сеть
    if (isWifiExists(ssid)) {
        writeOutput("Сеть уже существует!\n");
        return;
    }

    // Добавляем сеть в список
    addWifiToList(ssid, password);

    // Обновляем текущую конфигурацию (опционально)
    WifiConfig config = readWifiConfig();
    config.ssid = ssid;
    config.password = password;
    writeWifiConfig(config);

    writeOutput("Сеть добавлена и настроена!\n");
}

// Новые функции для работы с Wi-Fi
bool isWifiExists(const String& ssid) {
    fs::File file = LittleFS.open("/config/wifi_list.conf", FILE_READ);
    if (!file) return false;

    while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim();
        if (line.startsWith(ssid + ";")) {
            file.close();
            return true;
        }
    }
    file.close();
    return false;
}

void handleWifiList() {
    fs::File file = LittleFS.open("/config/wifi_list.conf", FILE_READ);
    if (!file) {
        writeOutput("Список сетей пуст\n");
        return;
    }

    writeOutput("Список сохранённых сетей:\n");
    while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim();
        int separator = line.indexOf(';');
        if (separator != -1) {
            String ssid = line.substring(0, separator);
            writeOutput("SSID: " + ssid + "\n");
        }
    }
    file.close();
}

void handleWifiRemove(String args) {
    if (args.length() == 0) {
        writeOutput("Использование: wifiremove <SSID>\n");
        return;
    }

    String targetSSID = args;
    String tempContent = "";
    bool found = false;

    fs::File file = LittleFS.open("/config/wifi_list.conf", FILE_READ);
    if (file) {
        while (file.available()) {
            String line = file.readStringUntil('\n');
            line.trim();
            if (line.startsWith(targetSSID + ";")) {
                found = true;
                continue;
            }
            tempContent += line + "\n";
        }
        file.close();
    }

    if (!found) {
        writeOutput("Сеть не найдена!\n");
        return;
    }

    LittleFS.remove("/config/wifi_list.conf");
    writeToFile("/config/wifi_list.conf", tempContent);
    writeOutput("Сеть удалена\n");
}

WifiConfig readWifiConfig() {
    WifiConfig config;
    config.createMode = false;
    config.channel = 1;
    
    fs::File file = LittleFS.open("/config/wifi.conf", "r");
    if (file) {
        while (file.available()) {
            String line = file.readStringUntil('\n');
            line.trim();
            
            if (line.startsWith("CREATE_WIFI=")) {
                config.createMode = line.substring(12) == "true";
            }
            else if (line.startsWith("SSID=")) {
                config.ssid = line.substring(5);
            }
            else if (line.startsWith("PASSWORD=")) {
                config.password = line.substring(9);
            }
            else if (line.startsWith("CHANNEL=")) {
                config.channel = line.substring(8).toInt();
            }
        }
        file.close();
    }
    return config;
}

void writeWifiConfig(const WifiConfig& config) {
    String content;
    content += "CREATE_WIFI=" + String(config.createMode ? "true" : "false") + "\n";
    content += "SSID=" + config.ssid + "\n";
    content += "PASSWORD=" + config.password + "\n";
    if(config.createMode) {
        content += "CHANNEL=" + String(config.channel) + "\n";
    }
    
    writeToFile("/config/wifi.conf", content);
}

// Обработчики команд
void handleWifiMode(String args) {
    WifiConfig config = readWifiConfig();
    
    if(args == "create") {
        config.createMode = true;
        writeOutput("Режим установлен: Создание точки доступа\n");
    }
    else if(args == "connect") {
        config.createMode = false;
        writeOutput("Режим установлен: Подключение к сети\n");
    }
    else {
        writeOutput("Использование: wifimode <create|connect>\n");
        return;
    }
    
    writeWifiConfig(config);
}

void handleWifiCreate(String args) {
    WifiConfig config = readWifiConfig();
    if(!config.createMode) {
        writeOutput("Сначала переключитесь в режим создания: wifimode create\n");
        return;
    }

    int space1 = args.indexOf(' ');
    int space2 = args.lastIndexOf(' ');
    
    if(space1 == -1 || space1 == space2) {
        writeOutput("Использование: wificreate <SSID> <PASSWORD> [CHANNEL]\n");
        return;
    }
    
    config.ssid = args.substring(0, space1);
    config.password = args.substring(space1+1, space2);
    config.channel = (space2 != -1) ? args.substring(space2+1).toInt() : 6;
    
    writeWifiConfig(config);
    writeOutput("Точка доступа настроена!\n");
}

void handleWifiConnect(String args) {
    if (args.length() == 0) {
        writeOutput("Использование: wificonnect <SSID>\n");
        return;
    }

    String ssid = args;
    String password;

    // Ищем сеть в списке
    if (!findWifiInList(ssid, password)) {
        writeOutput("Сеть не найдена в списке!\n");
        return;
    }

    // Обновляем текущую конфигурацию
    WifiConfig config = readWifiConfig();
    config.ssid = ssid;
    config.password = password;
    writeWifiConfig(config);

    writeOutput("Подключение к сети: " + ssid + "\n");
    // Здесь можно добавить вызов функции для реального подключения к Wi-Fi
}

void handleWifiInfo() {
    WifiConfig config = readWifiConfig();
    String info = "Текущий режим: ";
    info += config.createMode ? "Создание точки доступа\n" : "Подключение к сети\n";
    info += "SSID: " + config.ssid + "\n";
    info += "Пароль: " + config.password + "\n";
    if(config.createMode) {
        info += "Канал: " + String(config.channel) + "\n";
    }
    writeOutput(info);
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
