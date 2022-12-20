#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include "vm.h"
#include "pmalloc.h"
#include "opcodes.h"

// #define DEBUG

// System
void vm_init(vmstate_t *state, uint32_t memsize, uint32_t bootsize, uint32_t stacksize) {
	// Clear state
	memset(state, 0, sizeof(vmstate_t));
	// IP = 0
	state->ip = 0;
	// Clear CR
	state->condition = 0;
	// Allocate Host Memory
	state->memory = malloc(memsize);
	// Initialise pmalloc
	paddblock(state->memory+bootsize, memsize-stacksize);
	// Set stack pointer to end of memory
	state->registers.R.A7.i64 = memsize;
}

void vm_shutdown(vmstate_t *state) {
	free(state->memory);
}

uint64_t read_width(vmstate_t *state, uint8_t width) {
	uint64_t val;

	switch(width) {
		case WIDTH_8: 
			val = read_uint8(state);
			return val;
		case WIDTH_16: 
			val = read_uint16(state);
			return val;
		case WIDTH_32: 
			val = read_uint32(state);
			return val;
		case WIDTH_64: 
			val = read_uint64(state);
			return val;
	}

	fprintf(stderr, "error: unknown width\n");
	return 0xFFFFFFFFFFFFFFFF;
}

uint8_t read_uint8(vmstate_t *state) {
	uint8_t v = vm_read_memory(state->ip, WIDTH_8, state);
	state->ip += 1;
	return v;
}

uint16_t read_uint16(vmstate_t *state) {
	uint16_t v = vm_read_memory(state->ip, WIDTH_16, state);
	state->ip += 2;
	return v;
}

uint32_t read_uint32(vmstate_t *state) {
	uint32_t v = vm_read_memory(state->ip, WIDTH_32, state);
	state->ip += 4;
	return v;
}

uint64_t read_uint64(vmstate_t *state) {
	uint64_t v = vm_read_memory(state->ip, WIDTH_64, state);
	state->ip += 8;
	return v;
}

uint64_t vm_read_memory(uint32_t address, uint8_t width, vmstate_t *state) {
	uint64_t out = 0;
	uint8_t *ptr = state->memory + address;

	switch(width) {
		case WIDTH_8:
			// byte
			return *(uint8_t*)(ptr);
		case WIDTH_16:
			// word
			return *(uint16_t*)(ptr);
		case WIDTH_32:
			// long
			return *(uint32_t*)(ptr);
		case WIDTH_64:
			// longlong
			return *(uint64_t*)(ptr);
	}
	return out;
}

void vm_write_memory(uint32_t address, uint64_t value, uint8_t width, vmstate_t *state) {
	uint8_t *ptr = state->memory + address;

	switch(width) {
		case WIDTH_8:
			// byte
			*(uint8_t*)(ptr) = value;
		case WIDTH_16:
			// word
			*(uint16_t*)(ptr) = value;
			break;
		case WIDTH_32:
			// long
			*(uint32_t*)(ptr) = value;
			break;
		case WIDTH_64:
			// longlong
			*(uint64_t*)(ptr) = value;
			break;
	}
}

uint64_t vm_read_register_or_memory(uint8_t reg, uint8_t width, vmstate_t *state) {
	uint8_t mode = reg & 0xF0;
	reg = reg & 0xF;
	uint64_t val;

	switch(mode) {
		case VM_REG_DIRECT:
			// direct
			return state->registers.N[reg];
		case VM_REG_REFERENCE:
			// reference
			return vm_read_memory(state->registers.N[reg], width, state);
		case VM_REG_PRE_DEC:
			// reference with predecrement
			state->registers.N[reg] -= vm_width_to_bytes(width);
			return vm_read_memory(state->registers.N[reg], width, state);
		case VM_REG_POST_INC:
			// reference with postincrement
			val = vm_read_memory(state->registers.N[reg], width, state);
			state->registers.N[reg] += vm_width_to_bytes(width);
			return val;
		default:
			printf("vm_read_register_or_memory: UNKNOWN MODE %02x\n", mode);
			return 0;
	}
}

