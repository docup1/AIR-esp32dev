#include <LittleFS.h>
#include <FS.h>
#include <EEPROM.h>

// Эти заголовочные файлы должны содержать объявления функций
// для работы с командами, переменными окружения, утилитами и глобальными переменными.
#include "commands.h"      // Объявление функции printHelp() и handleCommand()
#include "utils.h"         // Объявление функции normalizePath()
#include "environment.h"   // Функции работы с переменными окружения, например getEnvVar()
#include "globals.h"       // Например, currentDirectory, EEPROM_SIZE
#include "file_system.h"   // Функции работы с файловой системой
// Настройка скорости последовательного порта
#define BAUDRATE 115200

// =================== Основной скетч ===================
void setup() {
  Serial.begin(BAUDRATE);
  while (!Serial); // Ожидание открытия Serial

  initializeFS();
  printHelp();
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.length() > 0) {
      handleCommand(input);
    }
  }
}
