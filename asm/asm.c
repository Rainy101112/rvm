/* 
 *
 *      asm.c
 * 
 *      By Rainy101112 2025/8/28
 *      Public under MIT license
 * 
 *      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *      IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *      FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <errno.h>
#include <locale.h>

enum instructions {
    OP_HALT = 0,    // Halt                         HLT

    OP_LOAD,        // Load value                   LD      [REG] [NUM]
    OP_LA,          // Load value from address      LA      [REG] [ADDR]
    OP_SA,          // Store value to address       SA      [REG] [ADDR]
    OP_MOV,         // Move value                   MOV     [DEST] [SRC]

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

    OP_PUSH,        // Push stack                   PUSH    [REG]
    OP_POP,         // Pop stack                    POP     [REG]
    OP_CALL,        // Call address                 CALL    [ADDRREG]
    OP_RET,         // Return                       RET

    OP_TRAP,        // Trap                         TRAP    [REG] [NUMREG]

    OP_PRINT,       // Print register               PRT     [REG]

    OP_FADD,        // Float add                    FADD    [DEST] [REG] [REG]
    OP_FSUB,        // Float subtract               FSUB    [DEST] [REG] [REG]
    OP_FMUL,        // Float multiply               FMUL    [DEST] [REG] [REG]
    OP_FDIV,        // Float divide                 FDIV    [DEST] [REG] [REG]
    OP_FCMP,        // Float equal                  FCMP    [DEST] [REG] [REG]
    OP_FLT,         // Float less than              FLT     [DEST] [REG] [REG]
    OP_FLE,         // Float less or equal          FLE     [DEST] [REG] [REG]
    OP_ITOF,        // Integer to float             ITOF    [DEST] [REG]
    OP_FTOI,        // Float to integer             FTOI    [DEST] [REG]
    OP_FLD,         // Load float immediate         FLD     [REG] [NUM]
    OP_FPRT,        // Print float register         FPRT    [REG]
};

/* Register amount */
#define NUM_REGISTERS 8

/* Structure of instruction */
typedef struct {
    const char* mnemonic;
    int opcode;
    int num_operands;
    const char* operand_types;  // 'R' = register, 'I' = immediate
} instruction_info;

/* Instruction table */
instruction_info instruction_table[] = {
    {"HLT",     OP_HALT,        0, ""},
    {"LD",      OP_LOAD,        2, "RI"},
    {"LA",      OP_LA,          2, "RI"},
    {"SA",      OP_SA,          2, "RI"},
    {"MOV",     OP_MOV,         2, "RR"},

    {"ADD",     OP_ADD,         3, "RRR"},
    {"SUB",     OP_SUB,         3, "RRR"},
    {"MUL",     OP_MULTI,       3, "RRR"},
    {"DIV",     OP_DIVIDE,      3, "RRR"},

    {"INC",     OP_INCREASE,    1, "R"},
    {"DEC",     OP_DECREASE,    1, "R"},

    {"AND",     OP_AND,         3, "RRR"},
    {"NOT",     OP_NOT,         1, "R"},
    {"OR",      OP_OR,          3, "RRR"},
    {"XOR",     OP_XOR,         3, "RRR"},
    {"CMP",     OP_CMP,         3, "RRR"},

    {"JMP",     OP_JUMP,        1, "R"},
    {"JNZ",     OP_JNZ,         2, "RR"},
    {"JZ",      OP_JZ,          2, "RR"},
    {"LOOP",    OP_LOOP,        2, "RR"},

    {"PUSH",    OP_PUSH,        1, "R"},
    {"POP",     OP_POP,         1, "R"},
    {"CALL",    OP_CALL,        1, "R"},
    {"RET",     OP_RET,         0, ""},

    {"TRAP",    OP_TRAP,        2, "RR"},

    {"PRT",     OP_PRINT,       1, "R"},

    {"FADD",    OP_FADD,        3, "RRR"},
    {"FSUB",    OP_FSUB,        3, "RRR"},
    {"FMUL",    OP_FMUL,        3, "RRR"},
    {"FDIV",    OP_FDIV,        3, "RRR"},
    {"FCMP",    OP_FCMP,        3, "RRR"},
    {"FLT",     OP_FLT,         3, "RRR"},
    {"FLE",     OP_FLE,         3, "RRR"},
    {"ITOF",    OP_ITOF,        2, "RR"},
    {"FTOI",    OP_FTOI,        2, "RR"},
    {"FLD",     OP_FLD,         2, "RF"},
    {"FPRT",    OP_FPRT,        1, "R"},

    {NULL, 0, 0, NULL}  // End
};

