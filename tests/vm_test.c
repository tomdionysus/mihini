#include "vm.h"
#include <stdio.h>

void test_opcode_def() {
	opcode op;
	op.num = 0b11111101;

	if(op.ww.num != 0b111111) printf("test_opcode_def: op.w.num != 0b111111 (%2x)\n", op.ww.num);
	if(op.ww.width != 0b01) printf("test_opcode_def: op.w.width != 0b01 (%02x)\n", op.ww.width);
	if(!op.sys) printf("test_opcode_def: op.sys != TRUE (%02x)\n", op.sys);

	printf(".");
}

int main() {
	test_opcode_def();
	printf("\nDone\n");
	return 0;
}




