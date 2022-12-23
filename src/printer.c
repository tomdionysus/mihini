#include "printer.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

char print_width(uint8_t width) {
	switch(width) {
		case WIDTH_8: return 'b';
		case WIDTH_16: return 'w';
		case WIDTH_32: return 'l';
		case WIDTH_64: return 'x';
		default:
			fprintf(stderr,"print_width: unknown width %d\n", width);
			return 0;
	}
}

void print_register(uint8_t reg, char *buffer) {
	// (0ddd arrr)
	uint8_t pos = 0;
	uint8_t num = reg & 0b111;
	bool a = (reg & 0b1000) != 0;
	uint8_t ref = (reg & 0b1110000) >> 4;

	switch(ref) {
		case 1:
		case 3:
			buffer[pos++] = '(';
			break;
		case 2:
			buffer[pos++] = '-';
			buffer[pos++] = '(';
			break;
	}

	buffer[pos++] = a ? 'a' : 'd';
	buffer[pos++] = '0'+num;

	switch(ref) {
		case 1:
		case 2:
			buffer[pos++] = ')';
			break;
		case 3:
			buffer[pos++] = ')';
			buffer[pos++] = '+';
			break;
	}

	buffer[pos++] = 0x00;
}

void print_register_bitmask(uint16_t mask, char *buffer) {
	uint8_t i, from, pos = 0, run = 0;
	bool first = true, current;

	// This, frankly, was a c*nt to write.

	if((mask & 0xFF) != 0) {
		for(i=0; i<8; i++) {
			current = ((mask) & (1 << (i))) != 0;
			if(current) {
				if(run == 0) {
					if(!first) buffer[pos++] = ',';
					buffer[pos++] = 'd';
					buffer[pos++] = '0'+i;
				}
				from = i;
				run++;
				first = false;
			} else {
				if(run > 1) {
					buffer[pos++] = '-';
					buffer[pos++] = 'd';
					buffer[pos++] = '0'+from;
				}
				run = 0;
			}
		}

		if(run > 1) {
			buffer[pos++] = '-';
			buffer[pos++] = 'd';
			buffer[pos++] = '0'+from;
		}
	}

	if(((mask & 0xFF) != 0) && ((mask & 0xFF00) != 0)) {
		buffer[pos++] = '/';
	} 

	first = true; from = false; run = 0;

	if((mask & 0xFF00) != 0) {
		for(i=0; i<8; i++) {
			current = ((mask) & (0x100 << (i))) != 0;
			if(current) {
				if(run == 0) {
					if(!first) buffer[pos++] = ',';
					buffer[pos++] = 'a';
					buffer[pos++] = '0'+i;
				}
				from = i;
				run++;
				first = false;
			} else {
				if(run > 1) {
					buffer[pos++] = '-';
					buffer[pos++] = 'a';
					buffer[pos++] = '0'+from;
				}
				run = 0;
			}
		}

		if(run > 1) {
			buffer[pos++] = '-';
			buffer[pos++] = 'a';
			buffer[pos++] = '0'+from;
		}	
	}

	buffer[pos++] = 0x00;

	return;
}

void print_immediate_value(uint64_t value, char *buffer) {
	sprintf(buffer, "#0x%llx", value);
}

void print_immediate_float_value(uint64_t value, char *buffer) {
	// Unpack a double from the uint64 
	double d = *(double*)(&value);

	sprintf(buffer, "#%f", d);
}
