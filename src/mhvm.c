#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <regex.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>

#include "mhvm.h"
#include "endian_shim.h"
#include "vm.h"

int main(int argc, char *argv[]) {
	// printf("Mihini Disassembler v0.1\n");
	if(argc != 2) {
      fprintf(stderr, "Usage: %s source.mihini\n", argv[0]);
		return -1;
	}

	vmstate_t state;

	// Initialise with 1Mb of memory, 0 of boot space, and 4Kb of stack.
	vm_init(&state, 1024*1024, 0, 4096);

	FILE *source = fopen(argv[1] ,"r");

	// Can't open file
	if(source == NULL) {
      fprintf(stderr, "Error opening source file %s\n", argv[1]);
      fclose(source);
      vm_shutdown(&state);
      return -1;
	}

	// Get Size
	fseek(source, 0L, SEEK_END);
	uint32_t sz = ftell(source);
	fseek(source, 0L, SEEK_SET);

	char buffer[10] = {};

	fread(buffer, 6, 1, source);
	buffer[6]=0;

	if(strcmp("mihini", buffer)!=0) {
		fprintf(stderr, "error: source file '%s' has an incorrect header (mihini<version>)\n", argv[1]);
		fclose(source);
		vm_shutdown(&state);
		return -1;
	}

	fread(buffer, 4, 1, source);
	uint32_t version = *(uint32_t*)buffer;

	if (version > MIHINI_VERSION) {
		fprintf(stderr, "error: source file '%s' is version %d, this mihini version %d is too old.\n", argv[1], version, MIHINI_VERSION);
		fclose(source);
		vm_shutdown(&state);
		return -1;
	}

	uint8_t *program = pmalloc(sz-10);
	fread(program, sz-10, 1, source); 
	fclose(source);

	// Execute
	vm_run(program - state.memory, &state);

	vm_shutdown(&state);
}