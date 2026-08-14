cmake_minimum_required(VERSION 3.12)
project(rvm VERSION 1.0.0 LANGUAGES C)

# C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Set build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Platform options
option(ENABLE_DEBUG "Enable debug output" OFF)
option(ENABLE_TESTS "Build tests" OFF)

# System platform options
if(WIN32)
    # Windows
    add_definitions(-D_WIN32)
else()
    # UNIX
    add_definitions(-D_POSIX_C_SOURCE=200809L)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lm")
endif()

# Compiler parameters
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O2 -g3")
    if(ENABLE_DEBUG)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
    endif()
endif()

include_directories(include)

file(GLOB_RECURSE SOURCES "src/*.c")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_executable(${PROJECT_NAME} ${SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:DEBUG=1>
)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include/${PROJECT_NAME}
    FILES_MATCHING PATTERN "*.h"
)

if(ENABLE_TESTS)
    enable_testing()
    message(STATUS "Testing enabled")

    # RASM: standalone assembler used to assemble the test programs
    add_executable(rasm asm/asm.c)

    # Generic runner: assemble a test program, run it, and check that the
    # expected output substring appears.
    function(rvm_add_test name source expected)
        add_test(NAME ${name}
            COMMAND ${CMAKE_COMMAND}
                -DASM=$<TARGET_FILE:rasm>
                -DRVM=$<TARGET_FILE:rvm>
                -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/test/${source}
                -DBINARY=${CMAKE_CURRENT_BINARY_DIR}/${name}.bin
                -DEXPECTED=${expected}
                -P ${CMAKE_CURRENT_SOURCE_DIR}/test/run_program.cmake)
    endfunction()

    # fib(30) runs ~37M steps with the default budget, regression-testing
    # both the integer and the floating-point instruction sets.
    rvm_add_test(fib test.rvs "PRT: R1 = 832040")
    rvm_add_test(fib_float fib_float.rvs "FPRT: R1 = 832040")
    rvm_add_test(float_test float_test.rvs "FPRT: R2 = inf")
endif()

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${PROJECT_NAME}ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

# Print configuation
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C compiler: ${CMAKE_C_COMPILER}")
message(STATUS "Source files: ${SOURCES}")