void vm_write_register_or_memory(uint8_t reg, uint64_t value, uint8_t width, vmstate_t *state) {
	uint8_t mode = reg & 0xF0;
	reg = reg & 0xF;
	uint64_t val, mask;

	switch(mode) {
		case VM_REG_DIRECT:
			// direct
			// mask = vm_width_to_mask(width);
			// state->registers.N[reg] &= !mask;
			// state->registers.N[reg] |= value;
			state->registers.N[reg] = value;
			break;
		case VM_REG_REFERENCE:
			// reference
			vm_write_memory(state->registers.N[reg], value, width, state);
			break;
		case VM_REG_PRE_DEC:
			// reference with predecrement
			state->registers.N[reg] -= vm_width_to_bytes(width);
			vm_write_memory(state->registers.N[reg], value, width, state);
			break;
		case VM_REG_POST_INC:
			// reference with postincrement
			vm_write_memory(state->registers.N[reg], value, width, state);
			state->registers.N[reg] += vm_width_to_bytes(width);
			break;
		default:
			fprintf(stderr, "vm_write_register_or_memory: UNKNOWN MODE %02x\n", mode);
	}
}

uint8_t vm_width_to_bytes(uint8_t width) {
	switch(width) {
		case WIDTH_8:  return 1;
		case WIDTH_16: return 2;
		case WIDTH_32: return 4;
		case WIDTH_64: return 8;
		default:
			fprintf(stderr, "vm_width_to_bytes: Unknown WIDTH");
			return 0;
	}
}

uint64_t vm_width_to_mask(uint8_t width) {
	switch(width) {
		case WIDTH_8: return 0xFF;
		case WIDTH_16: return 0xFFFF;
		case WIDTH_32: return 0xFFFFFFFF;
		case WIDTH_64: return 0xFFFFFFFFFFFFFFFF;
		default:
			fprintf(stderr, "vm_width_to_mask: Unknown WIDTH");
			return 0;
	}
}

void vm_run(uint32_t address, vmstate_t *state) {
	state->ip = address;
	while(!state->stopped) {
		debug("%08x", state->ip);
		vm_opcode_execute(read_uint8(state), state);

		#ifdef DEBUG
		if(getc(stdin)=='r') {
			dump_reg(state);
		}
		#endif
	}
}