/* Switch all characters to uppercase */
void to_upper(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = (char)toupper((unsigned char)str[i]);
    }
}

/* Get register name */
int parse_register(char* reg) {
    if (reg[0] == 'R' && isdigit((unsigned char)reg[1]) && reg[2] == '\0') {
        int reg_num = reg[1] - '0';
        if (reg_num >= 0 && reg_num < NUM_REGISTERS) {
            return reg_num;
        }
    }
    return -1;  // Invaild register
}

/* Get number. Returns -1 on invalid input, otherwise stores the
 * parsed value in *out and returns 0. */
int parse_number(const char* num_str, uint64_t* out) {
    char* endptr = NULL;
    errno = 0;

    /* Check if HEX */
    int base = (num_str[0] == '0' && (num_str[1] == 'x' || num_str[1] == 'X')) ? 16 : 10;

    unsigned long long num = strtoull(num_str, &endptr, base);
    if (endptr == num_str || *endptr != '\0' || errno == ERANGE) {
        return -1;
    }

    *out = (uint64_t)num;
    return 0;
}

/* Get double. Returns -1 on invalid input, otherwise stores the raw 64-bit
 * IEEE 754 bit pattern (little-endian, same convention as LD immediates)
 * in *out and returns 0. Requires LC_NUMERIC to be "C" (set in main). */
int parse_float(const char* num_str, uint64_t* out) {
    char* endptr = NULL;
    errno = 0;

    double num = strtod(num_str, &endptr);
    if (endptr == num_str || *endptr != '\0' || errno == ERANGE) {
        return -1;
    }

    memcpy(out, &num, sizeof(*out));
    return 0;
}

/* Close files and drop the partially written output on failure */
static int assemble_fail(FILE* input_file, FILE* output_file,
                         const char* output_filename) {
    fclose(input_file);
    fclose(output_file);
    remove(output_filename);
    return 1;
}

/* Label table (two-pass assembly: pass 1 collects offsets) */
#define MAX_LABELS 256

typedef struct {
    char name[32];
    size_t offset;
} label_entry;

static label_entry label_table[MAX_LABELS];
static int label_count = 0;

