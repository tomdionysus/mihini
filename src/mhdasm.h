#include <stddef.h>

#include "dll.h"
#include "stree.h"
#include "btree.h"

// Version
#define MIHINI_VERSION 0x0001

// General statebag of the disassembly process
typedef struct state {
	char* source_filename;		// The source mihini file name
	char* dest_filename;		// The destination assembly file name
	FILE *source;				// Source asm stdio file handle
	FILE *dest;					// Dest obj file handle

	uint16_t source_version;	// The version of the mihini file

	uint32_t pos;				// Current source position
	uint32_t line_number;		// Current destination line number

	uint32_t label_num;			// Label seed number

	btree_node_t *labels;		// Labels discovered during first pass

	btree_node_t *opcodes;		// The opcode to string lookup
} state_t;

uint32_t cleanup(state_t *state);

bool first_pass(uint8_t opcode, state_t* state);
bool disassemble(uint8_t opcode, state_t* state);

uint8_t read_uint8(state_t *state);
uint16_t read_uint16(state_t *state);
uint32_t read_uint32(state_t *state);
uint64_t read_uint64(state_t *state);
uint64_t read_variable(state_t *state, uint8_t width);
size_t read_buffer(state_t *state, void *buffer, uint32_t length);