// Process 
void vm_opcode_execute(uint8_t op, vmstate_t *state) {
	uint8_t width = 0x11;
	uint32_t val;
	
	if (op < OPCODE_BRA) {
		width = op & WIDTH_64;
		op &= 0b11111100;
	}

	debug(" opcode:%02x w:%d ", op, width);

	switch(op) {
		case OPCODE_NOP:
			vm_opcode_nop(width, state);
			break;
		case OPCODE_LOAD:
			vm_opcode_load(width, read_width(state, width), read_uint8(state), state);
			break;
		case OPCODE_MOVE:
			vm_opcode_move(width, read_uint8(state), read_uint8(state), state);
			break;
		case OPCODE_PUSH:
			vm_opcode_push(width, read_uint16(state), read_uint8(state), state);
			break;
		case OPCODE_POP:
			vm_opcode_pop(width, read_uint16(state), read_uint8(state), state);
			break;
		case OPCODE_INC:
			vm_opcode_inc(width, read_uint8(state), state);
			break;
		case OPCODE_DEC:
			vm_opcode_dec(width, read_uint8(state), state);
			break;
		case OPCODE_CLR:
			vm_opcode_clr(width, read_uint8(state), state);
			break;
		case OPCODE_ADDI:
			vm_opcode_addi(width, read_width(state, width), read_uint8(state), state);
			break;
		case OPCODE_ADD:
			vm_opcode_add(width, read_uint8(state), read_uint8(state), state);
			break;
		case OPCODE_SUBI:
			vm_opcode_subi(width, read_width(state, width), read_uint8(state), state);
			break;
		case OPCODE_SUB:
			vm_opcode_sub(width, read_uint8(state), read_uint8(state), state);
			break;
		case OPCODE_MULI:
			vm_opcode_muli(width, read_width(state, width), read_uint8(state), state);
			break;
		case OPCODE_MUL:
			vm_opcode_mul(width, read_uint8(state), read_uint8(state), state);
			break;
		case OPCODE_DIVI:
			vm_opcode_divi(width, read_width(state, width), read_uint8(state), read_uint8(state), state);
			break;
		case OPCODE_DIV:
			vm_opcode_div(width, read_uint8(state), read_uint8(state), read_uint8(state), state);
			break;
		case OPCODE_ANDI:
			vm_opcode_andi(width, read_width(state, width), read_uint8(state), state);
			break;
		case OPCODE_AND:
			vm_opcode_and(width, read_uint8(state), read_uint8(state), state);
			break;
		case OPCODE_ORI:
			vm_opcode_ori(width, read_width(state, width), read_uint8(state), state);
			break;
		case OPCODE_OR:
			vm_opcode_or(width, read_uint8(state), read_uint8(state), state);
			break;
		case OPCODE_XORI:
			vm_opcode_xori(width, read_width(state, width), read_uint8(state), state);
			break;
		case OPCODE_XOR:
			vm_opcode_xor(width, read_uint8(state), read_uint8(state), state);
			break;
		case OPCODE_LSLI:
			vm_opcode_lsli(width, read_uint8(state), read_uint8(state), state);
			break;
		case OPCODE_LSL:
			vm_opcode_lsl(width, read_uint8(state), read_uint8(state), state);
			break;
		case OPCODE_LSRI:
			vm_opcode_lsri(width, read_uint8(state), read_uint8(state), state);
			break;
		case OPCODE_LSR:
			vm_opcode_lsr(width, read_uint8(state), read_uint8(state), state);
			break;
		case OPCODE_ROLI:
			vm_opcode_roli(width, read_uint8(state),read_uint8(state), state);
			break;
		case OPCODE_ROL:
			vm_opcode_rol(width, read_uint8(state), read_uint8(state), state);
			break;
		case OPCODE_RORI:
			vm_opcode_rori(width, read_uint8(state), read_uint8(state), state);
			break;
		case OPCODE_ROR:
			vm_opcode_ror(width, read_uint8(state), read_uint8(state), state);
			break;
		case OPCODE_NOT:
			vm_opcode_not(width, read_uint8(state), state);
			break;
		case OPCODE_TSTI:
			vm_opcode_tsti(width, read_uint8(state), read_uint8(state), state);
			break;
		case OPCODE_TST:
			vm_opcode_tst(width, read_uint8(state), read_uint8(state), state);
			break;
		case OPCODE_CMPI:
			vm_opcode_cmpi(width, read_width(state, width), read_uint8(state), state);
			break;
		case OPCODE_CMP:
			vm_opcode_cmp(width, read_uint8(state), read_uint8(state), state);
			break;
		case OPCODE_FLOAD:
			vm_opcode_fload(width, read_uint64(state), read_uint8(state), state);
			break;
		case OPCODE_FCAST:
			vm_opcode_fcast(width, read_uint8(state), read_uint8(state), state);
			break;
		case OPCODE_FROUND:
			vm_opcode_fround(width, read_uint8(state), read_uint8(state), state);
			break;
		case OPCODE_FADD:
			vm_opcode_fadd(width, read_uint8(state), read_uint8(state), state);
			break;
		case OPCODE_FSUB:
			vm_opcode_fsub(width, read_uint8(state), read_uint8(state), state);
			break;
		case OPCODE_FMUL:
			vm_opcode_fmul(width, read_uint8(state), read_uint8(state), state);
			break;
		case OPCODE_FDIV:
			vm_opcode_fdiv(width, read_uint8(state), read_uint8(state), state);
			break;
		case OPCODE_FMOD:
			vm_opcode_fmod(width, read_uint8(state), read_uint8(state), state);
			break;
		case OPCODE_FSQRT:
			vm_opcode_fsqrt(width, read_uint8(state), read_uint8(state), state);
			break;
		case OPCODE_FEXP:
			vm_opcode_fexp(width, read_uint8(state), read_uint8(state), state);
			break;
		case OPCODE_FLOG:
			vm_opcode_flog(width, read_uint8(state), read_uint8(state), state);
			break;
		case OPCODE_FLOG10:
			vm_opcode_flog10(width, read_uint8(state), read_uint8(state), state);
			break;
		case OPCODE_FCEIL:
			vm_opcode_fceil(width, read_uint8(state), read_uint8(state), state);
			break;
		case OPCODE_FFLOOR:
			vm_opcode_ffloor(width, read_uint8(state), read_uint8(state), state);
			break;
		case OPCODE_FPOW:
			vm_opcode_fpow(width, read_uint8(state), read_uint8(state), state);
			break;
		case OPCODE_FSIN:
			vm_opcode_fsin(width, read_uint8(state), read_uint8(state), state);
			break;
		case OPCODE_FCOS:
			vm_opcode_fcos(width, read_uint8(state), read_uint8(state), state);
			break;
		case OPCODE_FTAN:
			vm_opcode_ftan(width, read_uint8(state), read_uint8(state), state);
			break;
		case OPCODE_BRA:
			vm_opcode_bra(width, (int32_t)read_uint32(state), state);
			break;
		case OPCODE_BEQ:
			vm_opcode_beq(width, (int32_t)read_uint32(state), state);
			break;
		case OPCODE_BNE:
			vm_opcode_bne(width, (int32_t)read_uint32(state), state);
			break;
		case OPCODE_BCB:
			vm_opcode_bcb(width, (int32_t)read_uint32(state), state);
			break;
		case OPCODE_BLT:
			vm_opcode_blt(width, (int32_t)read_uint32(state), state);
			break;
		case OPCODE_BGT:
			vm_opcode_bgt(width, (int32_t)read_uint32(state), state);
			break;
		case OPCODE_BLE:
			vm_opcode_ble(width, (int32_t)read_uint32(state), state);
			break;
		case OPCODE_BGE:
			vm_opcode_bge(width, (int32_t)read_uint32(state), state);
			break;
		case OPCODE_JMPI:
			vm_opcode_jmpi(width, read_uint32(state), state);
			break;
		case OPCODE_JMPM:
			vm_opcode_jmpm(width, read_uint8(state), state);
			break;
		case OPCODE_JMP:
			vm_opcode_jmp(width, (int32_t)read_uint32(state), state);
			break;
		case OPCODE_CALLI:
			vm_opcode_calli(width, read_uint32(state), state);
			break;
		case OPCODE_CALLM:
			vm_opcode_callm(width, read_uint8(state), state);
			break;
		case OPCODE_CALL:
			vm_opcode_call(width, (int32_t)read_uint32(state), state);
			break;
		case OPCODE_RET:
			vm_opcode_ret(width, state);
			break;
		case OPCODE_ALLOC:
			vm_opcode_alloc(width, read_uint32(state), read_uint8(state), state);
			break;
		case OPCODE_FREE:
			vm_opcode_free(width, read_uint8(state), state);
			break;
		case OPCODE_REFER:
			val = read_uint32(state);
			vm_opcode_refer(width, *(int32_t*)(&val), read_uint8(state), state);
			break;
		case OPCODE_SYSCALL:
			vm_opcode_syscall(width, read_uint32(state), state);
			break;
		case OPCODE_STOP:
			vm_opcode_stop(width, state);
			break;
		case OPCODE_EXT:
			// vm_opcode_ext(width, state);
			break;
		default:
			fprintf(stderr, "vm_opcode_execute: unknown opcode %d\n", op);
	}

	debug("\n");
}

