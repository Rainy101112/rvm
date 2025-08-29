#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

enum instructions {
    OP_HALT = 0,    // Halt             HLT
    OP_LOAD,        // Load value       LD      [REG] [NUM]
    OP_MOV,         // Move value       MOV     [REG] [REG]

    OP_ADD,         // Addition         ADD     [DEST] [REG] [REG]
    OP_SUB,         // Subtract         SUB     [DEST] [REG] [REG]
    OP_MULTI,       // Multiplying      MUL     [DEST] [REG] [REG]
    OP_DIVIDE,      // Divide           DIV     [DEST] [REG] [REG]

    OP_INCREASE,    // Reg++            INC     [REG]
    OP_DECREASE,    // Reg--            DEC     [REG]

    OP_AND,         // AND              AND     [DEST] [REG] [REG]
    OP_NOT,         // NOT              NOT     [REG]
    OP_OR,          // OR               OR      [DEST] [REG] [REG]
    OP_XOR,         // XOR              XOR     [DEST] [REG] [REG]
    OP_CMP,         // Compare          CMP     [DEST] [REG] [REG]

    OP_JUMP,        // Jump             JMP     [ADDR]
    OP_JNZ,         // Jump if not zero JNZ     [REG] [ADDR]
    OP_JZ,          // Jump if zero     JZ      [REG] [ADDR]
    OP_LOOP,        // Loop             LOOP    [REG] [ADDR]

    OP_PRINT,       // Print register   PRT
};

// 寄存器数量
#define NUM_REGISTERS 4

// 指令结构
typedef struct {
    char* mnemonic;
    int opcode;
    int num_operands;
} instruction_info;

// 指令表
instruction_info instruction_table[] = {
    {"HLT",     OP_HALT,        0},
    {"LD",      OP_LOAD,        2},
    {"MOV",     OP_MOV,         2},

    {"ADD",     OP_ADD,         3},
    {"SUB",     OP_SUB,         3},
    {"MUL",     OP_MULTI,       3},
    {"DIV",     OP_DIVIDE,      3},

    {"INC",     OP_INCREASE,    1},
    {"DEC",     OP_DECREASE,    1},

    {"AND",     OP_AND,         3},
    {"NOT",     OP_NOT,         1},
    {"OR",      OP_OR,          3},
    {"XOR",     OP_XOR,         3},
    {"CMP",     OP_CMP,         3},

    {"JMP",     OP_JUMP,        1},
    {"JNZ",     OP_JNZ,         2},
    {"JZ",      OP_JZ,          2},
    {"LOOP",    OP_LOOP,        2},

    {"PRT",     OP_PRINT,       1},

    {NULL, 0, 0}  // 结束标记
};

// 将字符串转换为大写
void to_upper(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = toupper(str[i]);
    }
}

// 解析寄存器名称
int parse_register(char* reg) {
    if (reg[0] == 'R' && isdigit(reg[1]) && reg[2] == '\0') {
        int reg_num = reg[1] - '0';
        if (reg_num >= 0 && reg_num < NUM_REGISTERS) {
            return reg_num;
        }
    }
    return -1; // 无效寄存器
}

// 解析数字（十进制或十六进制）
int parse_number(char* num_str) {
    // 检查是否为十六进制
    if (num_str[0] == '0' && (num_str[1] == 'x' || num_str[1] == 'X')) {
        return (int)strtol(num_str, NULL, 16);
    }
    return atoi(num_str);
}

