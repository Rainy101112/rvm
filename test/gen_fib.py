#!/usr/bin/env python3
"""Generate the recursive fib RVMASM source (test/test.rvs).

Usage:
    python3 gen_fib.py [OUTPUT]     # FIB_N=30 by default
    FIB_N=25 python3 gen_fib.py     # compute fib(25) instead

Naive recursion using the native PUSH/POP/CALL/RET instructions:
    fib(0) = 0; fib(1) = 1; fib(n) = fib(n-1) + fib(n-2)

Labels are emitted symbolically (e.g. `FIB:`, `LD R6 FIB`); the assembler
resolves them via its two-pass label support, so no hand-computed
addresses appear in the generated source.

Call convention: entry R0 = n; exit R1 = fib(n).  CALL pushes the return
address (address of the next instruction) and jumps to the address in the
operand register; RET pops the return address and jumps back.  The caller
is responsible for balancing its own PUSHes, so the recursive case saves
n and fib(n-1) on the stack across the calls.  fib(n-1) must live on the
stack (below the second call's return address) because every register is
clobbered by the sub-calls.
"""
import os
import sys

N = int(os.environ.get("FIB_N", "30"))  # fib(N)
OUTPUT = sys.argv[1] if len(sys.argv) > 1 else "test.rvs"

out = []          # list of (mnemonic, operands, comment)
pc = 0            # byte offset counter
FIXUPS = {}       # name -> pc

def emit(op, *args, comment=None):
    global pc
    sizes = {"LD": 10, "LA": 10, "SA": 10, "MOV": 3, "ADD": 4, "SUB": 4,
             "CMP": 4, "JNZ": 3, "JMP": 2, "PRT": 2, "HLT": 1, "LABEL": 0, ";": 0,
             "DEC": 2, "PUSH": 2, "POP": 2, "CALL": 2, "RET": 1}
    if op == "LABEL":
        FIXUPS[args[0]] = pc
        out.append((";", (f"{args[0]}:  ; @0x{pc:04X}",), ""))
        return
    out.append((op, args, comment))
    pc += sizes[op]

def call(target):
    emit("LD", "R6", target, comment=f"CALL {target}")
    emit("CALL", "R6")

# ---------------- main ----------------
emit("LD", "R0", f"0x{N:02X}",  comment=f"n = {N}")
call("FIB")
emit("LABEL", "MAIN_RET")
emit("PRT", "R1", comment="print fib(n)")
emit("HLT")

# ---------------- fib ----------------
emit("LABEL", "FIB")
emit(";", "# R0 = n; returns R1 = fib(n)", "")
emit("LD", "R4", "0x00",        comment="if n == 0 return 0")
emit("CMP", "R5", "R0", "R4")
emit("LD", "R4", "RET0")
emit("JNZ", "R5", "R4")
emit("LD", "R4", "0x01",        comment="if n == 1 return 1")
emit("CMP", "R5", "R0", "R4")
emit("LD", "R4", "RET1")
emit("JNZ", "R5", "R4")
# recursive case: n >= 2
emit("PUSH", "R0",              comment="save n")
emit("DEC", "R0",               comment="R0 = n - 1")
call("FIB")                     # R1 = fib(n-1); R0/R2 are clobbered by the call
emit("PUSH", "R1",              comment="save fib(n-1)")
emit("POP", "R2",               comment="R2 = fib(n-1)")
emit("POP", "R0",               comment="R0 = n")
emit("DEC", "R0",               comment="R0 = n - 2")
emit("DEC", "R0")
emit("PUSH", "R2",              comment="re-save fib(n-1) below the call's return addr")
call("FIB")                     # R1 = fib(n-2)
emit("POP", "R2",               comment="R2 = fib(n-1)")
emit("ADD", "R1", "R1", "R2",   comment="R1 = fib(n-1) + fib(n-2)")
emit("RET")
# base cases
emit("LABEL", "RET0")
emit("LD", "R1", "0x00")
emit("RET", comment="return 0")
emit("LABEL", "RET1")
emit("LD", "R1", "0x01")
emit("RET", comment="return 1")

# ---------------- write ----------------
# Label references are emitted symbolically; the assembler resolves them
# (test.rvs stays readable without hand-computed addresses).
resolved = []
for op, args, comment in out:
    if op == ";":
        resolved.append(args[0])
        continue
    if op == "LABEL":
        continue
    comment = f"  ; {comment}" if comment else ""
    resolved.append(f"{op} {' '.join(args)}{comment}")

header = f"""# RVM recursive fib example (benchmark standard)
# fib({N}), computed by naive recursion:
#   fib(0) = 0; fib(1) = 1; fib(n) = fib(n-1) + fib(n-2)
#
# Uses the native PUSH/POP/CALL/RET instructions (8-byte stack slots):
# CALL pushes the return address and jumps; RET pops it and returns.
# The recursive case saves n and fib(n-1) on the stack across the calls.
#
# Registers: R0 arg, R1 result, R2 saved fib(n-1), R4/R5 CMP scratch,
#            R6 call target
"""
with open(OUTPUT, "w") as f:
    f.write(header + "\n".join(resolved) + "\n")
print(f"wrote {OUTPUT}: {pc} bytes (0x{pc:X})")
print(f"labels: {FIXUPS}")
