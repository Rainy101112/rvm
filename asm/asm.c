#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

enum instructions {
    OP_HALT = 0,    // Halt                         HLT

    OP_LOAD,        // Load value                   LD      [REG] [NUM]
    OP_LA,          // Load value from address      LA      [REG] [ADDR]
    OP_SA,          // Store value to address       SA      [REG] [ADDR]
    OP_MOV,         // Move value                   MOV     [REG] [REG]

    OP_ADD,         // Addition                     ADD     [DEST] [REG] [REG]
    OP_SUB,         // Subtract                     SUB     [DEST] [REG] [REG]
    OP_MULTI,       // Multiplying                  MUL     [DEST] [REG] [REG]
    OP_DIVIDE,      // Divide                       DIV     [DEST] [REG] [REG]

    OP_INCREASE,    // Reg++                        INC     [REG]
    OP_DECREASE,    // Reg--                        DEC     [REG]

    OP_AND,         // AND                          AND     [DEST] [REG] [REG]
    OP_NOT,         // NOT                          NOT     [REG]
    OP_OR,          // OR                           OR      [DEST] [REG] [REG]
    OP_XOR,         // XOR                          XOR     [DEST] [REG] [REG]
    OP_CMP,         // Compare                      CMP     [DEST] [REG] [REG]

    OP_JUMP,        // Jump                         JMP     [ADDRREG]
    OP_JNZ,         // Jump if not zero             JNZ     [REG] [ADDRREG]
    OP_JZ,          // Jump if zero                 JZ      [REG] [ADDRREG]
    OP_LOOP,        // Loop                         LOOP    [REG] [ADDRREG]

    OP_PRINT,       // Print register               PRT     [REG]
};

/* Register amount */
#define NUM_REGISTERS 4

/* Structure of instruction */
typedef struct {
    char* mnemonic;
    int opcode;
    int num_operands;
} instruction_info;

/* Instruction table */
instruction_info instruction_table[] = {
    {"HLT",     OP_HALT,        0},
    {"LD",      OP_LOAD,        2},
    {"LA",      OP_LA,          2},
    {"SA",      OP_SA,          2},
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

    {NULL, 0, 0}  // End
};

/* Switch all characters to uppercase */
void to_upper(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = toupper(str[i]);
    }
}

/* Get register name */
int parse_register(char* reg) {
    if (reg[0] == 'R' && isdigit(reg[1]) && reg[2] == '\0') {
        int reg_num = reg[1] - '0';
        if (reg_num >= 0 && reg_num < NUM_REGISTERS) {
            return reg_num;
        }
    }
    return -1;  // Invaild register
}

/* Get number */
int parse_number(char* num_str) {
    /* Check if HEX */
    if (num_str[0] == '0' && (num_str[1] == 'x' || num_str[1] == 'X')) {
        return (int)strtol(num_str, NULL, 16);
    }
    return atoi(num_str);
}

/* Assembly */
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
        
        /* Remove newline */
        line[strcspn(line, "\n")] = 0;
        
        /* Skip empty lines and comments */
        if (line[0] == ';' || line[0] == '#' || line[0] == '\0') {
            continue;
        }
        
        /* Switch the characters to uppercase */
        to_upper(line);
        
        char opcode_str[32];
        char operands[3][32];

        /* Get instruction */
        int tokens = sscanf(line, "%31s %31s %31s %31s", 
                           opcode_str, operands[0], operands[1], operands[2]);
        
        if (tokens < 1) {
            printf("Line %d: Invalid instruction\n", line_num);
            continue;
        }
        
        /* Find instruction */
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
        
        /* Check operand amount */
        if (tokens - 1 != instr->num_operands) {
            printf("Line %d: Instruction '%s' needs %d operands, got %d\n", 
                  line_num, instr->mnemonic, instr->num_operands, tokens - 1);
            continue;
        }
        
        /* Write opcode */
        fputc(instr->opcode, output_file);
        
        /* Process operand */
        for (int i = 0; i < instr->num_operands; i++) {
            if (strncmp(operands[i], "R", 1) == 0) {
                /* Register operand */
                int reg = parse_register(operands[i]);
                if (reg == -1) {
                    printf("Line %d: Invalid register '%s'\n", line_num, operands[i]);
                    fclose(input_file);
                    fclose(output_file);
                    return 1;
                }
                fputc(reg, output_file);
            } else {
                /* Immediate operand */
                int num = parse_number(operands[i]);
                fputc(num, output_file);
            }
        }
    }
    
    fclose(input_file);
    fclose(output_file);
    return 0;
}

/* Disassembly for verification */
void disassemble(char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Could not open file\n");
        return;
    }
    
    int opcode;
    while ((opcode = fgetc(file)) != EOF) {
        /* Find instruction */
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
        
        /* Read and print operand */
        for (int i = 0; i < instr->num_operands; i++) {
            int operand = fgetc(file);
            if (operand == EOF) {
                printf(" Unexpected EOF\n");
                fclose(file);
                return;
            }
            
            /* Format output by instruction type */
            switch (instr->opcode) {
                case OP_LOAD: {
                    if (i == 0) printf(" R%d", operand);
                    else printf(" %d", operand);
                    break;
                }

                case OP_LA: {
                    if (i == 0) printf(" R%d", operand);
                    else printf(" %#x", operand);
                    break;
                }

                case OP_SA: {
                    if (i == 0) printf(" R%d", operand);
                    else printf(" %#x", operand);
                    break;
                }

                case OP_MOV: {
                    printf(" R%d", operand);
                    break;
                }
                
                case OP_ADD: {
                    if (i == 0) printf(" R%d", operand);
                    else printf(" %d", operand);
                    break;
                }

                case OP_SUB: {
                    if (i == 0) printf(" R%d", operand);
                    else printf(" %d", operand);
                    break;
                }

                case OP_MULTI :{
                    if (i == 0) printf(" R%d", operand);
                    else printf(" %d", operand);
                    break;
                }

                case OP_DIVIDE: {
                    if (i == 0) printf(" R%d", operand);
                    else printf(" %d", operand);
                    break;
                }

                case OP_AND: {
                    if (i == 0) printf(" R%d", operand);
                    else printf(" %d", operand);
                    break;
                }

                case OP_NOT: {
                    printf(" R%d", operand);
                    break;
                }

                case OP_OR: {
                    if (i == 0) printf(" R%d", operand);
                    else printf(" %d", operand);
                    break;
                }

                case OP_XOR: {
                    if (i == 0) printf(" R%d", operand);
                    else printf(" %d", operand);
                    break;
                }

                case OP_CMP: {
                    if (i == 0) printf(" R%d", operand);
                    else printf(" %d", operand);
                    break;
                }

                case OP_INCREASE: {
                    printf(" R%d", operand);
                    break;
                }

                case OP_DECREASE: {
                    printf(" R%d", operand);
                    break;
                }

                case OP_JUMP: {
                    printf(" R%d", operand);
                    break;
                }

                case OP_JNZ: {
                    printf(" R%d", operand);
                    break;
                }

                case OP_JZ: {
                    printf(" R%d", operand);
                    break;
                }

                case OP_LOOP: {
                    printf(" R%d", operand);
                    break;
                }

                case OP_PRINT: {
                    printf(" R%d", operand);
                    break;
                }

                default: {
                    printf(" %#x", operand);
                    break;
                }
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
