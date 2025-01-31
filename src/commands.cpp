#include "commands.h"
#include "file_system.h"
#include "utils.h"

extern String currentDirectory;
extern bool outputRedirected;
extern File outputFile;

void handleCommand(String input) {
    int redirectPos = input.indexOf(">");
    String outputFilename;
    int mode = 0; // 0 - нет, 1 - перезапись, 2 - добавление

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
    
    if (cmd.name == "ls") {
        listFiles(cmd.args);
    }
    else if (cmd.name == "cat") {
        if (checkArgs(cmd.args, 1)) catFile(cmd.args);
    }
    else if (cmd.name == "touch") {
        if (checkArgs(cmd.args, 1)) createFile(cmd.args);
    }
    else if (cmd.name == "echo") {
        handleEcho(cmd.args);
    }
    else if (cmd.name == "rm") {
        if (checkArgs(cmd.args, 1)) deleteFile(cmd.args);
    }
    else if (cmd.name == "mkdir") {
        if (checkArgs(cmd.args, 1)) createDir(cmd.args);
    }
    else if (cmd.name == "rmdir") {
        if (checkArgs(cmd.args, 1)) deleteDir(cmd.args);
    }
    else if (cmd.name == "cd") {
        if (checkArgs(cmd.args, 1)) changeDir(cmd.args);
    }
    else if (cmd.name == "pwd") {
        printWorkingDir();
    }
    else if (cmd.name == "tree") {
        printTree(cmd.args);
    }
    else if (cmd.name == "info") {
        printFSInfo();
    }
    else if (cmd.name == "cp") {
        if (checkArgs(cmd.args, 2)) copyFile(cmd.args);
    }
    else if (cmd.name == "mv") {
        if (checkArgs(cmd.args, 2)) moveFile(cmd.args);
    }
    else if (cmd.name == "help") {
        printHelp();
    }
    else {
        Serial.println("Неизвестная команда");
    }

    if (outputRedirected) {
        outputFile.close();
        outputRedirected = false;
    }
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

void printHelp() {
    String helpText = "Доступные команды:\n";
    helpText += "ls [path] - список файлов и папок\n";
    helpText += "cat <file> - вывести содержимое файла\n";
    helpText += "touch <file> - создать пустой файл\n";
    helpText += "echo <text> > <file> - записать текст в файл\n";
    helpText += "rm <file> - удалить файл\n";
    helpText += "mkdir <dir> - создать директорию\n";
    helpText += "rmdir <dir> - удалить директорию\n";
    helpText += "cd <dir> - сменить директорию\n";
    helpText += "pwd - показать текущую директорию\n";
    helpText += "tree [path] - вывести дерево каталогов\n";
    helpText += "info - информация о файловой системе\n";
    helpText += "cp <src> <dst> - скопировать файл\n";
    helpText += "mv <src> <dst> - переместить файл\n";
    helpText += "help - показать это сообщение\n";
    writeOutput(helpText);
}
