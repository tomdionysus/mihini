#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>

#include "pmalloc.h"

#define EXCEPTION_DIVIDE_BY_ZERO 0xF0000001
#define EXCEPTION_ADDRESS_ERROR  0xF0000002

#define CR_EQUAL_ZERO	0b00000001
#define CR_CARRY_BORROW	0b00000010
#define CR_OVERFLOW		0b00000100
#define CR_NEGATIVE		0b00001000
#define CR_GREATER_THAN	0b00010000
#define CR_LESS_THAN	0b00100000

#define VM_REG_DIRECT		0b00000000
#define VM_REG_REFERENCE	0b00010000
#define VM_REG_PRE_DEC		0b00100000
#define VM_REG_POST_INC		0b00110000

#define VM_REG_D0			0b00000000
#define VM_REG_D1			0b00000001
#define VM_REG_D2			0b00000010
#define VM_REG_D3			0b00000011
#define VM_REG_D4			0b00000100
#define VM_REG_D5			0b00000101
#define VM_REG_D6			0b00000110
#define VM_REG_D7			0b00000111

#define VM_REG_A0			0b00001000
#define VM_REG_A1			0b00001001
#define VM_REG_A2			0b00001010
#define VM_REG_A3			0b00001011
#define VM_REG_A4			0b00001100
#define VM_REG_A5			0b00001101
#define VM_REG_A6			0b00001110
#define VM_REG_A7			0b00001111

// Version
#define MIHINI_VERSION 0x1

typedef union vm_register_t {
	uint64_t i64;
	uint32_t i32;
	uint16_t i16;
	uint8_t i8;
} vm_register_t;

typedef struct registers_t {
	vm_register_t D0;
	vm_register_t D1;
	vm_register_t D2;
	vm_register_t D3;
	vm_register_t D4;
	vm_register_t D5;
	vm_register_t D6;
	vm_register_t D7;
	vm_register_t A0;
	vm_register_t A1;
	vm_register_t A2;
	vm_register_t A3;
	vm_register_t A4;
	vm_register_t A5;
	vm_register_t A6;
	vm_register_t A7;
} registers_t;

typedef union allregisters_t {
	registers_t R;
	struct {
		uint64_t D[8];
		uint64_t A[8];
	} da;
	uint64_t N[16];
} allregisters_t;

typedef struct vmstate_t {
	allregisters_t registers;
	uint8_t condition;
	uint32_t ip;
	uint8_t *memory;
	bool stopped;
} vmstate_t;

typedef struct exception_t {
	uint32_t errorcode;
	vmstate_t state;
} exception_t;

// System
void vm_init(vmstate_t *state, uint32_t memsize, uint32_t bootsize, uint32_t stacksize);
void vm_shutdown(vmstate_t *state);

// Read
uint64_t read_width(vmstate_t *state, uint8_t width);
uint8_t read_uint8(vmstate_t *state);
uint16_t read_uint16(vmstate_t *state);
uint32_t read_uint32(vmstate_t *state);
uint64_t read_uint64(vmstate_t *state);

// Low level
uint64_t vm_read_memory(uint32_t address, uint8_t width, vmstate_t *state);
void vm_write_memory(uint32_t address, uint64_t value, uint8_t width, vmstate_t *state);
uint64_t vm_read_register_or_memory(uint8_t reg, uint8_t width, vmstate_t *state);
void vm_write_register_or_memory(uint8_t reg, uint64_t value, uint8_t width, vmstate_t *state);

uint8_t vm_width_to_bytes(uint8_t width);
uint64_t vm_width_to_mask(uint8_t width);

// Main entry
void vm_run(uint32_t address, vmstate_t *state);

// Opcodes
void vm_opcode_execute(uint8_t op, vmstate_t *state);

void vm_opcode_nop(uint8_t width, vmstate_t *state);
void vm_opcode_load(uint8_t width, uint64_t value, uint8_t reg, vmstate_t *state);
void vm_opcode_move(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state);
void vm_opcode_push(uint8_t width, uint16_t regset, uint8_t reg, vmstate_t *state);
void vm_opcode_pop(uint8_t width, uint16_t regset, uint8_t reg, vmstate_t *state);
void vm_opcode_inc(uint8_t width, uint8_t reg, vmstate_t *state);
void vm_opcode_dec(uint8_t width, uint8_t reg, vmstate_t *state);
void vm_opcode_clr(uint8_t width, uint8_t reg, vmstate_t *state);

