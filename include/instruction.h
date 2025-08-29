/* 
 *
 *      instruction.h
 * 
 *      By Rainy101112 2025/8/28
 *      Public under MIT license
 * 
 *      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *      IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *      FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * 
 */

#ifndef INCLUDE_INSTRUCTION_H_
#define INCLUDE_INSTRUCTION_H_

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

#endif // INCLUDE_INSTRUCTION_H_