// 汇编器主函数
int assemble(char* input_filename, char* output_filename) {
    FILE* input_file = fopen(input_filename, "r");
    FILE* output_file = fopen(output_filename, "wb");
    
    if (!input_file || !output_file) {
        printf("Could not open file\n");
        return 1;
    }
    
    char line[256];
    int line_num = 0;
    
    while (fgets(line, sizeof(line), input_file)) {
        line_num++;
        
        // 移除行尾的换行符
        line[strcspn(line, "\n")] = 0;
        
        // 跳过空行和注释
        if (line[0] == ';' || line[0] == '#' || line[0] == '\0') {
            continue;
        }
        
        // 将行转换为大写以便处理
        to_upper(line);
        
        char opcode_str[32];
        char operands[3][32];

        // 解析指令和操作数
        int tokens = sscanf(line, "%31s %31s %31s %31s", 
                           opcode_str, operands[0], operands[1], operands[2]);
        
        if (tokens < 1) {
            printf("Line %d: Invalid instruction\n", line_num);
            continue;
        }
        
        // 查找指令
        instruction_info* instr = NULL;
        for (int i = 0; instruction_table[i].mnemonic != NULL; i++) {
            if (strcmp(opcode_str, instruction_table[i].mnemonic) == 0) {
                instr = &instruction_table[i];
                break;
            }
        }
        
        if (!instr) {
            printf("Line %d: Unknown instruction '%s'\n", line_num, opcode_str);
            continue;
        }
        
        // 检查操作数数量
        if (tokens - 1 != instr->num_operands) {
            printf("Line %d: Instruction '%s' needs %d operands, got %d\n", 
                  line_num, instr->mnemonic, instr->num_operands, tokens - 1);
            continue;
        }
        
        // 写入操作码
        fputc(instr->opcode, output_file);
        
        // 处理操作数
        for (int i = 0; i < instr->num_operands; i++) {
            if (strncmp(operands[i], "R", 1) == 0) {
                // 寄存器操作数
                int reg = parse_register(operands[i]);
                if (reg == -1) {
                    printf("Line %d: Invalid register '%s'\n", line_num, operands[i]);
                    fclose(input_file);
                    fclose(output_file);
                    return 1;
                }
                fputc(reg, output_file);
            } else {
                // 立即数操作数
                int num = parse_number(operands[i]);
                fputc(num, output_file);
            }
        }
    }
    
    fclose(input_file);
    fclose(output_file);
    return 0;
}

// 反汇编器（用于验证）
void disassemble(char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Could not open file\n");
        return;
    }
    
    int opcode;
    while ((opcode = fgetc(file)) != EOF) {
        // 查找指令
        instruction_info* instr = NULL;
        for (int i = 0; instruction_table[i].mnemonic != NULL; i++) {
            if (opcode == instruction_table[i].opcode) {
                instr = &instruction_table[i];
                break;
            }
        }
        
        if (!instr) {
            printf("Unknown opcode: %02X\n", opcode);
            break;
        }
        
        printf("%s", instr->mnemonic);
        
        // 读取并显示操作数
        for (int i = 0; i < instr->num_operands; i++) {
            int operand = fgetc(file);
            if (operand == EOF) {
                printf(" Unexpected EOF\n");
                fclose(file);
                return;
            }
            
            // 根据指令类型格式化输出
            switch (instr->opcode) {
                case OP_LOAD:
                    if (i == 0) printf(" R%d", operand);
                    else printf(" %d", operand); // 使用十进制显示更友好
                    break;
                    
                case OP_MOV:
                    printf(" R%d", operand);
                    break;
                    
                case OP_ADD:
                case OP_SUB:
                case OP_MULTI:
                case OP_DIVIDE:
                case OP_AND:
                case OP_OR:
                case OP_XOR:
                    printf(" R%d", operand);
                    break;
                    
                case OP_INCREASE:
                case OP_DECREASE:
                case OP_NOT:
                case OP_JUMP:
                case OP_PRINT:
                    printf(" R%d", operand);
                    break;
                    
                default:
                    printf(" %02X", operand); // 默认显示十六进制
                    break;
            }
        }
        printf("\n");
    }
    
    fclose(file);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <INPUT> <OUTPUT>\n", argv[0]);
        printf("Example: %s program.asm program.bin\n", argv[0]);
        return 1;
    }
    
    if (assemble(argv[1], argv[2]) == 0) {
        printf("Assembled successfully!\n");
        printf("Generated bytecode:\n");
        disassemble(argv[2]);
    } else {
        printf("Error during assembly\n");
    }
    
    return 0;
}
