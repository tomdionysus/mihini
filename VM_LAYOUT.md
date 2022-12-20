# Virtual Machine Layout

* D0-D7 - 8x 64 bit Data Registers
* A0-A7 - 8x 32 bit Address Registers (A7 is SP, Stack pointer)
* Memory Allocation/Deallocation
* Direct interface to the RP2040 GPIO/SPI/I2C/UART

## Condition Register

* Bit 0 - Equal Or Zero
* Bit 1 - Carry Or Borrow
* Bit 2 - Overflow 
* Bit 3 - Negative 

## General Opcode Format

The general opcode format is similar to Motorola 680x0 assembly, as follows:

COMMAND[.WIDTH] [...PARAMETERS]

## Immediate Values

Immediate values are always prefixed with # and presented in 0b, 0d or 0x format.

* (value) - a value of the specified instruction width
* (value8) - an 8 bit value
* (value16) - a 16 bit value
* (value32) - a 32 bit value
* (value64) - a 64 bit value

Example: `#0`, `#0d5`, `#0x20`, `#0b1011`, `#'\n'`

## Addresses (address)

Addresses are always unsigned 32 bit (value32).

## Offsets (offset)

Offsets are always signed 32 bit from the current IP.

### Widths (ww)

* 00 - .b 8 bits
* 01 - .w 16 bits
* 10 - .l 32 bits
* 11 - .x 64 bits

### Registers (arrr)

* 0000 - D0
* 0001 - D1
* 0010 - D2
* 0011 - D3
* 0100 - D4
* 0101 - D5
* 0110 - D6
* 0111 - D7
* 1000 - A0
* 1001 - A1
* 1010 - A2
* 1011 - A3
* 1100 - A4
* 1101 - A5
* 1110 - A6
* 1111 - A7

### Register access (ddd)

* 000 - Direct, the register itself - `A0`
* 001 - Reference, the memory pointed to by the register - `(A0)`
* 010 - Reference with Pre Decrement - `-(Ax)`
* 011 - Reference with Post Increment - `(Ax)+`

### Register Bitmask (regmask)

16 bit mask, low byte D0-D7, high byte A0-A7 e.g 0001000110000010 ==  D1,D7,A0,A4

In assembly, may be written:
	* D0
	* D0,D1
	* D0/D1
	* D0-D2,D4
	* D0-D2/D4
	* D0/A0/D1/A1
	* D2-D6,D0/A4-A5/D1

### Conditional (ccc)

* 000 - Unconditional
* 001 - Zero or Equal
* 010 - Not equal
* 011 - Carry or Borrow
* 100 - Less than
* 101 - Less than or equal
* 110 - Greater than
* 111 - Greater than or equal

## Opcodes

## No Operation

### NOP
* 00000000 - Do Nothing - `NOP`

## Moving data between memory and registers

These instructions move data or a specified width between registers and memory.

### LOAD
* 000001ww (value) (0ddd arrr) - LOAD immediate to register or memory - `LOAD.L #0xA5A5A5A5 D0`

### MOVE
* 000010ww (0ddd arrr) (0ddd arrr) - MOVE register or memory to register or memory - `MOVE.L D1 (A0)`

### PUSH
* 000011ww (bitmask) (0ddd arrr) - PUSH registers to memory -  `PUSH.L D0-D7/A0-A6, -(A7)`

### POP
* 000100ww (bitmask) (0ddd arrr) - POP registers from memory -  `POP.L (A7)+ D0-D7/A0-A6`

## Integer Math

### INC / DEC / CLR
* 000101ww (0ddd arrr) - Increment register or memory - `INC.B D0`
* 000110ww (0ddd arrr) - Decrement register or memory - `DEC.W (A6)+`
* 000111ww (0ddd arrr) - Clear register or memory - `CLR.W (A6)+`

### ADDI / ADD
* 001000ww (value) (0ddd arrr) - Add immediate to register or memory - `ADDI.X #0xFFFFEEEEDDDDCCCC D0`
* 001001ww (0ddd arrr) (0ddd arrr) - Add register or memory to register or memory - `ADD.X (A0) D0`

### SUBI / SUB
* 001010ww (value) (0ddd arrr) - Subtract immediate from register or memory - `SUBI.B #0d2 -(A1)`
* 001011ww (0ddd arrr) (0ddd arrr) - Subtract register or memory from register or memory - `SUB.L D0 (A6)+`

### MULI / MUL
* 001100ww (value) (0ddd arrr) - Multiply register or memory by immediate - `MULI.X #0xFFFFEEEEDDDDCCCC D0`
* 001101ww (0ddd arrr) (0ddd arrr) - Multiply register or memory by register or memory - `MUL.X (A0) D0`