// Opcodes
void vm_opcode_nop(uint8_t width, vmstate_t *state) {
	// Do Nothing.
	debug("nop\n");
}
void vm_opcode_load(uint8_t width, uint64_t value, uint8_t reg, vmstate_t *state) {
	debug("load %d %x\n", value, reg);
	// Load immediate into register or memory
	vm_write_register_or_memory(reg, value, width, state);
	if(value==0) state->condition = CR_EQUAL_ZERO;
}
void vm_opcode_move(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state) {
	uint64_t val = vm_read_register_or_memory(rega, width, state);
	debug("move rega:%d regb:%d regaval: %02x\n", rega, regb, val);
	vm_write_register_or_memory(regb, val, width, state);
	if(val == 0) state->condition = CR_EQUAL_ZERO;
}
void vm_opcode_push(uint8_t width, uint16_t regset, uint8_t reg, vmstate_t *state) {
	debug("push %x %d\n", regset, reg);
	for(uint8_t i=0; i<16; i++) {
		if((regset & (0x1 << i))==0) continue;
		vm_write_register_or_memory(reg, state->registers.N[i], width, state); 
	}
	state->condition = 0;
}
void vm_opcode_pop(uint8_t width, uint16_t regset, uint8_t reg, vmstate_t *state) {
	debug("pop %x %d\n", regset, reg);
	for(uint8_t i=15; i>=0; i--) {
		if((regset & (0x1 << i))==0) continue;
		state->registers.N[i] = vm_read_register_or_memory(reg, width, state); 
	}
	state->condition = 0;
}
void vm_opcode_inc(uint8_t width, uint8_t reg, vmstate_t *state) {
	debug("inc %x %d\n", reg);
	state->condition = 0;
	uint64_t val = vm_read_register_or_memory(reg, width, state);
	uint64_t mask = vm_width_to_mask(width);
	// Detect overflow/carry
	if((val & mask) == mask) state->condition |= CR_CARRY_BORROW;
	val++;
	vm_write_register_or_memory(reg, val, width, state);
	if(val == 0) state->condition |= CR_EQUAL_ZERO;
}
void vm_opcode_dec(uint8_t width, uint8_t reg, vmstate_t *state) {
	debug("dec");
	state->condition = 0;
	uint64_t val = vm_read_register_or_memory(reg, width, state) & vm_width_to_mask(width);
	// Detect underflow/borrow
	if(val == 0) state->condition |= CR_CARRY_BORROW;
	val--;
	vm_write_register_or_memory(reg, val, width, state);
	if(val == 0) state->condition |= CR_EQUAL_ZERO;
}

