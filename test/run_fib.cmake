# Assemble test.rvs and run it in the VM, verifying the expected result.
# Invoked by CTest with:
#   -DASM=<path to rasm> -DRVM=<path to rvm>
#   -DSOURCE=<path to test.rvs> -DBINARY=<output binary path>

execute_process(
    COMMAND "${ASM}" "${SOURCE}" "${BINARY}"
    RESULT_VARIABLE asm_result
    OUTPUT_VARIABLE asm_output
    ERROR_VARIABLE asm_error
)
if(NOT asm_result EQUAL 0)
    message(FATAL_ERROR "Assembler failed (${asm_result}):\n${asm_output}\n${asm_error}")
endif()

# RVM_QUIET disables per-instruction tracing (~90M steps/s vs ~6M with
# tracing); 0 = unlimited step budget (recursive fib(30) runs ~90M steps).
execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env RVM_QUIET=1 "${RVM}" "${BINARY}" 0xffff 0
    RESULT_VARIABLE vm_result
    OUTPUT_VARIABLE vm_output
    ERROR_VARIABLE vm_error
    TIMEOUT 120
)
if(NOT vm_result EQUAL 0)
    message(FATAL_ERROR "VM failed (${vm_result}):\n${vm_output}\n${vm_error}")
endif()

# Recursive fib(30) prints R1 = 832040
string(FIND "${vm_output}" "PRT: R1 = 832040" prt_pos)
if(prt_pos EQUAL -1)
    message(FATAL_ERROR "Expected 'PRT: R1 = 832040' in VM output:\n${vm_output}")
endif()

message(STATUS "fib test passed")
