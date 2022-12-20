#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "btree.h"
#include "dll.h"
#include "stree.h"
#include "pmalloc.h"
#include "parser.h"
#include "printer.h"

void test_printer() {
	char x;

	if((x = print_width(0b00))!='b') printf("test_printer: print_width(0b00) should return 'b' (%c)\n", x);
	if((x = print_width(0b01))!='w') printf("test_printer: print_width(0b01) should return 'w' (%c)\n", x);
	if((x = print_width(0b10))!='l') printf("test_printer: print_width(0b10) should return 'l' (%c)\n", x);
	if((x = print_width(0b11))!='x') printf("test_printer: print_width(0b11) should return 'x' (%c)\n", x);

	char buffer[1024] = {};

	print_register(0b00000110, buffer);
	if(strcmp(buffer,"d6")) printf("test_printer: print_register(0b00000110) should return 'd6' (%s)\n", buffer);
	print_register(0b00011010, buffer);
	if(strcmp(buffer,"(a2)")) printf("test_printer: print_register(0b00011010) should return '(a2)' (%s)\n", buffer);
	print_register(0b00101010, buffer);
	if(strcmp(buffer,"-(a2)")) printf("test_printer: print_register(0b00101010) should return '-(a2)' (%s)\n", buffer);
	print_register(0b00111010, buffer);
	if(strcmp(buffer,"(a2)+")) printf("test_printer: print_register(0b00111010) should return '(a2)+' (%s)\n", buffer);
	print_register(0b00111111, buffer);
	if(strcmp(buffer,"(a7)+")) printf("test_printer: print_register(0b00111111) should return '(a2)+' (%s)\n", buffer);

	print_register_bitmask(0b0111111111111111, buffer);
	if(strcmp(buffer,"d0-d7/a0-a6")) printf("test_printer: print_register_bitmask(0b0111111111111111) should return 'd0-d7/a0-a6' (%s)\n", buffer);
	print_register_bitmask(0b0000000000000001, buffer);
	if(strcmp(buffer,"d0")) printf("test_printer: print_register_bitmask(0b0000000000000001) should return 'd0' (%s)\n", buffer);
	print_register_bitmask(0b0000000000000101, buffer);
	if(strcmp(buffer,"d0,d2")) printf("test_printer: print_register_bitmask(0b0000000000000101) should return 'd0,d2' (%s)\n", buffer);
	print_register_bitmask(0b0000000011001111, buffer);
	if(strcmp(buffer,"d0-d3,d6-d7")) printf("test_printer: print_register_bitmask(0b0000000000000101) should return 'd0-d3,d6-d7' (%s)\n", buffer);
	print_register_bitmask(0b1010101001010101, buffer);
	if(strcmp(buffer,"d0,d2,d4,d6/a1,a3,a5,a7")) printf("test_printer: print_register_bitmask(0b0000000000000101) should return 'd0,d2,d4,d6/a1,a3,a5,a7' (%s)\n", buffer);

	print_immediate_value(5612, buffer);
	if(strcmp(buffer,"#0x15ec")) printf("test_printer: print_immediate_value(5612) should return '#0x15ec' (%s)\n", buffer);
	print_immediate_value(0b1111, buffer);
	if(strcmp(buffer,"#0xf")) printf("test_printer: print_immediate_value(0b1111) should return '#0xf' (%s)\n", buffer);
	print_immediate_value(0x2347, buffer);
	if(strcmp(buffer,"#0x2347")) printf("test_printer: print_immediate_value(5612) should return '#0x2347' (%s)\n", buffer);

	printf(".");
}