void vm_opcode_clr(uint8_t width, uint8_t reg, vmstate_t *state) {
	debug("clr");
	state->condition = 0;
	vm_write_register_or_memory(reg, 0, width, state);
	state->condition = CR_EQUAL_ZERO;
}

void vm_opcode_addi(uint8_t width, uint64_t value, uint8_t reg, vmstate_t *state) {
	debug("addi");
	state->condition = 0;
	value = vm_read_register_or_memory(reg, width, state) + value;
	vm_write_register_or_memory(reg, value, width, state);
	if(value == 0) state->condition = CR_EQUAL_ZERO;
}

void vm_opcode_add(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state) {
	debug("add");
	state->condition = 0;
	uint64_t value = vm_read_register_or_memory(rega, width, state) + vm_read_register_or_memory(regb, width, state);
	vm_write_register_or_memory(regb, value, width, state);
	if(value == 0) state->condition = CR_EQUAL_ZERO;
}

void vm_opcode_subi(uint8_t width, uint64_t value, uint8_t reg, vmstate_t *state) {
	debug("subi");
	state->condition = 0;
	value = vm_read_register_or_memory(reg, width, state) - value;
	vm_write_register_or_memory(reg, value, width, state);
	if(value == 0) state->condition = CR_EQUAL_ZERO;
}

void vm_opcode_sub(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state) {
	debug("sub");
	state->condition = 0;
	uint64_t value = vm_read_register_or_memory(rega, width, state) - vm_read_register_or_memory(regb, width, state);
	vm_write_register_or_memory(regb, value, width, state);
	if(value == 0) state->condition = CR_EQUAL_ZERO;
}

void vm_opcode_muli(uint8_t width, uint64_t value, uint8_t reg, vmstate_t *state) {
	debug("muli");
	state->condition = 0;
	value = vm_read_register_or_memory(reg, width, state) * value;
	vm_write_register_or_memory(reg, value, width, state);
	if(value == 0) state->condition = CR_EQUAL_ZERO;
}

void vm_opcode_mul(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state) {
	debug("mul");
	state->condition = 0;
	uint64_t value = vm_read_register_or_memory(rega, width, state) * vm_read_register_or_memory(regb, width, state);
	vm_write_register_or_memory(regb, value, width, state);
	if(value == 0) state->condition = CR_EQUAL_ZERO;
}
void vm_opcode_divi(uint8_t width, uint64_t value, uint8_t rega, uint8_t regb, vmstate_t *state) {
	debug("divi");
	state->condition = 0;
	uint64_t arga, argb;
	arga = vm_read_register_or_memory(rega, width, state);
	vm_write_register_or_memory(rega, arga / value, width, state);
	vm_write_register_or_memory(regb, arga % value, width, state);
	if(value == 0) state->condition = CR_EQUAL_ZERO;
}

void vm_opcode_div(uint8_t width, uint8_t rega, uint8_t regb, uint8_t regc, vmstate_t *state) {
	debug("div");
	state->condition = 0;
	uint64_t arga, argb;
	arga = vm_read_register_or_memory(rega, width, state);
	argb = vm_read_register_or_memory(regb, width, state);
	vm_write_register_or_memory(regb, arga / argb, width, state);
	vm_write_register_or_memory(regc, arga % argb, width, state);
}
void vm_opcode_andi(uint8_t width, uint64_t value, uint8_t reg, vmstate_t *state) {
	debug("andi");
	state->condition = 0;
	value = vm_read_register_or_memory(reg, width, state) & value;
	vm_write_register_or_memory(reg, value, width, state);
	if(value == 0) state->condition = CR_EQUAL_ZERO;
}

void vm_opcode_and(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state) {
	debug("and");
	state->condition = 0;
	uint64_t value = vm_read_register_or_memory(rega, width, state) & vm_read_register_or_memory(regb, width, state);
	vm_write_register_or_memory(regb, value, width, state);
	if(value == 0) state->condition = CR_EQUAL_ZERO;
}
void vm_opcode_ori(uint8_t width, uint64_t value, uint8_t reg, vmstate_t *state) {
	debug("ori");
	state->condition = 0;
	value = vm_read_register_or_memory(reg, width, state) | value;
	vm_write_register_or_memory(reg, value, width, state);
	if(value == 0) state->condition = CR_EQUAL_ZERO;
}