void vm_opcode_addi(uint8_t width, uint64_t value, uint8_t reg, vmstate_t *state);
void vm_opcode_add (uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state);
void vm_opcode_subi(uint8_t width, uint64_t value, uint8_t reg, vmstate_t *state);
void vm_opcode_sub(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state);
void vm_opcode_muli(uint8_t width, uint64_t value, uint8_t reg, vmstate_t *state);
void vm_opcode_mul(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state);
void vm_opcode_divi(uint8_t width, uint64_t value, uint8_t rega, uint8_t regb, vmstate_t *state);
void vm_opcode_div(uint8_t width, uint8_t rega, uint8_t regb, uint8_t regc, vmstate_t *state);
void vm_opcode_andi(uint8_t width, uint64_t value, uint8_t reg, vmstate_t *state);
void vm_opcode_and(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state);
void vm_opcode_ori(uint8_t width, uint64_t value, uint8_t reg, vmstate_t *state);
void vm_opcode_or(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state);
void vm_opcode_xori(uint8_t width, uint64_t value, uint8_t reg, vmstate_t *state);
void vm_opcode_xor(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state);
void vm_opcode_lsli(uint8_t width, uint64_t value, uint8_t reg, vmstate_t *state);
void vm_opcode_lsl(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state);
void vm_opcode_lsri(uint8_t width, uint64_t value, uint8_t reg, vmstate_t *state);
void vm_opcode_lsr(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state);
void vm_opcode_roli(uint8_t width, uint64_t value, uint8_t reg, vmstate_t *state);
void vm_opcode_rol(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state);
void vm_opcode_rori(uint8_t width, uint64_t value, uint8_t reg, vmstate_t *state);
void vm_opcode_ror(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state);
void vm_opcode_not(uint8_t width, uint8_t reg, vmstate_t *state);
void vm_opcode_tsti(uint8_t width, uint8_t bit, uint8_t reg, vmstate_t *state);
void vm_opcode_tst(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state);
void vm_opcode_cmpi(uint8_t width, uint64_t value, uint8_t reg, vmstate_t *state);
void vm_opcode_cmp(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state);

void vm_opcode_fload(uint8_t width, uint64_t value, uint8_t reg, vmstate_t *state);
void vm_opcode_fcast(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state);
void vm_opcode_fround(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state);
void vm_opcode_fadd(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state);
void vm_opcode_fsub(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state);
void vm_opcode_fmul(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state);
void vm_opcode_fdiv(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state);
void vm_opcode_fmod(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state);
void vm_opcode_fsqrt(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state);
void vm_opcode_fexp(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state);
void vm_opcode_flog(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state);
void vm_opcode_flog10(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state);
void vm_opcode_fceil(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state);
void vm_opcode_ffloor(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state);
void vm_opcode_fpow(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state);
void vm_opcode_fsin(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state);
void vm_opcode_fcos(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state);
void vm_opcode_ftan(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state);

void vm_opcode_bra(uint8_t width, int32_t offset, vmstate_t *state);
void vm_opcode_beq(uint8_t width, int32_t offset, vmstate_t *state);
void vm_opcode_bne(uint8_t width, int32_t offset, vmstate_t *state);
void vm_opcode_bcb(uint8_t width, int32_t offset, vmstate_t *state);
void vm_opcode_blt(uint8_t width, int32_t offset, vmstate_t *state);
void vm_opcode_bgt(uint8_t width, int32_t offset, vmstate_t *state);
void vm_opcode_ble(uint8_t width, int32_t offset, vmstate_t *state);
void vm_opcode_bge(uint8_t width, int32_t offset, vmstate_t *state);
void vm_opcode_jmpi(uint8_t width, uint32_t address, vmstate_t *state);
void vm_opcode_jmpm(uint8_t width, uint8_t reg, vmstate_t *state);
void vm_opcode_jmp(uint8_t width, int32_t offset, vmstate_t *state);
void vm_opcode_calli(uint8_t width, uint32_t address, vmstate_t *state);
void vm_opcode_callm(uint8_t width, uint8_t reg, vmstate_t *state);
void vm_opcode_call(uint8_t width, int32_t offset, vmstate_t *state);
void vm_opcode_ret(uint8_t width, vmstate_t *state);
void vm_opcode_alloc(uint8_t width, uint32_t size, uint8_t rega, vmstate_t *state);
void vm_opcode_free(uint8_t width, uint8_t reg, vmstate_t *state);
void vm_opcode_refer(uint8_t width, int32_t offset, uint8_t reg, vmstate_t *state);
void vm_opcode_syscall(uint8_t width, uint32_t op, vmstate_t *state);
void vm_opcode_stop(uint8_t width, vmstate_t *state);

void dump_mem(uint32_t address, vmstate_t *state);
void dump_reg(vmstate_t *state);

void debug(const char *fmt, ...);

#define VM_SYSCALL_DUMP_MEMORY  0x0
#define VM_SYSCALL_DUMP_STATE 	0x1
#define VM_SYSCALL_PRINT 		0x2
#define VM_SYSCALL_PUTC 		0x3
// #define VM_SYSCALL_GETC 		0x4
