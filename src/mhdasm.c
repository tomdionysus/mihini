#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <regex.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>

#include "endian_shim.h"
#include "mhdasm.h"
#include "stree.h"
#include "btree.h"
#include "opcodes.h"
#include "printer.h"
#include <time.h>

// Main
int main(int argc, char *argv[]) {
	// printf("Mihini Disassembler v0.1\n");
	if(argc != 3) {
      fprintf(stderr, "Usage: %s source destination.mh\n", argv[0]);
		return -1;
	}

	// Disassembler state
	state_t state = {};

	load_opcodes_strings(&(state.opcodes));

	state.source_filename = argv[1];
	state.dest_filename = argv[2];
	state.label_num = 2;

	btree_add(&(state.labels), 0, (void*)1);

	// Open Source File
	// printf("Opening %s\n", state.source_filename);
	state.source = fopen(state.source_filename ,"r");

	// Can't open file
	if(state.source == NULL) {
      fprintf(stderr, "Error opening source file %s\n", state.source_filename);
      return cleanup(&state);
	}

   	// Open Destination File
	// printf("Opening %s\n", state.dest_filename);
	state.dest = fopen(state.dest_filename, "wb");

	// Can't open file
	if(state.dest == NULL) {
      fprintf(stderr, "Error opening destination file %s\n", state.dest_filename);
      return cleanup(&state);
	}

	// Read Mihini header
	char buffer[1024];

	read_buffer(&state, buffer, 6); buffer[6]=0;
	if(strcmp("mihini", buffer)!=0) {
		fprintf(stderr, "error: source file '%s' has an incorrect header (mihini<version>)\n", state.source_filename);
		return cleanup(&state);
	}

	uint32_t version = read_uint32(&state);
	if (version > MIHINI_VERSION) {
		fprintf(stderr, "error: source file '%s' is version %d, this disassembler version %d is too old.\n", state.source_filename, version, MIHINI_VERSION);
		return cleanup(&state);
	}

	time_t t = time(NULL);

   /* Output the current year into the result string */
   strftime(buffer, sizeof(buffer), "%H:%M:%S %Z %a %d %b %Y", localtime(&t));

	// Write asm header
	fprintf(state.dest, "// Mihini Disassembler (v%d)\n", MIHINI_VERSION); state.line_number++;
	fprintf(state.dest, "// source: %s (v%d)\n", state.source_filename, version); state.line_number++;
	fprintf(state.dest, "// date: %s\n", buffer); state.line_number++;

	// printf("First Pass %s\n", state.source_filename);
	while(!feof(state.source)) {
		uint8_t opcode = read_uint8(&state);
		if(first_pass(opcode, &state)) break;
	}

	// Seek to beginning, second pass
	fseek(state.source, 10, SEEK_SET);
	state.pos = 0;

	uint32_t labels = 1;

	// Disassemble
	// printf("Second Pass %s\n", state.source_filename);
	while(!feof(state.source)) {
		// printf("Pos %d ", state.pos);
		uint8_t opcode = read_uint8(&state);
		if(disassemble(opcode, &state)) break;
	}

	// printf("Finished, disassembled %d bytes, output file %d lines\n", state.pos, state.line_number); 

	return cleanup(&state);
}

