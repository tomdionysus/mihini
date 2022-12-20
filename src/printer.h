#ifndef PRINTER
#define PRINTER

#include <stdint.h>

#define WIDTH_8   0b00
#define WIDTH_16  0b01
#define WIDTH_32  0b10
#define WIDTH_64  0b11
#define WIDTH_UNKNOWN  0xFF

char print_width(uint8_t width);
void print_register(uint8_t reg, char *buffer);
void print_register_bitmask(uint16_t mask, char *buffer);
void print_immediate_value(uint64_t value, char *buffer);
void print_immediate_float_value(uint64_t value, char *buffer);

#endif