#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <regex.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>

#include "endian_shim.h"
#include "mhasm.h"
#include "stree.h"
#include "opcodes.h"
#include "parser.h"

#include "vm.h"

// Main
int main(int argc, char *argv[]) {
	// printf("Mihini Assembler v0.1\n");
	if(argc != 3) {
     	fprintf(stderr, "Usage: %s source.mh destination\n", argv[0]);
		return -1;
	}

	// Assembler state
	state_t state = {};

	// Opcodes
	state.opcodes = stree_new('-');
	load_opcodes_values(state.opcodes);
	// stree_balance(&state.opcodes);

	// Set filenames
	state.source_filename = argv[1];
	state.dest_filename = argv[2];
	state.labels = stree_new('-');
	state.fwd_labels = stree_new('-');
	state.error = stderr;

	// Open Source File
	// printf("Opening %s\n", state.source_filename );
	state.source = fopen(state.source_filename ,"r");

	// Can't open file
	if(state.source == NULL) {
      fprintf(state.error, "Error opening source file %s\n", state.source_filename);
      return(-1);
	}

   	// Open Destination File
	// printf("Opening %s\n", state.dest_filename);
	state.dest = fopen(state.dest_filename, "w+");

	// Can't open file
	if(state.dest == NULL) {
      fprintf(state.error, "Error opening destination file %s\n", state.dest_filename);
      return(-1);
	}

	// INIT
	state.line_number = 0;
	char buffer2[1024];

	// Header
	write_buffer(&state, "mihini", 6);
	write_uint32(&state, MIHINI_VERSION);

    // Parse line by line
	while(!feof(state.source)) {
		if(fgets(state.sourceline, 1024, state.source) == NULL) break;
		state.line_number++;

		// Trim it up
		str_trim_right(state.sourceline);

		// Empty Line?
		if(strlen(state.sourceline)==0) continue;

		parse(state.sourceline, &state.offsets);
		if(!assemble(&state)) break; 
	}


	stree_iterate(state.fwd_labels, fix_fwd_labels_iter, &state);

	// printf("Finished, assembled %d lines, output file size %d bytes\n", state.line_number, state.pos); 

	stree_free(&(state.labels));
	stree_free(&(state.opcodes));

	stree_iterate(state.fwd_labels, free_fwd_labels_iter, &state);
	stree_free(&(state.fwd_labels));

	fclose(state.dest);
	fclose(state.source);
}

// Iterators

bool fix_fwd_labels_iter_2(void *value, void *context) {
	forward_ref_t *ref = (forward_ref_t*)value;
	uint32_t pos = (uint32_t)(uint64_t)context;

	int32_t offset = (int32_t)pos - ref->dest_pos - 8;
	
	// printf("- position %d, target %d, offset %d\n", ref->dest_pos, pos, offset);
	fseek(ref->dest, ref->dest_pos, SEEK_SET);
	fwrite(&offset, 4, 1, ref->dest);

	return false;
}

bool fix_fwd_labels_iter(char *key, void *value, void *context) { 
	// Get the state context
	state_t *state = (state_t*)context;
	dll_t *ref_pos_list = (dll_t*)value;

	// printf("%s: %d references\n", key, ((dll_t*)value)->count);
	
	// Find label
	stree_node_t *node = stree_get(state->labels, key);
	if(node==NULL) {
		// Ref not found.
		fprintf(state->error,"%s error: reference not defined '%s'\n", state->source_filename, key);
		return false;
	} else {
		// Iterate all refs and fixup
		dll_iterate(ref_pos_list, fix_fwd_labels_iter_2, node->value, true);
	}

	return false;
}

bool free_fwd_labels_iter(char *key, void *value, void *context) { 
	dll_free((dll_t*)value);
	return false;
}

bool whitespace(char c) { return (c == ' ') || (c == '\t') || (c == '\r') || (c == '\n'); }

void str_trim_right(char *buffer) {
	char *end = buffer + strlen(buffer)-1;
	while (end >= buffer && whitespace(*end)) { *end = 0; end--; }
}