void vm_opcode_or(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state) {
	debug("or");
	state->condition = 0;
	uint64_t value = vm_read_register_or_memory(rega, width, state) | vm_read_register_or_memory(regb, width, state);
	vm_write_register_or_memory(regb, value, width, state);
	if(value == 0) state->condition = CR_EQUAL_ZERO;
}
void vm_opcode_xori(uint8_t width, uint64_t value, uint8_t reg, vmstate_t *state) {
	debug("xori");
	state->condition = 0;
	value = vm_read_register_or_memory(reg, width, state) ^ value;
	vm_write_register_or_memory(reg, value, width, state);
	if(value == 0) state->condition = CR_EQUAL_ZERO;
}
void vm_opcode_xor(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state) {
	debug("xor");
	state->condition = 0;
	uint64_t value = vm_read_register_or_memory(rega, width, state) ^ vm_read_register_or_memory(regb, width, state);
	vm_write_register_or_memory(regb, value, width, state);
	if(value == 0) state->condition = CR_EQUAL_ZERO;
}
void vm_opcode_lsli(uint8_t width, uint64_t value, uint8_t reg, vmstate_t *state) {
	debug("lsli");
	state->condition = 0;
	value = vm_read_register_or_memory(reg, width, state) << value;
	vm_write_register_or_memory(reg, value, width, state);
	if(value == 0) state->condition = CR_EQUAL_ZERO;
}
void vm_opcode_lsl(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state) {
	debug("lsl");
	state->condition = 0;
	uint64_t value = vm_read_register_or_memory(rega, width, state) << vm_read_register_or_memory(regb, width, state);
	vm_write_register_or_memory(regb, value, width, state);
	if(value == 0) state->condition = CR_EQUAL_ZERO;
}
void vm_opcode_lsri(uint8_t width, uint64_t value, uint8_t reg, vmstate_t *state) {
	debug("lsri");
	state->condition = 0;
	value = vm_read_register_or_memory(reg, width, state) >> value;
	vm_write_register_or_memory(reg, value, width, state);
	if(value == 0) state->condition = CR_EQUAL_ZERO;
}
void vm_opcode_lsr(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state) {
	debug("lsr");
	state->condition = 0;
	uint64_t value = vm_read_register_or_memory(rega, width, state) >> vm_read_register_or_memory(regb, width, state);
	vm_write_register_or_memory(regb, value, width, state);
	if(value == 0) state->condition = CR_EQUAL_ZERO;
}

void vm_opcode_roli(uint8_t width, uint64_t value, uint8_t reg, vmstate_t *state) {
	debug("roli");
}

void vm_opcode_rol(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state) {
	debug("rol");
}

void vm_opcode_rori(uint8_t width, uint64_t value, uint8_t reg, vmstate_t *state) {
	debug("rori");
}

void vm_opcode_ror(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state) {
	debug("ror");
}

void vm_opcode_not(uint8_t width, uint8_t reg, vmstate_t *state) {
	debug("not");
	uint64_t value = !vm_read_register_or_memory(reg, width, state);
	vm_write_register_or_memory(reg, value, width, state);
	if(value == 0) state->condition = CR_EQUAL_ZERO;
}
void vm_opcode_tsti(uint8_t width, uint8_t bit, uint8_t reg, vmstate_t *state) {
	uint64_t value = vm_read_register_or_memory(reg, width, state);
	debug("tsti width:%d bit:%d value:%02x", width, bit, value);
	value &= (0x1 << bit);
	if(value == 0) state->condition = CR_EQUAL_ZERO;
}

void vm_opcode_tst(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state) {
	state->condition = 0;
	uint64_t valuea = vm_read_register_or_memory(rega, WIDTH_8, state);
	uint64_t valueb = vm_read_register_or_memory(regb, width, state);
	debug("tst width:%d rega:%02x regb:%2x regavalue: %d regbvalue:%02x on:%d", width,rega, regb, valuea, valueb, (!(valueb & (1 << valuea))) ? 0:1);
	if(!(valueb & (1 << valuea))) state->condition |= CR_EQUAL_ZERO;
}