### DIVI / DIV
* 001110ww (value) (0ddd arrr) (0ddd arrr) - Divide register or memory by immediate and store remainder - `DIVI.X #0xFFFFEEEEDDDDCCCC D0`
* 001111ww (0ddd arrr) (0ddd arrr) (0ddd arrr) - Divide first register or memory by second register or memory and store result in first and remainder in third - `DIV.X (A0) D0 D1`

## Bitwise operations

### ANDI / AND
* 010000ww (value) (0ddd arrr) - Bitwise AND register or memory with immediate - `ANDI.W #0b10110101 (A0)`
* 010001ww (0ddd arrr) (0ddd arrr) - Bitwise AND register or memory with register or memory - `AND.W D0, (A6)+`

### ORI / OR
* 010010ww (value) (0ddd arrr) - Bitwise OR register or memory with immediate - `ORI.W #0b10110101 (A0)`
* 010011ww (0ddd arrr) (0ddd arrr) - Bitwise OR register or memory with register or memory - `OR.W D0, (A6)+`

### XORI / XOR
* 010100ww (value) (0ddd arrr) - Bitwise XOR register or memory with immediate - `XORI.W #0b10110101 (A0)`
* 010101ww (0ddd arrr) (0ddd arrr) - Bitwise XOR register or memory with register or memory - `XOR.W D0, (A6)+`

### LSLI / LSL
* 010110ww (value8) (0ddd arrr) - Shift Left register or memory by value8 bits - `LSLI.L #3 (A0)`
* 010111ww (0ddd arrr) (0ddd arrr)- Shift Left register or memory by (8 bit value at register or memory) bits - `LSL.L D0 (A0)`

### LSRI / LSR
* 011000ww (value8) (0ddd arrr) - Shift Right register or memory by value8 bits - `LSRI.L #05 (A0)`
* 011001ww (0ddd arrr) (0ddd arrr)- Shift Right register or memory by (8 bit value at register or memory) bits - `LSR.L D0 (A0)`

## ROLI / ROL
* 011010ww (value8) (0ddd arrr) - Rotate Left register or memory by value8 bits - `ROLI.L #5 (A0)`
* 011011ww (0ddd arrr) (0ddd arrr)- Rotate Left register or memory by (8 bit value at register or memory) bits - `ROL.L D0 (A0)`

## RORI / ROR
* 011100ww (value8) (0ddd arrr) - Rotate Right register or memory by value8 bits - `RORI.L #0d5 (A0)`
* 011101ww (0ddd arrr) (0ddd arrr)- Rotate Right register or memory by (8 bit value at register or memory) bits - `ROR.L D0 (A0)`

### NOT
* 011110ww (0ddd arrr) - NOT register or memory - `NOT.X (A0)`

## Testing

### TSTI
* 011111ww (value8) (0ddd arrr) - Test bit value8 of register or memory - `TSTI.W #1 -(A3)`

### TST
* 100000ww (0ddd arrr) (0ddd arrr) - Test bit value8 of register or memory - `TST.W d0 -(A3)`

### CMPI / CMP
* 100001ww (value) (0ddd arrr) - Compare register or memory with immediate value - `CMPi.B #5 D3`
* 100010ww (0ddd arrr) (0ddd arrr) - Compare register or memory with register or memory - `CMP.B (A0)+ (A1)+`

## Floating Point Math

Floating point works in .w, .l and .x widths (float16, single precision, and double precision respectively).

### FLOAD
* 100010ww (value64) (0ddd arrr) - Load the immediate float16, single, or double into a register - `FLOAD.X #3.14321 D0` (arg2 = (float)arg1)

### FCAST
* 100011ww (0ddd arrr) (0ddd arrr) - Cast the integer in the first to floating point in the second - `FLOAT.L D0 D1` (arg2 = (float)arg1)

### FROUND 
* 100100ww (0ddd arrr) (0ddd arrr) - Round the float in the first to an integer in the second - `FROUND.L D0 D1` (arg2 = round(arg1))

### FADD
* 100101ww (0ddd arrr) (0ddd arrr) - Add two floating point registers, store result in second (arg2 = arg1 + arg2)

### FSUB
* 100110ww (0ddd arrr) (0ddd arrr) - Subtract two floating point registers, store result in second (arg2 = arg1 - arg2)

### FMUL
* 100111ww (0ddd arrr) (0ddd arrr) - Multiply two floating point registers, store result in second (arg2 = arg1 * arg2)

