#include "vm.h"

// Конструктор: инициализирует ВМ и выполняет сброс
VirtualMachine::VirtualMachine() {
    reset();
}

// Сброс состояния ВМ (сброс регистров, PC, стека и восстановление памяти)
void VirtualMachine::reset() {
    pc = 0;
    sp = STACK_SIZE - 1;
    running = false;
    memset(reg, 0, sizeof(reg));
    memset(stack, 0, sizeof(stack));
    storage.init();
    storage.restore();
}

// Чтение 32-битного значения из памяти (big-endian)
uint32_t VirtualMachine::read32(uint32_t address) {
    if (address + 3 >= MEM_SIZE) {
        Serial.printf("read32: Address 0x%04X out of bounds\n", address);
        return 0;
    }
    return (storage.read(address) << 24) |
           (storage.read(address + 1) << 16) |
           (storage.read(address + 2) << 8) |
            storage.read(address + 3);
}

// Запись 32-битного значения в память (big-endian)
void VirtualMachine::write32(uint32_t address, uint32_t value) {
    if (address + 3 >= MEM_SIZE) {
        Serial.printf("write32: Address 0x%04X out of bounds\n", address);
        return;
    }
    storage.write(address,     (value >> 24) & 0xFF);
    storage.write(address + 1, (value >> 16) & 0xFF);
    storage.write(address + 2, (value >> 8) & 0xFF);
    storage.write(address + 3,  value & 0xFF);
}

// Загрузка программы в память ВМ
void VirtualMachine::loadProgram(const uint8_t* program, size_t size) {
    size = min(size, static_cast<size_t>(MEM_SIZE)); // Ограничение размера программы размером памяти
    for (uint32_t i = 0; i < size; i++) {
        storage.write(i, program[i]);
    }
}

// Основной цикл выполнения программы
void VirtualMachine::run() {
    running = true;
    while (running && pc < MEM_SIZE) {
        uint8_t opcode = storage.read(pc++);
        switch (opcode) {

            // Загрузка константы в регистр: LOAD reg, <32-bit value>
            case OP_LOAD: {
                uint8_t reg_num = storage.read(pc++);
                uint32_t value = read32(pc);
                if (reg_num < NUM_REGS) {
                    reg[reg_num] = value;
                } else {
                    Serial.printf("LOAD: Invalid register number: %d\n", reg_num);
                }
                pc += 4;
                break;
            }

            // Запись значения из регистра в память: STORE reg, <32-bit address>
            case OP_STORE: {
                uint8_t reg_num = storage.read(pc++);
                uint32_t address = read32(pc);
                if (reg_num < NUM_REGS) {
                    write32(address, reg[reg_num]);
                } else {
                    Serial.printf("STORE: Invalid register number: %d\n", reg_num);
                }
                pc += 4;
                break;
            }

            // Сложение: ADD dst, src1, src2  => dst = src1 + src2
            case OP_ADD: {
                uint8_t dst  = storage.read(pc++);
                uint8_t src1 = storage.read(pc++);
                uint8_t src2 = storage.read(pc++);
                if (dst < NUM_REGS && src1 < NUM_REGS && src2 < NUM_REGS) {
                    reg[dst] = reg[src1] + reg[src2];
                } else {
                    Serial.println("ADD: Invalid register number");
                }
                break;
            }

            // Вычитание: SUB dst, src1, src2  => dst = src1 - src2
            case OP_SUB: {
                uint8_t dst  = storage.read(pc++);
                uint8_t src1 = storage.read(pc++);
                uint8_t src2 = storage.read(pc++);
                if (dst < NUM_REGS && src1 < NUM_REGS && src2 < NUM_REGS) {
                    reg[dst] = reg[src1] - reg[src2];
                } else {
                    Serial.println("SUB: Invalid register number");
                }
                break;
            }

            // Умножение: MUL dst, src1, src2  => dst = src1 * src2
            case OP_MUL: {
                uint8_t dst  = storage.read(pc++);
                uint8_t src1 = storage.read(pc++);
                uint8_t src2 = storage.read(pc++);
                if (dst < NUM_REGS && src1 < NUM_REGS && src2 < NUM_REGS) {
                    reg[dst] = reg[src1] * reg[src2];
                } else {
                    Serial.println("MUL: Invalid register number");
                }
                break;
            }

            // Деление: DIV dst, src1, src2  => dst = src1 / src2 (проверка деления на 0)
            case OP_DIV: {
                uint8_t dst  = storage.read(pc++);
                uint8_t src1 = storage.read(pc++);
                uint8_t src2 = storage.read(pc++);
                if (dst < NUM_REGS && src1 < NUM_REGS && src2 < NUM_REGS) {
                    if (reg[src2] != 0) {
                        reg[dst] = reg[src1] / reg[src2];
                    } else {
                        Serial.println("DIV: Division by zero");
                        reg[dst] = 0;
                    }
                } else {
                    Serial.println("DIV: Invalid register number");
                }
                break;
            }

            // Операция PUSH: PUSH reg  => Помещает значение регистра в стек
            case OP_PUSH: {
                uint8_t reg_num = storage.read(pc++);
                if (reg_num < NUM_REGS) {
                    if (!push(reg[reg_num])) {
                        Serial.println("PUSH: Stack overflow");
                    }
                } else {
                    Serial.println("PUSH: Invalid register number");
                }
                break;
            }

            // Операция POP: POP reg  => Извлекает значение из стека в регистр
            case OP_POP: {
                uint8_t reg_num = storage.read(pc++);
                uint32_t value;
                if (pop(value)) {
                    if (reg_num < NUM_REGS) {
                        reg[reg_num] = value;
                    } else {
                        Serial.println("POP: Invalid register number");
                    }
                } else {
                    Serial.println("POP: Stack underflow");
                }
                break;
            }

            // Системные вызовы
            case OP_SYSCALL: {
                uint8_t call_code = storage.read(pc++);
                handleSystemCall(call_code);
                break;
            }

            // Остановка выполнения программы
            case OP_HALT: {
                running = false;
                break;
            }

            default: {
                Serial.printf("Unknown opcode: 0x%02X at address 0x%04X\n", opcode, pc - 1);
                running = false;
                break;
            }
        }

        // Если pc выходит за пределы памяти, останавливаем выполнение
        if (pc >= MEM_SIZE) {
            Serial.println("PC reached end of memory. Halting.");
            running = false;
        }
    }
}