bool parse(char *buffer, source_line_t *offsets) {
	memset(offsets,0, sizeof(source_line_t));

	char *start = buffer;
	segment_t current = {};

	// Get first
	uint8_t pos = next_segment(buffer,0, &current);

	if(current.start == 0) {
		// No initial space
		if(buffer[current.start]=='/' && buffer[current.start+1]=='/') {
			// Comment - ignore rest of line
			current.end = strlen(buffer);
			current.length = current.end-current.start;
			offsets->comment = current;
			return true;
		} else {
			// Label
			offsets->label = current;
			pos = current.end;
		}
	} else {
		// Initial space - cannot be label
		if(buffer[current.start]=='/' && buffer[current.start+1]=='/') {
			// Comment - ignore rest of line
			current.end = strlen(buffer);
			current.length = current.end-current.start;
			offsets->comment = current;
			return true;
		} else {
			// Opcode
			if(buffer[current.end-2]=='.') {
				// Width?
				current.length -= 2;
				current.end -= 2;
				offsets->width.start = current.end+1;
				offsets->width.length = 1;
				offsets->width.end = offsets->width.start+1;
				pos = offsets->width.end;
			} else {
				pos = current.end;
			}
			offsets->opcode = current;
		}
	}

	// Opcode/First argument
	pos = next_segment(buffer, pos, &current);
	if(current.length==0) return true;
	if(buffer[current.start]=='/' && buffer[current.start+1]=='/') {
		// Comment - ignore rest of line
		current.end = strlen(buffer);
		current.length = current.end-current.start;
		offsets->comment = current;
		return true;
	} else {
		if(offsets->opcode.length==0) {
			// Opcode
			if(buffer[current.end-2]=='.') {
				// Width?
				current.length -= 2;
				current.end -= 2;
				offsets->width.start = current.end+1;
				offsets->width.length = 1;
				offsets->width.end = offsets->width.start+1;
				pos = offsets->width.end;
			} else {
				pos = current.end;
			}
			offsets->opcode = current;
		} else {
			offsets->arg1 = current;
			pos = current.end;
		}
	}

	// Second/First argument
	pos = next_segment(buffer, pos, &current);
	if(current.length==0) return true;
	if(buffer[current.start]=='/' && buffer[current.start+1]=='/') {
		// Comment - ignore rest of line
		current.end = strlen(buffer);
		current.length = current.end-current.start;
		offsets->comment = current;
		return true;
	} else {
		if(offsets->arg1.length==0) {
			offsets->arg1 = current;
		} else {
			offsets->arg2 = current;
		}
		pos = current.end;
	}

	// Third/Second argument
	pos = next_segment(buffer, pos, &current);
	if(current.length==0) return true;
	if(buffer[current.start]=='/' && buffer[current.start+1]=='/') {
		// Comment - ignore rest of line
		current.end = strlen(buffer);
		current.length = current.end-current.start;
		offsets->comment = current;
		return true;
	} else {
		if(offsets->arg2.length==0) {
			offsets->arg2 = current;
		} else {
			offsets->arg3 = current;
		}
		pos = current.end;
	}

	// Any trailing comment
	pos = next_segment(buffer, pos, &current);
	if(current.length==0) return true;
	if(buffer[current.start]=='/' && buffer[current.start+1]=='/')
	{
		// Comment - ignore rest of line
		current.end = strlen(buffer);
		current.length = current.end-current.start;
		offsets->comment = current;
		return true;
	}

	return true;
}

uint8_t next_segment(char *buffer, uint8_t pos, segment_t *out)
{
	// Skip whitespace
	while(buffer[pos]!=0 && whitespace(buffer[pos])) pos++;
	// Write down start
	out->start = pos;
	// Skip Text
	while(buffer[pos]!=0 && !whitespace(buffer[pos])) pos++;
	out->end = pos;
	out->length = out->end - out->start;
	return pos+1;
}

// Get segment of string from offset and lowercase it
void get_segment(char *dest, char *src, segment_t segment)
{
	uint8_t j = 0;
	uint8_t i = segment.start;
	while(i<segment.end && src[i]!=0) 
	{
		char c = src[i++];
		if(c>='A' && c<='Z') c += 'a'-'A';
		dest[j++]=c;
	}

	dest[j++] = 0;
}

