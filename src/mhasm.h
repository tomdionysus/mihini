#ifndef MHASM
#define MHASM

#include <stddef.h>

#include "dll.h"
#include "stree.h"

// A segment of a source line
typedef struct segment {
	uint16_t start;				// Start offset for line
	uint16_t length;			// Length of segment
	uint16_t end;				// End offset of segment
} segment_t;

// A source line parsed into segments
typedef struct source_line {
	segment_t label;			// The label (start, end, loop)
	segment_t opcode;			// The opcode (xor, add, bra)
	segment_t width;			// The width (b, w, l, x)
	segment_t arg1;				// First argument
	segment_t arg2;				// Second argument
	segment_t arg3;				// Third argument
	segment_t comment;			// Comment
} source_line_t;

// General statebag of the assembly process
typedef struct state {
	char* source_filename;		// The source assembly file name
	char* dest_filename;		// The destination mihini file name
	FILE *source;				// Source asm stdio file handle
	FILE *dest;					// Dest obj file handle
	FILE *error;				// Error print file handle

	char sourceline[1024];		// Current source line
	source_line_t offsets;		// Offsets of tokens in current source line
	uint32_t line_number;		// Current source line number
	
	uint32_t pos;				// Current dest position 
	stree_node_t *labels;		// Previous Labels so far (stree maps label -> pos)
	stree_node_t *fwd_labels;	// Forward Labels (stree maps label -> dll of positions)

	stree_node_t *opcodes;		// All mihini opcodes (lookup)
} state_t;

// A forward Reference 
typedef struct forward_ref {
	uint32_t line_number;		// Source file line number
	uint32_t line_offset;		// Source file line offset
	uint32_t dest_pos;			// The position to write the offset to in the dest file

	uint32_t label_pos;			// The position of the label once defined
	FILE *dest;					// The destination file
} forward_ref_t;

int main(int argc, char *argv[]);

bool fix_fwd_labels_iter_2(void *value, void *context);
bool fix_fwd_labels_iter(char *key, void *value, void *context);
bool free_fwd_labels_iter(char *key, void *value, void *context);

void str_trim_right(char *buffer);

bool parse(char *buffer, source_line_t *offsets);

uint8_t next_segment(char *buffer, uint8_t pos, segment_t *out);
void get_segment(char *dest, char *src, segment_t segment);

bool assemble(state_t *state);

uint8_t parse_reg_source(char* sourceline, segment_t reg, state_t *state);
uint16_t parse_reg_bitmask_source(char* sourceline, segment_t reg, state_t *state);
uint64_t parse_immediate_value_source(char* sourceline, segment_t reg, uint8_t width, state_t *state);
uint64_t parse_immediate_value_float_source(char* sourceline, segment_t reg, uint8_t width, state_t *state);
int32_t parse_label(char* sourceline, segment_t reg, state_t *state);

bool check_args(char *opcode, state_t *state, bool haswidth, uint8_t args);

void write_uint8(state_t *state, uint8_t value);
void write_uint16(state_t *state, uint16_t value);
void write_uint32(state_t *state, uint32_t value);
void write_uint64(state_t *state, uint64_t value);
void write_variable(state_t *state, uint64_t value, uint8_t width);
void write_data(state_t *state, uint8_t width);
void write_buffer(state_t *state, void *buffer, uint32_t length);

#endif