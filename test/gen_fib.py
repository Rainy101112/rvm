#!/usr/bin/env python3
"""Generate the recursive fib RVMASM source (test/test.rvs).

Usage:
    python3 gen_fib.py [OUTPUT]     # FIB_N=30 by default
    FIB_N=25 python3 gen_fib.py     # compute fib(25) instead

Recursion is implemented with an explicit stack (R7 = SP, grows down from
0x1000, slot = 8 bytes) plus self-modifying code: since LA/SA take an
immediate address, push/pop patch the address field of an inline SA/LA
with the current SP right before executing it.

Call convention: entry R0 = n, [SP+8] = return address; exit R1 = fib(n),
SP restored. "return" = pop R6; JMP R6.
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
             "DEC": 2}
    if op == "LABEL":
        FIXUPS[args[0]] = pc
        out.append((";", (f"{args[0]}: @0x{pc:04X}",), ""))
        return
    out.append((op, args, comment))
    pc += sizes[op]

def push(reg):
    """store reg at [SP]; SP -= 8 (self-modifying inline trampoline)"""
    site = pc + 3 + 10  # MOV(3) + patch SA(10) -> inline "SA r 0"
    emit("MOV", "R5", "R7",   comment=f"push {reg}: R5 = SP")
    emit("SA", "R5", f"0x{site+2:04X}", comment=f"patch addr field of SA @0x{site:04X}")
    emit("SA", reg, "0x0000", comment="(patched) store at [SP]")
    emit("SUB", "R7", "R7", "R3", comment="SP -= 8")

def pop(reg):
    """SP += 8; reg = [SP]"""
    site = pc + 4 + 3 + 10  # ADD(4) + MOV(3) + patch SA(10) -> inline "LA r 0"
    emit("ADD", "R7", "R7", "R3", comment=f"pop {reg}: SP += 8")
    emit("MOV", "R5", "R7",  comment="R5 = SP")
    emit("SA", "R5", f"0x{site+2:04X}", comment=f"patch addr field of LA @0x{site:04X}")
    emit("LA", reg, "0x0000", comment="(patched) load [SP]")

def call(target_name, ret_label):
    emit("LD", "R6", ret_label, comment=f"R6 = {ret_label} (return addr)")
    emit("MOV", "R1", "R6")
    push("R1")
    emit("LD", "R6", target_name, comment=f"call {target_name}")
    emit("JMP", "R6")

# ---------------- main ----------------
emit("LD", "R0", f"0x{N:02X}",  comment=f"n = {N}")
emit("LD", "R7", "0x1000",      comment="SP = stack base (grows down)")
emit("LD", "R3", "0x08",        comment="R3 = 8 (slot size)")
call("FIB", "MAIN_RET")
emit("LABEL", "MAIN_RET")
emit("PRT", "R1", comment="print fib(n)")
emit("HLT")

# ---------------- fib ----------------
emit("LABEL", "FIB")
emit(";", "R0 = n; returns R1 = fib(n). [SP+8] = return address", "")
emit("LD", "R4", "0x00",        comment="if n == 0 return 0")
emit("CMP", "R5", "R0", "R4")
emit("LD", "R4", "RET0")
emit("JNZ", "R5", "R4")
emit("LD", "R4", "0x01",        comment="if n == 1 return 1")
emit("CMP", "R5", "R0", "R4")
emit("LD", "R4", "RET1")
emit("JNZ", "R5", "R4")
# recursive case: n >= 2
emit("MOV", "R1", "R0")
push("R1")                      # save n
emit("DEC", "R0",               comment="R0 = n - 1")
call("FIB", "AFTER1")           # R1 = fib(n-1)
emit("LABEL", "AFTER1")
emit("MOV", "R2", "R1",         comment="R2 = fib(n-1)")
# child returned with SP = E-8; the dead AFTER1 slot at [E-8] stays below
# SP and is overwritten by the next call level, so only two pops remain:
pop("R0")                       # R0 = n
pop("R6")                       # R6 = this frame's return addr
emit("MOV", "R1", "R2")
push("R1")                      # save fib(n-1) -- R2 is clobbered by the next call
emit("DEC", "R0",               comment="R0 = n - 2")
emit("DEC", "R0")
emit("MOV", "R1", "R6")
push("R1")                      # restore return addr
call("FIB", "AFTER2")           # R1 = fib(n-2)
emit("LABEL", "AFTER2")
pop("R6")                       # this frame's return addr
pop("R2")                       # R2 = saved fib(n-1)
emit("ADD", "R2", "R2", "R1",   comment="R2 = fib(n-1) + fib(n-2)")
emit("MOV", "R1", "R2")
emit("JMP", "R6",               comment="return")
# base cases
emit("LABEL", "RET0")
emit("LD", "R1", "0x00")
pop("R6")
emit("JMP", "R6", comment="return 0")
emit("LABEL", "RET1")
emit("LD", "R1", "0x01")
pop("R6")
emit("JMP", "R6", comment="return 1")

# ---------------- resolve labels & write ----------------
resolved = []
for op, args, comment in out:
    if op == ";":
        resolved.append(args[0])
        continue
    if op == "LABEL":
        continue
    new_args = []
    for a in args:
        if a in FIXUPS:
            new_args.append(f"0x{FIXUPS[a]:04X}")
        else:
            new_args.append(a)
    comment = f"  ; {comment}" if comment else ""
    resolved.append(f"{op} {' '.join(new_args)}{comment}")

header = f"""# RVM recursive fib example (benchmark standard)
# fib({N}), computed by naive recursion:
#   fib(0) = 0; fib(1) = 1; fib(n) = fib(n-1) + fib(n-2)
#
# RVM has no CALL/RET and LA/SA take immediate addresses, so recursion is
# emulated with an explicit stack (R7 = SP, 8-byte slots growing down from
# 0x1000) and self-modifying code: each push/pop patches the address field
# of an inline SA/LA with the current SP before executing it, and "return"
# is JMP to the return address popped from the stack.
#
# Stack discipline: SP points at the last occupied slot; push stores at
# [SP] then SP -= 8, pop does SP += 8 then loads [SP].  A callee enters
# with SP = E and its return address at [E+8], and returns with SP = E+8.
# Frame (recursive case, entry SP = E):
#   [E+8]  return address (later reused to hold fib(n-1) across the call)
#   [E]    saved n
#   [E-8]  return address of the active sub-call (dead slot is reclaimed)
#
# Registers: R0 arg, R1 result/scratch, R2 saved fib(n-1), R3 = 8,
#            R4/R5 scratch, R6 return address / jump target, R7 SP
"""
with open(OUTPUT, "w") as f:
    f.write(header + "\n".join(resolved) + "\n")
print(f"wrote {OUTPUT}: {pc} bytes (0x{pc:X})")
print(f"labels: {FIXUPS}")