bool assemble(state_t *state) {
	char strsegment[1024];
	uint8_t opcode, width;

	// Label defined?
	if(state->offsets.label.length != 0) {
		get_segment(strsegment, state->sourceline, state->offsets.label);
		
		// printf("Label Definition: '%s' pos %d\n", strsegment, state->pos); 

		// If the label already exists, fail with error
		if(stree_exists(state->labels, strsegment)) {
			fprintf(state->error,"%s:%d:%d: error: label '%s' redefinition\n", state->source_filename, state->line_number, state->offsets.label.start, strsegment);
			return false;
		}
		// Create the label
		stree_set(state->labels, strsegment, (void*)(uint64_t)state->pos);
	}

	// Opcode with width?
	width = 0;
	if(state->offsets.width.length != 0) {
		get_segment(strsegment, state->sourceline, state->offsets.width);
		width = parse_width(strsegment[0], state);
	}

	// The opcode itself
	if(state->offsets.opcode.length != 0) {
		get_segment(strsegment, state->sourceline, state->offsets.opcode);

		// Lookup Opcode
		stree_node_t *node = stree_get(state->opcodes, strsegment);

		// Opcode not found
		if(node == NULL) {
			fprintf(state->error,"%s:%d:%d: error: unknown opcode '%s'\n", state->source_filename, state->line_number, state->offsets.opcode.start, strsegment);
			return false;
		}

		uint8_t opcode = (uint8_t)(uint64_t)node->value;

		switch(opcode) {
			// No arguments
			case OPCODE_NOP:
			case OPCODE_RET:
			case OPCODE_STOP:
				if(!check_args(strsegment, state, false, 0)) return false;
				write_uint8(state, opcode);
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
				if(!check_args(strsegment, state, false, 1)) return false;
				write_uint8(state, opcode);
				write_uint32(state, parse_label(state->sourceline, state->offsets.arg1, state));
				break;

			// One immediate arg, no width
			case OPCODE_CALLI:
			case OPCODE_JMPI:
			case OPCODE_SYSCALL:
				if(!check_args(strsegment, state, false, 1)) return false;
				write_uint8(state, opcode);
				write_uint32(state, parse_immediate_value_source(state->sourceline, state->offsets.arg1, 2, state));
				break;

			// One regsiter arg, no width
			case OPCODE_CALLM:
			case OPCODE_JMPM:
			case OPCODE_FREE:
				if(!check_args(strsegment, state, false, 1)) return false;
				write_uint8(state, opcode | width);
				write_uint8(state, parse_reg_source(state->sourceline, state->offsets.arg1, state));
				break;

			// One register arg, has width
			case OPCODE_NOT:
			case OPCODE_INC:
			case OPCODE_DEC:
			case OPCODE_CLR:
				if(!check_args(strsegment, state, true, 1)) return false;
				write_uint8(state, opcode | width);
				write_uint8(state, parse_reg_source(state->sourceline, state->offsets.arg1, state));
				break;

			// 2 args - One label, one register, no width
			case OPCODE_REFER:
				if(!check_args(strsegment, state, false, 2)) return false;
				write_uint8(state, opcode);
				write_uint32(state, parse_label(state->sourceline, state->offsets.arg1, state));
				write_uint8(state, parse_reg_source(state->sourceline, state->offsets.arg2, state));
				break;

			// 2 args - One immediate arg, one register, no width
			case OPCODE_ALLOC:
				if(!check_args(strsegment, state, false, 2)) return false;
				write_uint8(state, opcode | width);
				write_uint32(state, parse_immediate_value_source(state->sourceline, state->offsets.arg1, width, state));
				write_uint8(state, parse_reg_source(state->sourceline, state->offsets.arg2, state));
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
				if(!check_args(strsegment, state, true, 2)) return false;
				write_uint8(state, opcode | width);
				write_variable(state, parse_immediate_value_source(state->sourceline, state->offsets.arg1, width, state), width);
				write_uint8(state, parse_reg_source(state->sourceline, state->offsets.arg2, state));
				break;

			// 2 args - One byte immediate arg, one register, has width
			case OPCODE_TSTI:
				if(!check_args(strsegment, state, true, 2)) return false;
				write_uint8(state, opcode | width);
				write_uint8(state, parse_immediate_value_source(state->sourceline, state->offsets.arg1, width, state));
				write_uint8(state, parse_reg_source(state->sourceline, state->offsets.arg2, state));
				break;

			// 2 args - One immediate float arg, one register, has width
			case OPCODE_FLOAD:
				if(!check_args(strsegment, state, true, 2)) return false;
				write_uint8(state, opcode | width);
				write_uint64(state, parse_immediate_value_float_source(state->sourceline, state->offsets.arg1, width, state));
				write_uint8(state, parse_reg_source(state->sourceline, state->offsets.arg2, state));
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
				if(!check_args(strsegment, state, true, 2)) return false;
				write_uint8(state, opcode | width);
				write_uint8(state, parse_reg_source(state->sourceline, state->offsets.arg1, state));
				write_uint8(state, parse_reg_source(state->sourceline, state->offsets.arg2, state));
				break;

			// 2 args - One bitmask, one register
			case OPCODE_PUSH:
				if(!check_args(strsegment, state, true, 2)) return false;
				write_uint8(state, opcode | width);
				write_uint16(state, parse_reg_bitmask_source(state->sourceline, state->offsets.arg1, state));
				write_uint8(state, parse_reg_source(state->sourceline, state->offsets.arg2, state));	
				break;

			case OPCODE_POP:
				if(!check_args(strsegment, state, true, 2)) return false;
				write_uint8(state, opcode | width);
				write_uint16(state, parse_reg_bitmask_source(state->sourceline, state->offsets.arg2, state));
				write_uint8(state, parse_reg_source(state->sourceline, state->offsets.arg1, state));		
				break;		

			// 3 args - One immediate arg, two registers, has width
			case OPCODE_DIVI:
				if(!check_args(strsegment, state, true, 3)) return false;
				write_uint8(state, opcode | width);
				write_variable(state, parse_immediate_value_source(state->sourceline, state->offsets.arg1, width, state), width);
				write_uint8(state, parse_reg_source(state->sourceline, state->offsets.arg2, state));
				write_uint8(state, parse_reg_source(state->sourceline, state->offsets.arg3, state));
				break;

			// 3 args - 3 registers, has width
			case OPCODE_DIV:
				if(!check_args(strsegment, state, true, 3)) return false;
				write_uint8(state, opcode | width);
				write_uint8(state, parse_reg_source(state->sourceline, state->offsets.arg1, state));
				write_uint8(state, parse_reg_source(state->sourceline, state->offsets.arg2, state));
				write_uint8(state, parse_reg_source(state->sourceline, state->offsets.arg3, state));
				break;

			case OPCODE_DATA:
				// Special case, because we now re-parse the source line
				if(state->offsets.width.length == 0) {
					fprintf(state->error,"%s:%d:%d: error: 'data' expects width (.b, .w, .l, .x)\n", state->source_filename, state->line_number, state->offsets.opcode.end);
					width = 0;
				}
				write_data(state, width);
				break;
		}
	}

	return true;
}