// Сохранение состояния памяти на файловую систему
void VirtualMachine::persistState() {
    storage.persist();
}

// Вывод текущего состояния ВМ (PC, регистры)
void VirtualMachine::printState() {
    Serial.println("\nVM State:");
    Serial.printf("PC: 0x%04X\n", pc);
    for (int i = 0; i < NUM_REGS; i++) {
        Serial.printf("R%d: 0x%08X\n", i, reg[i]);
    }
    Serial.println("------------------");
}

// Обработчик системных вызовов (расширяемый)
void VirtualMachine::handleSystemCall(uint8_t code) {
    switch (code) {
        // Печать строки, расположенной в памяти по адресу, хранящемуся в reg[0]
        case 0x01: { // PRINT_STRING
            uint32_t addr = reg[0];
            while (addr < MEM_SIZE) {
                char c = storage.read(addr++);
                if (c == 0) break;
                Serial.print(c);
            }
            break;
        }
        // Загрузка данных: копирование length байт из области программы (data_addr) в память по адресу dest_addr
        case 0x02: { // LOAD_DATA
            uint32_t dest_addr = reg[0]; // Адрес в памяти, куда копировать данные
            uint32_t data_addr = reg[1]; // Адрес данных в памяти
            uint32_t length    = reg[2]; // Длина данных
            for (uint32_t i = 0; i < length; i++) {
                if ((dest_addr + i) >= MEM_SIZE || (data_addr + i) >= MEM_SIZE) {
                    Serial.println("LOAD_DATA: Memory access violation");
                    break;
                }
                storage.write(dest_addr + i, storage.read(data_addr + i));
            }
            break;
        }
        default:
            Serial.printf("Unknown system call: 0x%02X\n", code);
            break;
    }
}

// Вспомогательная функция: помещение значения в стек
bool VirtualMachine::push(uint32_t value) {
    if (sp == 0) {
        // Стек переполнен
        return false;
    }
    stack[sp--] = value;
    return true;
}

// Вспомогательная функция: извлечение значения из стека
bool VirtualMachine::pop(uint32_t &value) {
    if (sp >= STACK_SIZE - 1) {
        // Стек пуст
        return false;
    }
    value = stack[++sp];
    return true;
}
