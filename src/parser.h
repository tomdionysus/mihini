#ifndef PARSER
#define PARSER

#include <stdint.h>
#include "opcodes.h"
#include "mhasm.h"

uint8_t parse_width(char width, state_t *state);
uint8_t parse_register(char *regstr, state_t *state);
uint16_t parse_register_bitmask(char *regstr, state_t *state);
uint64_t parse_immediate_value(char *immediate, uint8_t width, state_t *state);
uint64_t parse_immediate_float_value(char *value, uint8_t width, state_t *state);

#endif