bool first_pass(uint8_t opcode, state_t* state) {
	uint8_t origopcode = opcode;
	uint8_t width = opcode & 0b11;
	int32_t offset;
	if(opcode < OPCODE_BRA) opcode &= 0b11111100;
	btree_node_t *node = btree_get(state->opcodes, opcode);
	if(node == NULL) {
		printf("error %s:%d unknown opcode %02x\n", state->source_filename, state->line_number, opcode);
	}
	char *opcode_asm = (char*)node->value;

	switch(opcode) {
		// No arguments
		case OPCODE_NOP:
		case OPCODE_RET:
		case OPCODE_STOP:
			break;

		// One immediate arg, no width
		case OPCODE_JMPI:
		case OPCODE_CALLI:
		case OPCODE_SYSCALL:
			read_uint32(state);
			break;

		// One register arg, has width
		case OPCODE_NOT:
		case OPCODE_INC:
		case OPCODE_DEC:
		case OPCODE_CLR:
		case OPCODE_JMPM:
		case OPCODE_CALLM:
		case OPCODE_FREE:
			read_uint8(state);
			break;

		// One label arg, no width
		case OPCODE_BRA:
		case OPCODE_BEQ:
		case OPCODE_BNE:
		case OPCODE_BCB:
		case OPCODE_BLT:
		case OPCODE_BGT:
		case OPCODE_BLE:
		case OPCODE_BGE:
		case OPCODE_CALL:
			offset = (int32_t)read_uint32(state);
			// printf("bxx/call reference to position %d\n", (int32_t)state->pos + offset);
			btree_add(&(state->labels), (int32_t)state->pos + offset, (void*)(uint64_t)(state->label_num++));
			break;

		// 2 args - One label, one register
		case OPCODE_REFER:
			offset = (int32_t)read_uint32(state);
			// printf("refer reference to position %d\n", (int32_t)state->pos + offset);
			btree_add(&(state->labels), (int32_t)state->pos + offset, (void*)(uint64_t)(state->label_num++));
			read_uint8(state);
			break;

		// 2 args - One bitmask, one register, no width
		case OPCODE_POP:
		case OPCODE_PUSH:
			read_uint16(state);
			read_uint8(state);
			break;	


		// 2 args - One immediate arg, one register, no width
		case OPCODE_ALLOC:
			read_uint32(state);
			read_uint8(state);
			break;

		// 2 args - One immediate arg, one register, has width
		case OPCODE_LOAD:
		case OPCODE_ADDI:
		case OPCODE_SUBI:
		case OPCODE_MULI:
		case OPCODE_ANDI:
		case OPCODE_ORI:
		case OPCODE_XORI:
		case OPCODE_LSLI:
		case OPCODE_LSRI:
		case OPCODE_ROLI:
		case OPCODE_RORI:
		case OPCODE_CMPI:
		case OPCODE_FLOAD:
			read_uint64(state);
			read_uint8(state);
			break;

		// 2 args - One byte immediate arg, one register, has width
		case OPCODE_TSTI:
			read_uint8(state);
			read_uint8(state);
			break;

		// 2 args - Two registers, has width
		case OPCODE_MOVE:
		case OPCODE_ADD:
		case OPCODE_SUB:
		case OPCODE_MUL:
		case OPCODE_AND:
		case OPCODE_OR:
		case OPCODE_XOR:
		case OPCODE_LSL:
		case OPCODE_LSR:
		case OPCODE_ROL:
		case OPCODE_ROR:
		case OPCODE_CMP:
		case OPCODE_TST:

		case OPCODE_FCAST:
		case OPCODE_FROUND:
		case OPCODE_FADD:
		case OPCODE_FSUB:
		case OPCODE_FMUL:
		case OPCODE_FDIV:
		case OPCODE_FMOD:
		case OPCODE_FSQRT:
		case OPCODE_FEXP:
		case OPCODE_FLOG:
		case OPCODE_FLOG10:
		case OPCODE_FCEIL:
		case OPCODE_FFLOOR:
		case OPCODE_FPOW:
		case OPCODE_FSIN:
		case OPCODE_FCOS:
		case OPCODE_FTAN:
			read_uint8(state);
			read_uint8(state);
			break;

		// 3 args - One immediate arg, two registers, has width
		case OPCODE_DIVI:
			read_variable(state, width);
			read_uint8(state);
			read_uint8(state);
			break;

		// 3 args - 3 registers, has width
		case OPCODE_DIV:
			read_uint8(state);
			read_uint8(state);
			read_uint8(state);
			break;
	}

	return false;
} 

