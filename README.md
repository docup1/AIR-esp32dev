# AIR-esp32

## Описание проекта
AIR-esp32 — это проект, направленный на создание системы позволяющей просто и быстро разробатывать ПО для микроконтроллеров.

## Цели проекта
- Реализация консольного интерфейса, схожего с Bash для.
- Создание виртуальной машины для исполнения байткода.
- Обеспечение взаимодействия байткода с портами и интерфейсами микроконтроллера.
- Интеграция с файловой системой SPIFFS и SD-карт для хранения и загрузки данных.
- Подключение к Wi-Fi и взаимодействие с сервером.
- Создание API для удалённого управления.
- Разработка веб-интерфейса для настройки и управления системой.
- Тестирование стабильности и производительности системы.

## Ожидания от проекта
- Гибкость в использовании различных модулей и датчиков.
- Возможность интеграции с облачными сервисами.
- Энергоэффективность и устойчивость к ошибкам.
- Лёгкость в развертывании и настройке.

## 📂 Структура файловой системы

Файлы и каталоги в LittleFS:
- `/system` – системные файлы.
  - `/system/outputs` – логи системы.
    - `info.log` – лог информационных сообщений.
    - `error.log` – лог ошибок.
  - `systemdata.dat` – данные системы.
  - `board.conf` – конфигурация платы.
  - `settings.conf` – основные настройки.
  - `device_info.conf` – информация об устройстве.
- `/config` – файлы конфигурации.
  - `wifi.conf` – текущая Wi-Fi конфигурация.
  - `wifi_list.conf` – список доступных Wi-Fi сетей.
  - `port_init.conf` – настройки портов.
  - `interface_init.conf` – параметры интерфейса.
- `/utils` – утилиты.
  - `/utils/scripts` – скрипты.
  - `/utils/tools` – инструменты.
- `/home` – пользовательские файлы.

## 🛠 Доступные команды консоли

| Команда            | Описание |
|--------------------|----------|
| `help`            | Вывести список доступных команд. |
| `ls [path]`       | Вывести список файлов и папок. |
| `cat <file>`      | Показать содержимое файла. |
| `touch <file>`    | Создать пустой файл. |
| `echo <text> > file` | Записать данные в файл. |
| `rm <file>`       | Удалить файл. |
| `mkdir <dir>`     | Создать директорию. |
| `rmdir <dir>`     | Удалить директорию. |
| `cd <dir>`        | Сменить текущую директорию. |
| `pwd`             | Показать текущую директорию. |
| `tree [path]`     | Вывести дерево каталогов. |
| `info`            | Показать информацию о файловой системе. |
| `cp <src> <dst>`  | Копировать файл или директорию. |
| `mv <src> <dst>`  | Переместить файл или директорию. |
| `setenv <key> <value>` | Установить переменную окружения. |
| `getenv <key>`    | Получить значение переменной окружения. |
| `unsetenv <key>`  | Удалить переменную окружения. |
| `printenv`        | Вывести все переменные окружения. |
| `shutdown`        | Выключить устройство. |
| `reboot`          | Перезагрузить устройство. |
| `status`          | Показать состояние системы. |
| `skript <file>`   | Выполнить скрипт. |
| `run <file>`      | Запустить программу. |
| `infolog`         | Показать информационные логи. |
| `errlog`         | Показать ошибки. |
| `clear`          | Очистить все логи. |
| `clearinfolog`   | Очистить информационные логи. |
| `clearerrlog`    | Очистить логи ошибок. |
| `wifi <ssid> <pass>` | Добавить Wi-Fi сеть в список. |
| `wifilist`        | Показать сохранённые Wi-Fi сети. |
| `wifiremove <ssid>` | Удалить сохранённую Wi-Fi сеть. |
| `wificonnect <ssid>` | Подключиться к сохранённой Wi-Fi сети. |
| `wifimode <create/connect>` | Установить режим работы Wi-Fi. |
| `wificreate <ssid> <pass> [channel]` | Создать точку доступа. |
| `wifiinfo`        | Показать текущие настройки Wi-Fi. |
| `compile <file> <bytecode>` | Скомпилировать байт-код. |

