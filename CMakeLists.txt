cmake_minimum_required(VERSION 3.15)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

project(Mihini)

include_directories(src)

add_executable(mhasm src/mhasm.c src/parser.c src/dll.c src/opcodes.c src/stree.c src/btree.c)
add_executable(mhdasm src/mhdasm.c src/dll.c src/opcodes.c src/stree.c src/btree.c src/printer.c)
add_executable(mihini src/mihini.c src/vm.c src/parser.c src/pmalloc.c src/stree.c src/btree.c)

add_executable(mihini_tests tests/asm_test.c src/dll.c src/parser.c src/pmalloc.c src/stree.c src/btree.c src/printer.c)
# add_executable(vm_test tests/vm_test.c)