static int label_lookup(const char* name) {
    for (int i = 0; i < label_count; i++) {
        if (strcmp(label_table[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

static int label_add(const char* name, size_t offset) {
    if (label_lookup(name) >= 0) {
        printf("Duplicate label '%s'\n", name);
        return -1;
    }
    if (label_count >= MAX_LABELS) {
        printf("Too many labels (max %d)\n", MAX_LABELS);
        return -1;
    }

    size_t name_len = strlen(name);
    if (name_len >= sizeof(label_table[label_count].name)) {
        name_len = sizeof(label_table[label_count].name) - 1;
    }
    memcpy(label_table[label_count].name, name, name_len);
    label_table[label_count].name[name_len] = '\0';
    label_table[label_count].offset = offset;
    label_count++;
    return 0;
}

static instruction_info* find_instruction(const char* mnemonic) {
    for (int i = 0; instruction_table[i].mnemonic != NULL; i++) {
        if (strcmp(mnemonic, instruction_table[i].mnemonic) == 0) {
            return &instruction_table[i];
        }
    }
    return NULL;
}

/* Byte size of an instruction's encoding */
static size_t instruction_size(const instruction_info* instr) {
    /* LD/LA/SA/FLD encode their second operand as an 8-byte immediate */
    if (instr->opcode == OP_LOAD || instr->opcode == OP_LA ||
        instr->opcode == OP_SA || instr->opcode == OP_FLD) {
        return 1 + 1 + 8;
    }
    return (size_t)(1 + instr->num_operands);
}

/* A label definition is "NAME:" with nothing but whitespace after the
 * colon. Stores the name and returns true if the line is one.
 * Names are [A-Z_][A-Z0-9_]* so they never look like numbers or registers. */
static bool parse_label(char* line, char name[32]) {
    char* colon = strchr(line, ':');
    if (colon == NULL) {
        return false;
    }

    /* Nothing but whitespace may follow the colon */
    for (const char* p = colon + 1; *p; p++) {
        if (!isspace((unsigned char)*p)) {
            return false;
        }
    }

    /* Extract the name (leading/trailing whitespace ignored) */
    char* start = line;
    while (isspace((unsigned char)*start)) {
        start++;
    }
    size_t len = (size_t)(colon - start);
    while (len > 0 && isspace((unsigned char)start[len - 1])) {
        len--;
    }
    if (len == 0 || len >= 32) {
        return false;
    }

    if (!(isalpha((unsigned char)start[0]) || start[0] == '_')) {
        return false;
    }
    for (size_t i = 1; i < len; i++) {
        if (!(isalnum((unsigned char)start[i]) || start[i] == '_')) {
            return false;
        }
    }

    memcpy(name, start, len);
    name[len] = '\0';
    return true;
}

/* Read the next line, skipping overlong ones. Returns false at EOF. */
static bool next_line(FILE* input_file, char* line, size_t line_size, int* line_num) {
    while (fgets(line, (int)line_size, input_file)) {
        (*line_num)++;

        /* Detect overlong lines: fgets reads at most line_size-1 chars.
         * A full buffer without a trailing newline means the line was split
         * and the remainder would be misassembled as a new instruction. */
        size_t line_len = strlen(line);
        if (line_len == line_size - 1 && line[line_len - 1] != '\n') {
            int next = fgetc(input_file);
            if (next != EOF && next != '\n') {
                printf("Line %d: line too long (max %zu characters), skipping\n",
                       *line_num, line_size - 1);
                while (next != '\n' && next != EOF) {
                    next = fgetc(input_file);
                }
                continue;
            }
            /* Exact fit: the line ends here, the peeked byte was consumed */
        }

        return true;
    }
    return false;
}

/* Assembly */
int assemble(char* input_filename, char* output_filename) {
    FILE* input_file = fopen(input_filename, "r");
    if (!input_file) {
        printf("Could not open input file: %s\n", input_filename);
        return 1;
    }

    FILE* output_file = fopen(output_filename, "wb");
    if (!output_file) {
        printf("Could not open output file: %s\n", output_filename);
        fclose(input_file);
        return 1;
    }

    char line[256];
    char opcode_str[32];
    char operands[3][32];
    int line_num = 0;

    /* First pass: collect label definitions and their byte offsets.
     * The offset only advances for lines that will emit code. */
    label_count = 0;
    size_t offset = 0;
    while (next_line(input_file, line, sizeof(line), &line_num)) {
        /* Remove newline (and possible \r from CRLF files) */
        line[strcspn(line, "\r\n")] = 0;

        /* Strip trailing comments (; or #) */
        char* comment = strpbrk(line, ";#");
        if (comment) {
            *comment = '\0';
        }

        /* Switch the characters to uppercase */
        to_upper(line);

        char label[32];
        if (parse_label(line, label)) {
            if (label_add(label, offset) != 0) {
                return assemble_fail(input_file, output_file, output_filename);
            }
            continue;
        }

        /* Skip empty lines and comments */
        int only_space = 1;
        for (char* p = line; *p; p++) {
            if (!isspace((unsigned char)*p)) {
                only_space = 0;
                break;
            }
        }
        if (line[0] == '\0' || only_space) {
            continue;
        }

        /* Get instruction */
        int tokens = sscanf(line, "%31s %31s %31s %31s",
                           opcode_str, operands[0], operands[1], operands[2]);
        if (tokens < 1) {
            continue;
        }

        instruction_info* instr = find_instruction(opcode_str);
        if (!instr || tokens - 1 != instr->num_operands) {
            /* Same skip behavior as pass 2: nothing is emitted */
            continue;
        }

        offset += instruction_size(instr);
    }
    rewind(input_file);
    line_num = 0;

    /* Second pass: emit code */
    while (next_line(input_file, line, sizeof(line), &line_num)) {
        /* Remove newline (and possible \r from CRLF files) */
        line[strcspn(line, "\r\n")] = 0;

        /* Strip trailing comments (; or #) */
        char* comment = strpbrk(line, ";#");
        if (comment) {
            *comment = '\0';
        }

        /* Switch the characters to uppercase */
        to_upper(line);

        char label[32];
        if (parse_label(line, label)) {
            continue;   // Offsets were recorded in pass 1
        }

        /* Skip empty lines and comments */
        int only_space = 1;
        for (char* p = line; *p; p++) {
            if (!isspace((unsigned char)*p)) {
                only_space = 0;
                break;
            }
        }
        if (line[0] == '\0' || only_space) {
            continue;
        }

        /* Get instruction */
        int tokens = sscanf(line, "%31s %31s %31s %31s",
                           opcode_str, operands[0], operands[1], operands[2]);

        if (tokens < 1) {
            printf("Line %d: Invalid instruction\n", line_num);
            continue;
        }

        /* Find instruction */
        instruction_info* instr = find_instruction(opcode_str);
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
            /* A valid register name is detected exactly; anything else
             * (number, label) is treated as an immediate */
            int reg = parse_register(operands[i]);
            char want_type = instr->operand_types[i];

            if (want_type == 'R' && reg == -1) {
                printf("Line %d: operand %d of '%s' must be a register\n",
                       line_num, i + 1, instr->mnemonic);
                return assemble_fail(input_file, output_file, output_filename);
            }

            if (want_type == 'I' && reg != -1) {
                /* The VM always reads 8 bytes for the address of LD/LA/SA;
                 * a register here would emit 1 byte and corrupt the stream */
                printf("Line %d: operand %d of '%s' must be a number or label\n",
                       line_num, i + 1, instr->mnemonic);
                return assemble_fail(input_file, output_file, output_filename);
            }

            if (want_type == 'F' && reg != -1) {
                printf("Line %d: operand %d of '%s' must be a number\n",
                       line_num, i + 1, instr->mnemonic);
                return assemble_fail(input_file, output_file, output_filename);
            }

            if (want_type == 'R') {
                fputc(reg, output_file);
            } else if (want_type == 'F') {
                /* Float immediate: emit the raw IEEE 754 bit pattern */
                uint64_t bits = 0;
                if (parse_float(operands[i], &bits) != 0) {
                    printf("Line %d: Invalid float '%s'\n",
                           line_num, operands[i]);
                    return assemble_fail(input_file, output_file, output_filename);
                }

                for (int j = 0; j < 8; j++) {
                    fputc((bits >> (j * 8)) & 0xFF, output_file);
                }
            } else {
                /* Immediate operand (the 8-byte address/value of LD/LA/SA):
                 * a label resolves to its byte offset, otherwise a number */
                uint64_t num = 0;
                int label_idx = label_lookup(operands[i]);
                if (label_idx >= 0) {
                    num = (uint64_t)label_table[label_idx].offset;
                } else if (parse_number(operands[i], &num) != 0) {
                    printf("Line %d: Invalid number or unknown label '%s'\n",
                           line_num, operands[i]);
                    return assemble_fail(input_file, output_file, output_filename);
                }

                /* Write 8 bytes address (Little endian) */
                for (int j = 0; j < 8; j++) {
                    fputc((num >> (j * 8)) & 0xFF, output_file);
                }
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
                case OP_LOAD:
                case OP_LA: 
                case OP_SA: {
                    if (i == 0) {
                        printf(" R%d", operand);
                    } else {
                        size_t addr = (size_t)operand;
                        for (int j = 1; j < 8; j++) {
                            int next_byte = fgetc(file);
                            if (next_byte == EOF) {
                                printf(" Unexpected EOF\n");
                                fclose(file);
                                return;
                            }
                            addr |= (size_t)next_byte << (j * 8);
                        }
                        printf(" 0x%zx", addr);
                    }
                    break;
                }

                case OP_FLD: {
                    if (i == 0) {
                        printf(" R%d", operand);
                    } else {
                        size_t bits = (size_t)operand;
                        for (int j = 1; j < 8; j++) {
                            int next_byte = fgetc(file);
                            if (next_byte == EOF) {
                                printf(" Unexpected EOF\n");
                                fclose(file);
                                return;
                            }
                            bits |= (size_t)next_byte << (j * 8);
                        }
                        double value;
                        memcpy(&value, &bits, sizeof(value));
                        printf(" %g", value);
                    }
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

                case OP_FADD:
                case OP_FSUB:
                case OP_FMUL:
                case OP_FDIV:
                case OP_FCMP:
                case OP_FLT:
                case OP_FLE: {
                    if (i == 0) printf(" R%d", operand);
                    else printf(" %d", operand);
                    break;
                }

                case OP_ITOF:
                case OP_FTOI: {
                    printf(" R%d", operand);
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

                case OP_PUSH: {
                    printf(" R%d", operand);
                    break;
                }

                case OP_POP: {
                    printf(" R%d", operand);
                    break;
                }

                case OP_CALL: {
                    printf(" R%d", operand);
                    break;
                }

                case OP_RET: {
                    /* No operands */
                    break;
                }

                case OP_TRAP: {
                    printf(" R%d", operand);
                    break;
                }

                case OP_PRINT:
                case OP_FPRT: {
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
    /* Float literals always use '.' as the decimal separator, regardless
     * of the system locale (strtod and %g both honor LC_NUMERIC). */
    setlocale(LC_NUMERIC, "C");

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
        return 1;
    }

    return 0;
}
