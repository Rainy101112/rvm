# RVM
A simple cross-platform virtual machine.

## Features
Cross-platform with C language. Fast speed and tidy.

Support both GCC/clang. Completely `0` error `0` warning and less UB.

Runs on Windows, Linux and macOS. Write once run anywhere.

## Compiling
Firstly you should install `cmake`, `make` and any C compiler like `C` or `clang`. If you don't know how to install them just search it on the Internet.

Then run the shell command below:

```bash
	mkdir build
	cd build
	cmake ..
	make -j
```

At last you can see the executable file in the `build` directory. Commonly it will be `rvm` or `rvm.exe`.

## Virtual machine
Currently we have `19` instructions. See below.

```C
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

		OP_JUMP,        // Jump             JMP     [ADDRREG]
		OP_JNZ,         // Jump if not zero JNZ     [REG] [ADDRREG]
		OP_JZ,          // Jump if zero     JZ      [REG] [ADDRREG]
		OP_LOOP,        // Loop             LOOP    [REG] [ADDRREG]

		OP_PRINT,       // Print register   PRT
	};
```

There is `4` registers in the virtual machine. `(R0~R3)`

## RVM assembly
The assemblier code is under the `asm` directory. Please compile it manually.

If you don't know how to compiler an independent code file with C compiler, just search it on the Internet.

Code example:

```RVMASM
	LD R0 0x01	; Load value 0x01 to register R0
	LD R1 0x00	; Load value 0x00 to register R1
	LD R2 0xff	; Load value 0xff to register R2

	PRT R2		; Print the value of register R2

	LOOP R0 R1	; R0 is 0x01 so loop at address 0x00(the value of R1)

	HLT			; Stop
```