bool check_args(char *opcode, state_t *state, bool haswidth, uint8_t args) {
	if(haswidth) {
		if(state->offsets.width.length == 0) {
			fprintf(state->error,"%s:%d:%d: error: '%s' expects width (.b, .w, .l, .x)\n", state->source_filename, state->line_number, state->offsets.opcode.end, opcode);
			// return false;
		}
	} else {
		if(state->offsets.width.length != 0) {
			fprintf(state->error,"%s:%d:%d: error: '%s' does not expect width\n", state->source_filename, state->line_number, state->offsets.opcode.end, opcode);
			// return false;
		}
	}

	if(args==0) {
		if(state->offsets.arg1.length != 0) {
			fprintf(state->error,"%s:%d:%d: error: '%s' takes no arguments\n", state->source_filename, state->line_number, state->offsets.opcode.end, opcode);
			// return false;
		}
	} else if(args==1) {
		if(state->offsets.arg1.length == 0) {
			fprintf(state->error,"%s:%d:%d: error: '%s' needs 1 argument\n", state->source_filename, state->line_number, state->offsets.arg1.end, opcode);
			// return false;
		}
		if(state->offsets.arg2.length != 0) {
			fprintf(state->error,"%s:%d:%d: error: '%s' takes only 1 argument\n", state->source_filename, state->line_number, state->offsets.arg1.end, opcode);
			// return false;
		}
	} else if(args == 2) {
		if(state->offsets.arg1.length == 0 || state->offsets.arg2.length == 0) {
			fprintf(state->error,"%s:%d:%d: error: '%s' needs 2 arguments\n", state->source_filename, state->line_number, state->offsets.arg1.end, opcode);
			// return false;
		}
		if(state->offsets.arg3.length != 0) {
			fprintf(state->error,"%s:%d:%d: error: '%s' takes only 2 arguments\n", state->source_filename, state->line_number, state->offsets.arg1.end, opcode);
			// return false;
		}
	} else if(args==3) {
		if(state->offsets.arg1.length == 0 || state->offsets.arg2.length == 0 || state->offsets.arg3.length == 0) {
			fprintf(state->error,"%s:%d:%d: error: '%s' needs 3 arguments\n", state->source_filename, state->line_number, state->offsets.arg1.end, opcode);
			// return false;
		}
	}

	return true;
} 

uint8_t parse_reg_source(char* sourceline, segment_t reg, state_t *state) {
	char regstr[1024];
	get_segment(regstr, sourceline, reg);
	return parse_register(regstr, state);
}

uint16_t parse_reg_bitmask_source(char* sourceline, segment_t reg, state_t *state) {
	char regstr[1024];
	get_segment(regstr, sourceline, reg);
	return parse_register_bitmask(regstr, state);
}

