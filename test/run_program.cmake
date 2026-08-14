# Assemble SOURCE and run it in the VM, verifying that EXPECTED appears in
# the program output. Invoked by CTest with:
#   -DASM=<path to rasm> -DRVM=<path to rvm>
#   -DSOURCE=<path to .rvs> -DBINARY=<output binary path>
#   -DEXPECTED=<substring that must appear in the VM output>

execute_process(
    COMMAND "${ASM}" "${SOURCE}" "${BINARY}"
    RESULT_VARIABLE asm_result
    OUTPUT_VARIABLE asm_output
    ERROR_VARIABLE asm_error
)
if(NOT asm_result EQUAL 0)
    message(FATAL_ERROR "Assembler failed (${asm_result}):\n${asm_output}\n${asm_error}")
endif()

# rasm reports bad lines but still exits 0 (lenient), so also fail the test
# on any assembler diagnostic -- the source must assemble 100% cleanly.
string(FIND "${asm_output}" "Unknown instruction" bad_pos)
if(NOT bad_pos EQUAL -1)
    message(FATAL_ERROR "Assembler reported errors:\n${asm_output}")
endif()

# Run at error log level: per-instruction tracing is off (~90M steps/s vs
# ~6M with tracing) and any VM error still shows up. Default memory size,
# default 100M step budget (recursive fib(30) takes ~37M steps).
execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env RVM_LOGGER_LEVEL=error "${RVM}" "${BINARY}"
    RESULT_VARIABLE vm_result
    OUTPUT_VARIABLE vm_output
    ERROR_VARIABLE vm_error
    TIMEOUT 120
)
if(NOT vm_result EQUAL 0)
    message(FATAL_ERROR "VM failed (${vm_result}):\n${vm_output}\n${vm_error}")
endif()

string(FIND "${vm_output}" "${EXPECTED}" expected_pos)
if(expected_pos EQUAL -1)
    message(FATAL_ERROR "Expected '${EXPECTED}' in VM output:\n${vm_output}")
endif()

message(STATUS "test passed: '${EXPECTED}' found in VM output")
