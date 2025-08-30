# RVM
A simple cross-platform virtual machine.

## Features
- Cross-platform compatibility using the C language
- High performance and clean design
- Supports both GCC and Clang compilers
- Zero errors, zero warnings, and minimal undefined behavior
- 64-bit addressing support
- Runs on Windows, Linux, and macOS — write once, run anywhere

## Compilation
To build RVM, ensure you have `cmake`, `make`, and a C compiler (such as `GCC` or `Clang`) installed. If you need help installing these tools, please refer to online resources.

Run the following commands:

```bash
mkdir build
cd build
cmake ..
make -j
```

The resulting executable will be located in the `build` directory, typically named `rvm` or `rvm.exe`.

## Virtual machine
RVM currently supports `22` instructions, listed below:

```C
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

	OP_TRAP,        // Trap                         TRAP    [REG] [NUMREG]

	OP_PRINT,       // Print register               PRT     [REG]
};
```

The virtual machine includes `8` registers (`R0` to `R7`).

## RASM
The assembler source code is located in the `asm` directory. Please compile it manually.
If you are unfamiliar with compiling standalone C code, consult online tutorials.

RASM usage:

```
RASM [INPUT_FILE] [OUTPUT_BINARY]
```

Example code:

```RVMASM
LD R0 0x01   ; Load value 0x01 into register R0
LD R1 0x00   ; Load value 0x00 into register R1
LD R2 0xff   ; Load value 0xff into register R2

PRT R2       ; Print the value in R2

LOOP R0 R1   ; Loop using R0 (0x01) at the address stored in R1 (0x00)

HLT          ; Halt execution
```