void test_btree() {
	btree_node_t *bt;


	btree_add(&bt, 1, "one");
	btree_add(&bt, 2, "two");
	btree_add(&bt, 3, "three");
	btree_add(&bt, 4, "four");
	btree_add(&bt, 5, "five");
	btree_add(&bt, 7, "seven");
	btree_add(&bt, 8, "eight");
	btree_add(&bt, 9, "nine");
	btree_add(&bt, 10, "ten");
	btree_add(&bt, 11, "eleven");
	btree_add(&bt, 12, "twelve");

	btree_balance(&bt);

	btree_node_t *node;

	node = btree_get(bt, 0);
	if(node != NULL) printf("test_btree: 0 found when it doesn't exist\n");
	
	node = btree_get(bt, 1);
	if(node == NULL || strcmp("one",(char*)node->value) != 0) printf("test_btree: 1 not found or has incorrect value %s\n", (char*)node->value);
	node = btree_get(bt, 2);
	if(node == NULL || strcmp("two",(char*)node->value) != 0) printf("test_btree: 2 not found or has incorrect value %s\n", (char*)node->value);
	node = btree_get(bt, 3);
	if(node == NULL || strcmp("three",(char*)node->value) != 0) printf("test_btree: 3 not found or has incorrect value %s\n", (char*)node->value);
	node = btree_get(bt, 4);
	if(node == NULL || strcmp("four",(char*)node->value) != 0) printf("test_btree: 4 not found or has incorrect value %s\n", (char*)node->value);
	node = btree_get(bt, 5);
	if(node == NULL || strcmp("five",(char*)node->value) != 0) printf("test_btree: 5 not found or has incorrect value %s\n", (char*)node->value);

	node = btree_get(bt, 6);
	if(node != NULL) printf("test_btree: 6 found when it doesn't exist\n");

	node = btree_get(bt, 7);
	if(node == NULL || strcmp("seven",(char*)node->value) != 0) printf("test_btree: 7 not found or has incorrect value %s\n", (char*)node->value);
	node = btree_get(bt, 8);
	if(node == NULL || strcmp("eight",(char*)node->value) != 0) printf("test_btree: 8 not found or has incorrect value %s\n", (char*)node->value);
	node = btree_get(bt, 9);
	if(node == NULL || strcmp("nine",(char*)node->value) != 0) printf("test_btree: 9 not found or has incorrect value %s\n", (char*)node->value);
	node = btree_get(bt, 10);
	if(node == NULL || strcmp("ten",(char*)node->value) != 0) printf("test_btree: 10 not found or has incorrect value %s\n", (char*)node->value);
	node = btree_get(bt, 11);
	if(node == NULL || strcmp("eleven",(char*)node->value) != 0) printf("test_btree: 11 not found or has incorrect value %s\n", (char*)node->value);
	node = btree_get(bt, 12);
	if(node == NULL || strcmp("twelve",(char*)node->value) != 0) printf("test_btree: 12 not found or has incorrect value %s\n", (char*)node->value);

	node = btree_get(bt, 13);
	if(node != NULL) printf("test_btree: 13 found when it doesn't exist\n");

	btree_free(&bt);

	if(bt != NULL) printf("test_btree: btree_free does not clear pointer\n");

	printf(".");
}

void test_dll() {
	dll_t list = {};

	void *x;

	// Push/Pop
	if((x = dll_pop(&list)) != NULL) printf("test_dll: dll_pop did not return NULL for empty list (%02llx)\n", (uint64_t)x);
	if((x = dll_shift(&list)) != NULL) printf("test_dll: dll_shift did not return NULL for empty list (%02llx)\n", (uint64_t)x);

	if(list.count != 0) printf("test_dll: count should be 0 (%d)\n", list.count);
	dll_push(&list,(void*)6);
	if(list.count != 1) printf("test_dll: count should be 1 (%d)\n", list.count);
	dll_push(&list,(void*)20);
	if(list.count != 2) printf("test_dll: count should be 2 (%d)\n", list.count);
	dll_push(&list,"foo");
	if(list.count != 3) printf("test_dll: count should be 3 (%d)\n", list.count);

	x = dll_pop(&list);
	if(strcmp(x,"foo")!=0) printf("test_dll: dll_pop did not return 'foo' for element 2 (%02llx)\n", (uint64_t)x);
	if(list.count != 2) printf("test_dll: count should be 2 (%d)\n", list.count);
	
	if((x = dll_pop(&list)) != (void*)20) printf("test_dll: dll_pop did not return 20 for element 1 (%02llx)\n", (uint64_t)x);
	if(list.count != 1) printf("test_dll: count should be 1 (%d)\n", list.count);
	
	if((x = dll_pop(&list)) != (void*)6) printf("test_dll: dll_pop did not return 6 for element 0 (%02llx)\n", (uint64_t)x);
	if(list.count != 0) printf("test_dll: count should be 0 (%d)\n", list.count);
	
	if((x = dll_pop(&list)) != NULL) printf("test_dll: dll_pop did not return NULL for empty list (%02llx)\n", (uint64_t)x);

	// Unshift/Shift
	dll_unshift(&list,(void*)6);
	dll_unshift(&list,(void*)20);
	dll_unshift(&list,"foo");

	x = dll_shift(&list);
	if(strcmp(x,"foo")!=0) printf("test_dll: dll_shift did not return 'foo' for element 2 (%02llx)\n", (uint64_t)x);
	if((x = dll_shift(&list)) != (void*)20) printf("test_dll: dll_shift did not return 20 for element 1 (%02llx)\n", (uint64_t)x);
	if((x = dll_shift(&list)) != (void*)6) printf("test_dll: dll_shift did not return 6 for element 0 (%02llx)\n", (uint64_t)x);
	if((x = dll_shift(&list)) != NULL) printf("test_dll: dll_shift did not return NULL for empty list (%02llx)\n", (uint64_t)x);

	// Push/Shift
	dll_push(&list,(void*)6);
	dll_push(&list,(void*)20);
	dll_push(&list,"foo");

	if((x = dll_shift(&list)) != (void*)6) printf("test_dll: dll_shift did not return 6 for element 0 (%02llx)\n", (uint64_t)x);
	if((x = dll_shift(&list)) != (void*)20) printf("test_dll: dll_shift did not return 20 for element 1 (%02llx)\n", (uint64_t)x);
	x = dll_shift(&list);
	if(strcmp(x,"foo")!=0) printf("test_dll: dll_shift did not return 'foo' for element 2 (%02llx)\n", (uint64_t)x);
	if((x = dll_shift(&list)) != NULL) printf("test_dll: dll_shift did not return NULL for empty list (%02llx)\n", (uint64_t)x);

	dll_free(&list);

	printf(".");
}