bool disassemble(uint8_t opcode, state_t* state) {
	char buffer1[1024];
	char buffer2[1024];
	char buffer3[1024];

	btree_node_t *node = btree_get(state->labels, state->pos-1);
	if(node!=NULL) {
		// printf("Adding label label%d\n", node->value); 
		fprintf(state->dest,"\nlabel%d", (uint32_t)(uint64_t)node->value); state->line_number++;
	} else {
		fprintf(state->dest,"\t");
	}

	uint8_t origopcode = opcode;
	int32_t offset;

	uint8_t width = opcode & 0b11;

	// Opcodes less than OPCODE_BRA use the last two bits for width.
	if(opcode < OPCODE_BRA) opcode &= 0b11111100;
	
	node = btree_get(state->opcodes, opcode);
	
	if(node == NULL) {
		printf("error %s:%d unknown opcode %02x\n", state->source_filename, state->line_number, opcode);
		return 0;
	}

	char *opcode_asm = (char*)node->value;

	// printf("Opcode: 0x%02x ASM: %s Width: %d\n", origopcode, opcode_asm, width);

	switch(opcode) {
		// No arguments
		case OPCODE_NOP:
		case OPCODE_RET:
		case OPCODE_STOP:
			fprintf(state->dest, "\t%s\n", opcode_asm); state->line_number++;
			break;

		// One label arg, no width
		case OPCODE_BRA:
		case OPCODE_BEQ:
		case OPCODE_BNE:
		case OPCODE_BCB:
		case OPCODE_BLT:
		case OPCODE_BGT:
		case OPCODE_BLE:
		case OPCODE_BGE:
		case OPCODE_CALL:
		case OPCODE_JMP:
			offset = read_uint32(state);

			node = btree_get(state->labels, state->pos + offset + 4);
			if(node!=NULL) {
				fprintf(state->dest, "\t%s label%d\n", opcode_asm, (uint32_t)(uint64_t)node->value); state->line_number++;
			} else {
				printf("error %s:%d bxx/jmp - cannot find label for position %d\n", state->source_filename, state->line_number, state->pos + offset + 4);
			}
			break;

		// One immediate arg, no width
		case OPCODE_JMPI:
		case OPCODE_CALLI:
		case OPCODE_SYSCALL:
			print_immediate_value(read_uint32(state), buffer1);
			fprintf(state->dest, "\t%s %s\n", opcode_asm, buffer1); state->line_number++;
			break;

		// 2 args - One label, one register
		case OPCODE_REFER:
			offset = read_uint32(state);
			node = btree_get(state->labels, state->pos + offset + 4);
			print_register(read_uint8(state), buffer1);
			if(node!=NULL) {
				fprintf(state->dest, "\t%s label%d %s\n", opcode_asm, (uint32_t)(uint64_t)node->value, buffer1); state->line_number++;
			} else {
				printf("error %s:%d refer - cannot find label for position %d\n", state->source_filename, state->line_number, state->pos + offset + 4);
			}
			break;


		// 2 args - One bitmask, one register, no width
		case OPCODE_PUSH:
			print_register_bitmask(read_uint16(state), buffer1);
			print_register(read_uint8(state), buffer2);
			
			fprintf(state->dest, "\t%s.%c %s %s\n", opcode_asm, print_width(width), buffer1, buffer2); state->line_number++;
			break;	

		case OPCODE_POP:
			print_register_bitmask(read_uint16(state), buffer1);
			print_register(read_uint8(state), buffer2);
			
			fprintf(state->dest, "\t%s.%c %s %s\n", opcode_asm, print_width(width), buffer2, buffer1); state->line_number++;
			break;

		// One register arg, no width
		case OPCODE_JMPM:
		case OPCODE_CALLM:
		case OPCODE_FREE:
			print_register(read_uint8(state), buffer1);
			fprintf(state->dest, "\t%s %s\n", opcode_asm, buffer1); state->line_number++;
			break;

		// One register arg, has width
		case OPCODE_NOT:
		case OPCODE_INC:
		case OPCODE_DEC:
		case OPCODE_CLR:
			print_register(read_uint8(state), buffer1);
			fprintf(state->dest, "\t%s.%c %s\n", opcode_asm, print_width(width), buffer1); state->line_number++;
			break;

		// 2 args - One immediate arg, one register, no width
		case OPCODE_ALLOC:
			print_immediate_value(read_uint32(state), buffer1);
			print_register(read_uint8(state), buffer2);

			fprintf(state->dest, "\t%s %s %s\n", opcode_asm, buffer1, buffer2); state->line_number++;
			break;

		// 2 args - One immediate arg, one register, has width
		case OPCODE_LOAD:
		case OPCODE_ADDI:
		case OPCODE_SUBI:
		case OPCODE_MULI:
		case OPCODE_ANDI:
		case OPCODE_ORI:
		case OPCODE_XORI:
		case OPCODE_LSLI:
		case OPCODE_LSRI:
		case OPCODE_ROLI:
		case OPCODE_RORI:
		case OPCODE_CMPI:
		case OPCODE_TSTI:
			print_immediate_value(read_uint8(state), buffer1);
			print_register(read_uint8(state), buffer2);
			
			fprintf(state->dest, "\t%s.%c %s %s\n", opcode_asm, print_width(width), buffer1, buffer2); state->line_number++;
			break;

		// 2 args - One immediate float arg, one register, has width
		case OPCODE_FLOAD:
			print_immediate_float_value(read_uint64(state), buffer1);
			print_register(read_uint8(state), buffer2);
			
			fprintf(state->dest, "\t%s.%c %s %s\n", opcode_asm, print_width(width), buffer1, buffer2); state->line_number++;
			break;	

		// 2 args - Two registers, has width
		case OPCODE_MOVE:
		case OPCODE_ADD:
		case OPCODE_SUB:
		case OPCODE_MUL:
		case OPCODE_AND:
		case OPCODE_OR:
		case OPCODE_XOR:
		case OPCODE_LSL:
		case OPCODE_LSR:
		case OPCODE_ROL:
		case OPCODE_ROR:
		case OPCODE_CMP:
		case OPCODE_TST:

		case OPCODE_FCAST:
		case OPCODE_FROUND:
		case OPCODE_FADD:
		case OPCODE_FSUB:
		case OPCODE_FMUL:
		case OPCODE_FDIV:
		case OPCODE_FMOD:
		case OPCODE_FSQRT:
		case OPCODE_FEXP:
		case OPCODE_FLOG:
		case OPCODE_FLOG10:
		case OPCODE_FCEIL:
		case OPCODE_FFLOOR:
		case OPCODE_FPOW:
		case OPCODE_FSIN:
		case OPCODE_FCOS:
		case OPCODE_FTAN:
			print_register(read_uint8(state), buffer1);
			print_register(read_uint8(state), buffer2);

			fprintf(state->dest, "\t%s.%c %s %s\n", opcode_asm, print_width(width), buffer1, buffer2); state->line_number++;
			break;

		// 3 args - One immediate arg, two registers, has width
		case OPCODE_DIVI:
			print_immediate_value(read_variable(state, width), buffer1);
			print_register(read_uint8(state), buffer2);
			print_register(read_uint8(state), buffer3);

			fprintf(state->dest, "\t%s.%c %s %s %s\n", opcode_asm, print_width(width), buffer1, buffer2, buffer3); state->line_number++;
			break;

		// 3 args - 3 registers, has width
		case OPCODE_DIV:
			print_register(read_uint8(state), buffer1);
			print_register(read_uint8(state), buffer2);
			print_register(read_uint8(state), buffer3);

			fprintf(state->dest, "\t%s.%c %s %s %s\n", opcode_asm, print_width(width), buffer1, buffer2, buffer3); state->line_number++;
			break;
	}

	return false;
} 