### FDIV
* 101000ww (0ddd arrr) (0ddd arrr) - Divide two floating point registers and place the remainder in the third, and store result in second (arg2 = arg1 / arg2)

### FMOD
* 101001ww (0ddd arrr) (0ddd arrr) - Find the modulus of two floating point registers and store result in second (arg2 = arg1 % arg2)

### FSQRT
* 101010ww (0ddd arrr) (0ddd arrr) - Find the square root of a floating point register and store result in second (arg2 = sqrt(arg1) 

### FEXP
* 101011ww (0ddd arrr) (0ddd arrr) - Find the exponential e^x of a floating point register and store result in second (arg2 = e ^ arg1)

### FLOG
* 101100ww (0ddd arrr) (0ddd arrr) - Find the natural logarithm of first (base e) and store result in second (arg2 = loge(arg1))

### FLOG10
* 101101ww (0ddd arrr) (0ddd arrr) - Find the logarithm of first (base 10) and store result in second  (arg2 = log10(arg2))

### FCEIL
* 101110ww (0ddd arrr) (0ddd arrr) - Rounds register to smallest higher integer and store integer result in second (arg2 = ceil(arg1)) 

### FFLOOR
* 101111ww (0ddd arrr) (0ddd arrr) - Rounds register to largest lower integer and store integer result in second (arg2 = ceil(arg1))

### FPOW
* 110000ww (0ddd arrr) (0ddd arrr) - Find first raised to power second and store result in second (arg2 = arg1 ^ arg2)

### FSIN
* 110001ww (0ddd arrr) (0ddd arrr) - Find the sine of the register or memory and store result in second (arg2 = sin(arg1))

### FCOS
* 110010ww (0ddd arrr) (0ddd arrr) - Find the cosine of the register or memory and store result in second (arg2 = cos(arg1))

### FTAN
* 110011ww (0ddd arrr) (0ddd arrr) - Find the tangent of the register or memory and store result in second (arg2 = tan(arg1))

### F16TO32
* 11010000 (0ddd arrr) (0ddd arrr) - Convert a float16 to a single

### F32TO64
* 11010001 (0ddd arrr) (0ddd arrr) - Convert a single to a double

### F64TO32
* 11010010 (0ddd arrr) (0ddd arrr) - Convert a double to a single

### F32TO16
* 11010011 (0ddd arrr) (0ddd arrr) - Convert a single to a float16

### F16TO64
* 11010000 (0ddd arrr) (0ddd arrr) - Convert a float16 to a double

### F64TO16
* 11010000 (0ddd arrr) (0ddd arrr) - Convert a double to a float16


## Branching (Bxx)

* 11101ccc (offset) - Branch to offset with condition (ccc):
	* BRA - 000 - Unconditional - `BRA label`
	* BEQ - 001 - Zero or Equal - `BEQ end`
	* BNE - 010 - Not equal or non-zero - `BNE loop`
	* BCB - 011 - Carry or Borrow
	* BLT - 100 - Less than
	* BGT - 101 - Less than or equal
	* BLE - 110 - Greater than
	* BGE - 111 - Greater than or equal

## Jumping, Calling, Returning

### JMPI / JMPM / JMP
* 11110000 (address) - Jump to the address - `JMPI 0xFEFFAFDD`
* 11110001 (0ddd arrr) - Jump to the register or memory address - `JMP A4`

### CALLI / CALLM / CALL
* 11110010 (offset) - Place the IP on the A7 stack (Call) and jump to offset address - `CALL sublabel`
* 11110011 (0ddd arrr) - Place the IP on the A7 stack (Call) and jump to offset address - `CALLM A0`

### RET
* 11110100 - Return by popping return address from A7 stack and jumping to it - `RET`

## Memory Management

### ALLOC
* 11110101 (value32) (0ddd arrr) - Allocate value32 bytes of memory and store address in register or memory (NULL if failed) - `ALLOC 0d2048 A0`

### FREE
* 11110110 (0ddd arrr) - Free the memory pointed to by register or memory - `FREE A0`

## System & Extensions

### REFER
* 11111101 (offset) (0ddd arrr) - Add the given offset to the instruction pointer and store the result in register or memory 

### STOP
* 11111101 - Stop the VM - `STOP`

### SYSCALL
* 11111110 (value32) - Call a system routine - see [System Routines](SYSTEM.md)

### [extension]
* 11111111 (instruction16) - 16-bit Extended Instruction follows

# Data Assembly directives

## DATA

Freeform data in decimal, hex, binary, or string format 
* `data.w 26 46 1 1 0x44 0b333`
* `data.b "Hello World\n" 0`


# Extended Instructions

All Start with byte 0xFF

## Floating Point