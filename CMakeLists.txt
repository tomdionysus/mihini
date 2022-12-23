cmake_minimum_required(VERSION 3.15)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY bin)

project(Mihini)

add_compile_options(-Os -ffunction-sections -fdata-sections -fno-exceptions)
add_link_options(-Wl -dead_strip)

include_directories(src)

add_library(mihini src/opcodes.c src/vm.c src/parser.c src/pmalloc.c src/stree.c src/btree.c src/dll.c src/printer.c)

add_executable(mhasm src/mhasm.c)
target_link_libraries(mhasm mihini)

add_executable(mhdasm src/mhdasm.c)
target_link_libraries(mhdasm mihini)

add_executable(mhvm src/mhvm.c)
target_link_libraries(mhvm mihini)

add_executable(mihini_tests tests/asm_test.c)
target_link_libraries(mihini_tests mihini)

add_executable(minimal src/minimal.c)

# add_executable(vm_tests tests/vm_test.c)
# target_link_libraries(vm_tests mihini)