uint32_t cleanup(state_t *state) {
	if (state->source!=NULL) { fclose(state->source); state->source = NULL; }
	if (state->dest!=NULL) { fclose(state->dest); state->dest = NULL; }
	if (state->labels!=NULL) { btree_free(&(state->labels)); }
	if (state->opcodes!=NULL) { btree_free(&(state->opcodes));  }
	return 0;
}

uint8_t read_uint8(state_t *state) {
	uint8_t value;
	fread(&value,1,1,state->source);
	state->pos++;
	return value;
}

uint16_t read_uint16(state_t *state) {
	uint16_t value;
	fread(&value,2,1,state->source);
	state->pos += 2;
	return value;
}

uint32_t read_uint32(state_t *state) {
	uint32_t value;
	fread(&value,4,1,state->source);
	state->pos += 4;
	return value;
}

uint64_t read_uint64(state_t *state) {
	uint64_t value;
	fread(&value,8,1,state->source);
	state->pos += 8;
	return value;
}

uint64_t read_variable(state_t *state, uint8_t width) {
	switch(width) {
		case WIDTH_8:
			return read_uint8(state);
			break;
		case WIDTH_16:
			return read_uint16(state);
			break;
		case WIDTH_32:
			return read_uint32(state);
			break;
		case WIDTH_64:
			return read_uint64(state);
			break;
	}

	return 0xFFFFFFFFFFFFFFFF;
}

size_t read_buffer(state_t *state, void *buffer, uint32_t length) {
	return fread(buffer,1,length,state->source);
}