void vm_opcode_cmpi(uint8_t width, uint64_t varb, uint8_t reg, vmstate_t *state) {
	debug("cmpi");
	state->condition = 0;
	uint64_t vara = vm_read_register_or_memory(reg, width, state);
	if(varb == vara) state->condition |= CR_EQUAL_ZERO;
	if(varb > vara) state->condition |= CR_GREATER_THAN;
	if(varb < vara) state->condition |= CR_LESS_THAN;
}
void vm_opcode_cmp(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state) {
	debug("cmp");
	state->condition = 0;
	uint64_t vara = vm_read_register_or_memory(rega, width, state);
	uint64_t varb = vm_read_register_or_memory(regb, width, state);
	if(varb == vara) state->condition |= CR_EQUAL_ZERO;
	if(varb > vara) state->condition |= CR_GREATER_THAN;
	if(varb < vara) state->condition |= CR_LESS_THAN;
}

void vm_opcode_fload(uint8_t width, uint64_t value, uint8_t reg, vmstate_t *state) {}
void vm_opcode_fcast(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state) {}
void vm_opcode_fround(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state) {}
void vm_opcode_fadd(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state) {}
void vm_opcode_fsub(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state) {}
void vm_opcode_fmul(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state) {}
void vm_opcode_fdiv(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state) {}
void vm_opcode_fmod(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state) {}
void vm_opcode_fsqrt(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state) {}
void vm_opcode_fexp(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state) {}
void vm_opcode_flog(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state) {}
void vm_opcode_flog10(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state) {}
void vm_opcode_fceil(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state) {}
void vm_opcode_ffloor(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state) {}
void vm_opcode_fpow(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state) {}
void vm_opcode_fsin(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state) {}
void vm_opcode_fcos(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state) {}
void vm_opcode_ftan(uint8_t width, uint8_t rega, uint8_t regb, vmstate_t *state) {}

void vm_opcode_bra(uint8_t width, int32_t offset, vmstate_t *state) {
	debug("bra");
	state->ip += offset + 4;
}
void vm_opcode_beq(uint8_t width, int32_t offset, vmstate_t *state) {
	debug("beq");
	if((state->condition & CR_EQUAL_ZERO)) state->ip += offset + 4;
}
void vm_opcode_bne(uint8_t width, int32_t offset, vmstate_t *state) {
	debug("bne CR_EQUAL_ZERO: %d", (state->condition & CR_EQUAL_ZERO) ? 1 : 0);
	if(!(state->condition & CR_EQUAL_ZERO)) state->ip += offset + 4;
}
void vm_opcode_bcb(uint8_t width, int32_t offset, vmstate_t *state) {
	debug("bcb");
	if(!(state->condition & CR_CARRY_BORROW)) state->ip += offset + 4;
}
void vm_opcode_blt(uint8_t width, int32_t offset, vmstate_t *state) {
	debug("blt");
	if((state->condition & CR_LESS_THAN)) state->ip += offset + 4;
}
void vm_opcode_bgt(uint8_t width, int32_t offset, vmstate_t *state) {
	debug("bgt");
	if((state->condition & CR_GREATER_THAN)) state->ip += offset + 4;
}
void vm_opcode_ble(uint8_t width, int32_t offset, vmstate_t *state) {
	debug("ble");
	if((state->condition & (CR_EQUAL_ZERO | CR_LESS_THAN))) state->ip += offset + 4;
}
void vm_opcode_bge(uint8_t width, int32_t offset, vmstate_t *state) {
	debug("bge");
	if((state->condition & (CR_EQUAL_ZERO | CR_GREATER_THAN))) state->ip += offset + 4;
}
void vm_opcode_jmpi(uint8_t width, uint32_t address, vmstate_t *state) {
	debug("jmpi");
	state->ip = address;
}
void vm_opcode_jmpm(uint8_t width, uint8_t reg, vmstate_t *state) {
	debug("jmpm");
	state->ip = vm_read_register_or_memory(reg, WIDTH_32, state);
}
void vm_opcode_jmp(uint8_t width, int32_t offset, vmstate_t *state) {
	debug("jmp");
	state->ip += offset + 4;
}
void vm_opcode_calli(uint8_t width, uint32_t address, vmstate_t *state) {
	debug("calli");
	vm_write_register_or_memory(VM_REG_PRE_DEC | VM_REG_A7, state->ip, WIDTH_32, state);
	state->ip = address;
}
void vm_opcode_callm(uint8_t width, uint8_t reg, vmstate_t *state)  {
	debug("callm");
	vm_write_register_or_memory(VM_REG_PRE_DEC | VM_REG_A7, state->ip, WIDTH_32, state);
	state->ip = vm_read_register_or_memory(reg, WIDTH_32, state);
}
void vm_opcode_call(uint8_t width, int32_t offset, vmstate_t *state) {
	debug("call");
	vm_write_register_or_memory(VM_REG_PRE_DEC | VM_REG_A7, state->ip, WIDTH_32, state);
	debug(" offset = %d new ip = %08x", offset + 4, state->ip + offset + 4);
	state->ip += offset + 4;
}
void vm_opcode_ret(uint8_t width, vmstate_t *state) {
	uint32_t ptr = vm_read_register_or_memory(VM_REG_POST_INC | VM_REG_A7, WIDTH_32, state);
	debug("ret to %08x ", ptr);
	state->ip = ptr;
}
void vm_opcode_alloc(uint8_t width, uint32_t size, uint8_t rega, vmstate_t *state) {
	debug("alloc");
	vm_write_register_or_memory(rega, (uint64_t)pmalloc(size), WIDTH_32, state);
}
void vm_opcode_free(uint8_t width, uint8_t reg, vmstate_t *state) {
	debug("free");
	pfree((void*)vm_read_register_or_memory(reg, WIDTH_32, state));
}
void vm_opcode_refer(uint8_t width, int32_t offset, uint8_t reg, vmstate_t *state) {
	debug("refer");
	vm_write_register_or_memory(reg, state->ip + offset + 3, WIDTH_32, state);
	// dump_mem(state->ip + offset + 3, state);
}
void vm_opcode_syscall(uint8_t width, uint32_t op, vmstate_t *state) {
	debug("syscall");
	uint32_t num;
	uint32_t str;
	uint8_t ch;

	switch(op) {
		case VM_SYSCALL_DUMP_MEMORY:
			num = vm_read_register_or_memory(VM_REG_POST_INC | VM_REG_A7, WIDTH_32, state);
			dump_mem(num, state);
			break;

		case VM_SYSCALL_PRINT:
			str = vm_read_register_or_memory(VM_REG_POST_INC | VM_REG_A7, WIDTH_32, state);
			printf("%s", str+(char*)state->memory);
			break;

		case VM_SYSCALL_PUTC:
			ch = vm_read_register_or_memory(VM_REG_POST_INC | VM_REG_A7, WIDTH_8, state);
			putc(ch, stdout);
			break;

		default:
			fprintf(stderr, "vm_opcode_syscall: unknown syscall op #%d\n", op);
			break;
	}
}

