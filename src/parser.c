#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "parser.h"

uint8_t parse_width(char width, state_t *state) {
	switch(width) {
		case 'b': return WIDTH_8;
		case 'w': return WIDTH_16;
		case 'l': return WIDTH_32;
		case 'x': return WIDTH_64;
		default:
			fprintf(state->error,"%s:%d:%d: error: unknown width '.%c'\n", state->source_filename, state->line_number, state->offsets.opcode.end+1, width); 
			return WIDTH_UNKNOWN;
	}
}

uint8_t parse_register(char *regstr, state_t *state) {
	// 0dddarrr

	uint8_t len = strlen(regstr);

	// Direct
	if(len==2) {
		if(regstr[0]=='d') {
			// Direct D
			if(regstr[1]-'0'>7) fprintf(state->error,"%s:%d:%d: error: cannot parse register '%s'\n", state->source_filename, state->line_number, state->offsets.arg1.start, regstr); 
			return regstr[1]-'0';
		}
		if(regstr[0]=='a') {
			// Direct A
			if(regstr[1]-'0'>7) fprintf(state->error,"%s:%d:%d: error: cannot parse register '%s'\n", state->source_filename, state->line_number, state->offsets.arg1.start, regstr);
			return 0b1000 | (regstr[1]-'0');
		}
	}

	// References
	if(len==4 && regstr[0]=='(' && regstr[3]==')') {
		if(regstr[1]=='d') {
			// Direct D
			if(regstr[2]-'0'>7) fprintf(state->error,"%s:%d:%d: error: cannot parse register '%s'\n", state->source_filename, state->line_number, state->offsets.arg1.start, regstr);
			return 0b10000 | regstr[2]-'0';
		}
		if(regstr[1]=='a') {
			// Direct A
			if(regstr[2]-'0'>7) fprintf(state->error,"%s:%d:%d: error: cannot parse register '%s'\n", state->source_filename, state->line_number, state->offsets.arg1.start, regstr);
			return 0b11000 | (regstr[2]-'0');
		}
	}

	// Predecrement References
	if(len==5) {
		if(regstr[0]=='-' && regstr[1]=='(' && regstr[4]==')') {
			if(regstr[2]=='d') {
				// Direct D
				if(regstr[3]-'0'>7) fprintf(state->error,"%s:%d:%d: error: cannot parse register '%s'\n", state->source_filename, state->line_number, state->offsets.arg1.start, regstr);
				return 0b100000 | regstr[3]-'0';
			}
			if(regstr[2]=='a') {
				// Direct A
				if(regstr[3]-'0'>7) fprintf(state->error,"%s:%d:%d: error: cannot parse register '%s'\n", state->source_filename, state->line_number, state->offsets.arg1.start, regstr);
				return 0b101000 | (regstr[3]-'0');
			}
		}

		// Postincrement
		if(regstr[0]=='(' && regstr[3]==')' && regstr[4]=='+') {
			if(regstr[1]=='d') {
				// Direct D
				if(regstr[2]-'0'>7) fprintf(state->error,"%s:%d:%d: error: cannot parse register '%s'\n", state->source_filename, state->line_number, state->offsets.arg1.start, regstr);
				return 0b110000 | regstr[2]-'0';
			}
			if(regstr[1]=='a') {
				// Direct A
				if(regstr[2]-'0'>7) fprintf(state->error,"%s:%d:%d: error: cannot parse register '%s'\n", state->source_filename, state->line_number, state->offsets.arg1.start, regstr);
				return 0b111000 | (regstr[2]-'0');
			}
		}
	}

	fprintf(state->error,"%s:%d:%d: error: cannot parse register '%s'\n", state->source_filename, state->line_number, state->offsets.arg1.start, regstr); 

	return 0xFF;
}

uint16_t parse_register_bitmask(char *regstr, state_t *state) {
	uint16_t out = 0;
	uint8_t last = 0xFF, to;
	char lastr = 'x';

	char* original = regstr;

	while(*regstr != 0x00) {
		switch(*regstr++) {
			case 'd':
				last = *(regstr++)-'0';
				if(last<0 || last>7) return 0xFF;
				lastr = 'd';
				out |= (1 << last);
				break;
			case 'a':
				last = *(regstr++)-'0';
				if(last<0 ||  last>7) return 0xFF;
				lastr = 'a';
				out |= (0b100000000 << last);
				break;
			case '-':
				if(*(regstr++) != lastr) return 0xFF;
				to = *(regstr++)-'0';
				if(to < last) { uint8_t t = last; last = to; to = t; }
				if(last<0 || last>7 || to<0 || to>7) return 0xFF;
				for(uint8_t i = last; i<=to; i++) out |= ((lastr=='d' ? 1:0b100000000) << i);
				lastr = 'x';
				last = 0xFF;
			case ',':
			case '/':
				break;
			default:
				fprintf(state->error,"%s:%d:%d: error: cannot parse register set '%s'\n", state->source_filename, state->line_number, state->offsets.arg1.start, original); 
				return 0xFFFF;
		}
	}

	return out;
}

uint64_t parse_immediate_value(char *value, uint8_t width, state_t *state) {
	// #(decimal)
	// #0d(decimal)
	// #0x(hex)
	// #0b(binary)

	uint16_t pos = 0, len;
	
	len = strlen(value);

	if(len<2 || value[0]!='#') {
		fprintf(state->error,"%s:%d:%d: error: cannot parse immediate value '%s'\n", state->source_filename, state->line_number, state->offsets.arg1.start, value); 
		return 0xFFFFFFFF;
	}

	if(value[1]=='0') {
		if((value[2]=='x' || value[2] == 'X')) {
			return strtoul(value+3, NULL, 16);
		} else if((value[2]=='b' || value[2] == 'B')) {
			return strtoul(value+3, NULL, 2);
		} else if((value[2]=='d' || value[2] == 'd')) {
			return strtoul(value+3, NULL, 10);
		} else {
			return strtoul(value+1, NULL, 10);
		}
	} else if(value[1]=='\'') {
		if(value[2]=='\\') {
			switch(value[3]) {
			case 'r': return 13;
			case 'n': return 10;
			case 't': return 9;
			case '\\': return '\\';
			default: return 0xFF;
			}
		} else return value[2];
	} else {
		return strtol(value+1, NULL, 10);
	}
}

uint64_t parse_immediate_float_value(char *value, uint8_t width, state_t *state) {
	// #(float)

	uint16_t pos = 0, len;
	
	len = strlen(value);

	if(len<2 || value[0]!='#') {
		fprintf(state->error,"%s:%d:%d: error: cannot parse immediate float value '%s'\n", state->source_filename, state->line_number, state->offsets.arg1.start, value); 
		return 0;
	}

	// Pack the double into an uint64 without rounding it.
	double x = atof(value+1);
	return *((uint64_t*)(&x));
}