void test_parser() {

	uint16_t x;

	state_t *state = calloc(sizeof(state_t),1);
	state->error = fopen("/dev/null", "w");

	// Parse Width
	if((x = parse_width('b', state) != 0b00)) printf("test_parser: parse_width did not return 0b00 for .b (%02x)\n", x);
	if((x = parse_width('w', state) != 0b01)) printf("test_parser: parse_width did not return 0b01 for .w (%02x)\n", x);
	if((x = parse_width('l', state) != 0b10)) printf("test_parser: parse_width did not return 0b10 for .l (%02x)\n", x);
	if((x = parse_width('x', state) != 0b11)) printf("test_parser: parse_width did not return 0b11 for .x (%02x)\n", x);

	printf(".");
	
	// Parse Register

	// Direct
	if((x = parse_register("d0", state)) != 0b00000000) printf("test_parser: parse_register should return 0b00000000 for d0 (%02x)\n", x);
	if((x = parse_register("d1", state)) != 0b00000001) printf("test_parser: parse_register should return 0b00000001 for d1 (%02x)\n", x);
	if((x = parse_register("d2", state)) != 0b00000010) printf("test_parser: parse_register should return 0b00000010 for d2 (%02x)\n", x);
	if((x = parse_register("d3", state)) != 0b00000011) printf("test_parser: parse_register should return 0b00000011 for d3 (%02x)\n", x);
	if((x = parse_register("d4", state)) != 0b00000100) printf("test_parser: parse_register should return 0b00000100 for d4 (%02x)\n", x);
	if((x = parse_register("d5", state)) != 0b00000101) printf("test_parser: parse_register should return 0b00000101 for d5 (%02x)\n", x);
	if((x = parse_register("d6", state)) != 0b00000110) printf("test_parser: parse_register should return 0b00000110 for d6 (%02x)\n", x);
	if((x = parse_register("d7", state)) != 0b00000111) printf("test_parser: parse_register should return 0b00000111 for d7 (%02x)\n", x);
	if((x = parse_register("a0", state)) != 0b00001000) printf("test_parser: parse_register should return 0b00001000 for a0 (%02x)\n", x);
	if((x = parse_register("a1", state)) != 0b00001001) printf("test_parser: parse_register should return 0b00001001 for a1 (%02x)\n", x);
	if((x = parse_register("a2", state)) != 0b00001010) printf("test_parser: parse_register should return 0b00001010 for a2 (%02x)\n", x);
	if((x = parse_register("a3", state)) != 0b00001011) printf("test_parser: parse_register should return 0b00001011 for a3 (%02x)\n", x);
	if((x = parse_register("a4", state)) != 0b00001100) printf("test_parser: parse_register should return 0b00001100 for a4 (%02x)\n", x);
	if((x = parse_register("a5", state)) != 0b00001101) printf("test_parser: parse_register should return 0b00001101 for a5 (%02x)\n", x);
	if((x = parse_register("a6", state)) != 0b00001110) printf("test_parser: parse_register should return 0b00001110 for a6 (%02x)\n", x);
	if((x = parse_register("a7", state)) != 0b00001111) printf("test_parser: parse_register should return 0b00001111 for a7 (%02x)\n", x);

	// Reference
	if((x = parse_register("(a7)",NULL)) != 0b00011111) printf("test_parser: parse_register should return 0b00011111 for (a7) (%02x)\n", x);
	if((x = parse_register("(d3)",NULL)) != 0b00010011) printf("test_parser: parse_register should return 0b00010011 for (d3) (%02x)\n", x);

	// Predecrement reference
	if((x = parse_register("-(a7)",NULL)) != 0b00101111) printf("test_parser: parse_register should return 0b00101111 for -(a7) (%02x)\n", x);

	// Postincrement reference
	if((x = parse_register("(a3)+",NULL)) != 0b00111011) printf("test_parser: parse_register should return 0b00111011 for (a3)+ (%02x)\n", x);
	if((x = parse_register("(a7)+",NULL)) != 0b00111111) printf("test_parser: parse_register should return 0b00111111 for (a7)+ (%02x)\n", x);
	
	// Malformed
	if((x = parse_register("(a3", state)) != 0xFF) printf("test_parser: parse_register should return 0xFF for (a3 (%02x)\n", x);

	printf(".");

	// Parse Register bitmask

	if((x = parse_register_bitmask("d0", state)) != 0b00000001) printf("test_parser: parse_register_bitmask should return 0x00000001 for d0 (%02x)\n", x);
	if((x = parse_register_bitmask("d0,d1,d2", state)) != 0b00000111) printf("test_parser: parse_register_bitmask should return 0b00000111 for d0,d1,d2 (%02x)\n", x);
	if((x = parse_register_bitmask("d0/d1/d2", state)) != 0b00000111) printf("test_parser: parse_register_bitmask should return 0b00000111 for d0,d1,d2 (%02x)\n", x);
	if((x = parse_register_bitmask("d0-d2", state)) != 0b00000111) printf("test_parser: parse_register_bitmask should return 0b00000111 for d0-d2 (%02x)\n", x);
	if((x = parse_register_bitmask("d5-d7", state)) != 0b11100000) printf("test_parser: parse_register_bitmask should return 0b11100000 for d5-d7 (%02x)\n", x);
	if((x = parse_register_bitmask("d7-d2", state)) != 0b11111100) printf("test_parser: parse_register_bitmask should return 0b11111100 for d7-d2 (%02x)\n", x);
	if((x = parse_register_bitmask("d0-a2", state)) != 0xFF) printf("test_parser: parse_register_bitmask should return 0xFF for d0-a2 (%02x)\n", x);
	if((x = parse_register_bitmask("d0-d2,d7", state)) != 0b10000111) printf("test_parser: parse_register_bitmask should return 0b10000111 for d0-d2,d7 (%02x)\n", x);
	if((x = parse_register_bitmask("d0-d2/d7", state)) != 0b10000111) printf("test_parser: parse_register_bitmask should return 0b10000111 for d0-d2/d7 (%02x)\n", x);

	if((x = parse_register_bitmask("a0", state)) != 0b100000000) printf("test_parser: parse_register_bitmask should return 0b100000000 for a0 (%02x)\n", x);
	if((x = parse_register_bitmask("a0,a1,a2", state)) != 0b0000011100000000) printf("test_parser: parse_register_bitmask should return 0b0000011100000000 for a0,a1,a2 (%02x)\n", x);
	if((x = parse_register_bitmask("a0/a1/a2", state)) != 0b0000011100000000) printf("test_parser: parse_register_bitmask should return 0b0000011100000000 for a0,a1,a2 (%02x)\n", x);
	if((x = parse_register_bitmask("a0-a2", state)) != 0b0000011100000000) printf("test_parser: parse_register_bitmask should return 0b0000011100000000 for a0-a2 (%02x)\n", x);
	if((x = parse_register_bitmask("a5-a7", state)) != 0b1110000000000000) printf("test_parser: parse_register_bitmask should return 0b1110000000000000 for a5-a7 (%02x)\n", x);
	if((x = parse_register_bitmask("a7-a2", state)) != 0b1111110000000000) printf("test_parser: parse_register_bitmask should return 0b1111110000000000 for a7-a2 (%02x)\n", x);
	if((x = parse_register_bitmask("a0-d2", state)) != 0xFF) printf("test_parser: parse_register_bitmask should return 0xFF for a0-d2 (%02x)\n", x);
	if((x = parse_register_bitmask("a0-a2,a7", state)) != 0b1000011100000000) printf("test_parser: parse_register_bitmask should return 0b1000011100000000 for a0-a2,a7 (%02x)\n", x);
	if((x = parse_register_bitmask("a0-a2/a7", state)) != 0b1000011100000000) printf("test_parser: parse_register_bitmask should return 0b1000011100000000 for a0-a2/a7 (%02x)\n", x);

	if((x = parse_register_bitmask("d0-d7/a0-a6", state)) != 0b0111111111111111) printf("test_parser: parse_register_bitmask should return 0b1111111111111 for d0-d7/a0-a6 (%02x)\n", x);
	if((x = parse_register_bitmask("a0,a1,a2/d0-d2", state)) != 0b0000011100000111) printf("test_parser: parse_register_bitmask should return 0b0000011100000111 for a0,a1,a2/d0-d2 (%02x)\n", x);

	printf(".");

	// Parse Immediate Value

	uint64_t lx;

	if((lx = parse_immediate_value("#2",1, state)) != 2) printf("test_parser: parse_immediate_value should return 0x02 for '#2' (0x%02llx)\n", lx);
	if((lx = parse_immediate_value("#21381238",1, state)) != 0x1464076) printf("test_parser: parse_immediate_value should return 0x1464076 for '#21381238' (0x%02llx)\n", lx);
	if((lx = parse_immediate_value("#0b1010101",1, state)) != 0x55) printf("test_parser: parse_immediate_value should return 0x02 for '#0b1010101' (0x%02llx)\n", lx);
	if((lx = parse_immediate_value("#0x21ff",1 , state)) != 0x21ff)printf("test_parser: parse_immediate_value should return 0x21ff for '#0x21ff' (0x%02llx)\n", lx);
	if((lx = parse_immediate_value("#'*'",1 , state)) != 42)printf("test_parser: parse_immediate_value should return 42 for \"#'*'\" (0x%02llx)\n", lx);
	if((lx = parse_immediate_value("#'\\n'",1 , state)) != 10)printf("test_parser: parse_immediate_value should return 10 for \"#'n'\"  (0x%02llx)\n", lx);
	if((lx = parse_immediate_value("#'\\r'",1 , state)) != 13)printf("test_parser: parse_immediate_value should return 13 for \"#'r'\"  (0x%02llx)\n", lx);
	if((lx = parse_immediate_value("#'\\t'",1 , state)) != 9)printf("test_parser: parse_immediate_value should return 09 for \"#'t'\"  (0x%02llx)\n", lx);
	if((lx = parse_immediate_value("#'\\\\'",1 , state)) != 0x5c)printf("test_parser: parse_immediate_value should return 0x5c for \"#'\\'\"  (0x%02llx)\n", lx);

	fclose(state->error);
	free(state);

	printf(".");
}