void vm_opcode_stop(uint8_t width, vmstate_t *state) {
	debug("stop");
	state->stopped = true;
}

void dump_mem(uint32_t address, vmstate_t *state) {
	printf("\n\nDump Memory %08x\n", address);
	uint8_t i,j,c;
	for(i = 0; i<4; i++) {
		printf("%08x  ", address+(i*8));
		for(j = 0; j<16; j++) {
			c = state->memory[address+(i*16)+j];
			printf("%02x ", c);
		}

		printf(" ");

		for(j = 0; j<16; j++) {
			c = state->memory[address+(i*16)+j];
			printf("%c", c >= ' ' ? c : '.');
		}
		printf("\n");
	}
}

void dump_reg(vmstate_t *state) {
	printf("\n\nDump Registers\n");
	uint8_t r = 0;
	for(uint8_t i=0; i<4; i++) {
		printf("D%d %016llx ", r, state->registers.N[r]);
		r++;
	}
	printf("\n");
	for(uint8_t i=0; i<4; i++) {
		printf("D%d %016llx ", r, state->registers.N[r]);
		r++;
	}
	printf("\n\n");
	for(uint8_t i=0; i<4; i++) {
		printf("A%d %08llx ", r-8, state->registers.N[r] & 0xFFFFFFFF);
		r++;
	}
	printf("\n");
	for(uint8_t i=0; i<4; i++) {
		printf("A%d %08llx ", r-8, state->registers.N[r] & 0xFFFFFFFF);
		r++;
	}
	printf("\n");
	printf("CR_EQUAL_ZERO %d ", state->condition & CR_EQUAL_ZERO ? 1:0); 
	printf("CR_CARRY_BORROW %d ", state->condition & CR_CARRY_BORROW ? 1:0); 
	printf("CR_OVERFLOW %d\n ", state->condition & CR_OVERFLOW ? 1:0); 
	printf("CR_NEGATIVE %d ", state->condition & CR_NEGATIVE ? 1:0); 
	printf("CR_GREATER_THAN %d ", state->condition & CR_GREATER_THAN ? 1:0); 
	printf("CR_LESS_THAN %d\n", state->condition & CR_LESS_THAN ? 1:0); 
	printf("\n");
}

void debug(const char *fmt, ...) {
#ifdef DEBUG
	va_list args;
	va_start(args, fmt);
	vfprintf(stdout, fmt, args);
	va_end (args);
#endif
}