uint64_t parse_immediate_value_source(char* sourceline, segment_t reg, uint8_t width, state_t *state) {
	char regstr[1024];
	get_segment(regstr, sourceline, reg);
	return parse_immediate_value(regstr, width, state);
}

uint64_t parse_immediate_value_float_source(char* sourceline, segment_t reg, uint8_t width, state_t *state) {
	char regstr[1024];
	get_segment(regstr, sourceline, reg);
	return parse_immediate_float_value(regstr, width, state);
}

int32_t parse_label(char* sourceline, segment_t reg, state_t *state) {
	char label[1024];
	get_segment(label, sourceline, reg);

	stree_node_t *node = stree_get(state->labels, label);
	if(node==NULL) {
		// No existing label 

		// Is label defined in fwd_labels
		dll_t *poslist;
		node = stree_get(state->fwd_labels, label);
		if(node==NULL) {
			// Define forward Label and poslist
			poslist = dll_new();
			stree_set(state->fwd_labels, label, poslist);
		} else {
			poslist = (dll_t*)node->value;
		}

		// printf("Forward reference '%s' added pos %d\n", label, state->pos);

		forward_ref_t *ref = malloc(sizeof(forward_ref_t));
		ref->dest = state->dest;
		ref->line_number = state->line_number;
		ref->line_offset = reg.start;
		ref->dest_pos = state->pos;

		dll_push(poslist, ref);
		return 0xA5A5A5A5;
	} else {
		// Return offset
		uint32_t pos = (uint32_t)(uint64_t)node->value;
		// printf("Got Label '%s' pos %d offset %d\n", label, pos, (int32_t)pos - state->pos);
		return (int32_t)pos - state->pos - 8;
	}
}

void write_uint8(state_t *state, uint8_t value) {
	fwrite(&value,1,1,state->dest);
	state->pos++;
}

void write_uint16(state_t *state, uint16_t value) {
	value = htole16(value);
	fwrite(&value,2,1,state->dest);
	state->pos += 2;
}

void write_uint32(state_t *state, uint32_t value) {
	value = htole32(value);
	fwrite(&value,4,1,state->dest);
	state->pos += 4;
}

void write_uint64(state_t *state, uint64_t value) {
	value = htole64(value);
	fwrite(&value,8,1,state->dest);
	state->pos += 8;
}

void write_variable(state_t *state, uint64_t value, uint8_t width) {
	switch(width) {
		case WIDTH_8:
			write_uint8(state, value);
			break;
		case WIDTH_16:
			write_uint16(state, value);
			break;
		case WIDTH_32:
			write_uint32(state, value);
			break;
		case WIDTH_64:
			write_uint64(state, value);
			break;
	}
}

void write_buffer(state_t *state, void *buffer, uint32_t length) {
	fwrite(buffer,length,1,state->dest);
	state->pos += length;
}

void write_data(state_t *state, uint8_t width) {
	char *str = state->sourceline+state->offsets.arg1.start;

	// printf("'%c%c%c'\n", *(str-1),*str,*(str+1));

	uint8_t mode = 0; 
	char c;
	uint8_t i, pos;
	char buffer[1024];
	uint64_t data;

	while((c = *str++)!=0) {
		switch(mode) {
			case 0:
				if(whitespace(c)) break;
				if(c=='"') {
					mode=1;
				} else {
					mode = 4;
					pos = 0;
					buffer[pos++]='#';
					buffer[pos++]=c;
				}
				break;
			case 1:
				switch(c) {
					case '\\':
						mode = 2;
						break;
					case '"':
						mode = 0;
						break;
					default:
						write_variable(state, c, width);
						break;
				}
				break;
			case 2:
				switch(c) {
					case 'r':
						write_variable(state, 13, width);
						break;
					case 'n':
						write_variable(state, 10, width);
						break;
					case 't':
						write_variable(state, 9, width);
						break;
					case '"':
						write_variable(state, '"', width);
						break;
				}
				mode = 1;
				break;
			case 4:
				if(whitespace(c) || c == 0) {
					buffer[pos++]=0;
					data = parse_immediate_value(buffer, width, state);
					write_variable(state, data, width);
					// printf("Got data %02x from %s\n", data, buffer);
					mode = 0;
					break;
				}
				buffer[pos++]=c;
				break;
		}
	}

	if(mode == 4) {
		buffer[pos++]=0;
		data = parse_immediate_value(buffer, width, state);
		write_variable(state, data, width);
		// printf("Got data %02x from %s\n", data, buffer);
		mode = 0;
	}
}