void test_stree() {
	stree_node_t *root = stree_new('-');

	stree_set(root, "twenty", (void*)20);
	stree_set(root, "two", (void*)2);
	stree_set(root, "thr", (void*)300);
	stree_set(root, "three", (void*)3);
	stree_set(root, "four", (void*)4);

	stree_balance(&root);

	stree_node_t *node;

	node = stree_get(root, "zero");
	if(node!=NULL) printf("test_stree: zero found when it should not have been\n");

	node = stree_get(root, "one");
	if(node!=NULL) printf("test_stree: one found when it should not have been\n");

	node = stree_get(root, "two");
	if(node==NULL || (uint64_t)node->value != 2) printf("test_stree: two not found when it should have been\n");

	node = stree_get(root, "three");
	if(node==NULL || (uint64_t)node->value != 3) printf("test_stree: three not found when it should have been\n");

	node = stree_get(root, "thr");
	if(node==NULL || (uint64_t)node->value != 300) printf("test_stree: thr not found when it should have been\n");

	node = stree_get(root, "four");
	if(node==NULL || (uint64_t)node->value != 4) printf("test_stree: four not found when it should have been\n");

	node = stree_get(root, "five");
	if(node!=NULL) printf("test_stree: five found when it should not have been\n");

	node = stree_get(root, "twenty");
	if(node==NULL) {
		if ((uint64_t)node->value != 20) printf("test_stree: twenty not found when it should have been\n");
		if(strcmp("twenty", node->key) !=0) printf("test_stree: twenty key is not 'twenty'\n");
	}

	printf(".");
}

void test_malloc() {

	char buffer[65536];

	paddblock(&buffer, 65536);

	uint32_t len[6] = { 150, 256, 512, 100, 1024, 65536 };
	void* mem[6];

	for(uint32_t i = 0; i<6; i++) mem[i] = pmalloc(len[i]);

	if(mem[5]!=NULL) printf("test_malloc: pmalloc(65536) allocated when it should not have\n");

	for(uint8_t i = 0; i<5; i++) {
		if(len[i]!=psizeof(mem[i])) printf("test_malloc: psizeof incorrectly reports size %d for block %d\n", psizeof(mem[i]), i);
	}

	for(uint32_t i = 0; i<5; i++) pfree(mem[i]);

	printf(".");
}

int main() {
	test_stree();
	test_malloc();
	test_parser();
	test_dll();
	test_btree();
	test_printer();

	printf("\nDone\n");
	return 0;
}
