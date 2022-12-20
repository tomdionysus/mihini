#ifndef OPCODES
#define OPCODES

#include "btree.h"
#include "stree.h"

#define OPCODE_NOP			0b00000000
#define OPCODE_LOAD			0b00000100
#define OPCODE_MOVE			0b00001000
#define OPCODE_PUSH			0b00001100
#define OPCODE_POP			0b00010000
#define OPCODE_INC			0b00010100
#define OPCODE_DEC			0b00011000
#define OPCODE_CLR			0b00011100
#define OPCODE_ADDI			0b00100000
#define OPCODE_ADD			0b00100100
#define OPCODE_SUBI			0b00101000
#define OPCODE_SUB			0b00101100
#define OPCODE_MULI			0b00110000
#define OPCODE_MUL			0b00110100
#define OPCODE_DIVI			0b00111000
#define OPCODE_DIV			0b00111100
#define OPCODE_ANDI			0b01000000
#define OPCODE_AND			0b01000100
#define OPCODE_ORI			0b01001000
#define OPCODE_OR			0b01001100
#define OPCODE_XORI			0b01010000
#define OPCODE_XOR			0b01010100
#define OPCODE_LSLI			0b01011000
#define OPCODE_LSL			0b01011100
#define OPCODE_LSRI			0b01100000
#define OPCODE_LSR			0b01100100
#define OPCODE_ROLI			0b01101000
#define OPCODE_ROL			0b01101100
#define OPCODE_RORI			0b01110000
#define OPCODE_ROR			0b01110100
#define OPCODE_NOT			0b01111000
#define OPCODE_TSTI			0b01111100
#define OPCODE_TST			0b10000000
#define OPCODE_CMPI			0b10000100
#define OPCODE_CMP			0b10001000

#define OPCODE_FLOAD		0b10001100
#define OPCODE_FCAST		0b10010000
#define OPCODE_FROUND		0b10010100
#define OPCODE_FADD			0b10011000
#define OPCODE_FSUB			0b10011100
#define OPCODE_FMUL			0b10100000
#define OPCODE_FDIV			0b10100100
#define OPCODE_FMOD			0b10101000
#define OPCODE_FSQRT		0b10101100
#define OPCODE_FEXP			0b10110000
#define OPCODE_FLOG			0b10110100
#define OPCODE_FLOG10		0b10111000
#define OPCODE_FCEIL		0b10111100
#define OPCODE_FFLOOR		0b11000000
#define OPCODE_FPOW			0b11000100
#define OPCODE_FSIN			0b11001000
#define OPCODE_FCOS			0b11001100
#define OPCODE_FTAN			0b11010000

#define OPCODE_DATA 		0b11011100

#define OPCODE_BRA			0b11101000
#define OPCODE_BEQ			0b11101001
#define OPCODE_BNE			0b11101010
#define OPCODE_BCB			0b11101011
#define OPCODE_BLT			0b11101100
#define OPCODE_BGT			0b11101101
#define OPCODE_BLE			0b11101110
#define OPCODE_BGE			0b11101111
#define OPCODE_JMPI			0b11110000
#define OPCODE_JMPM			0b11110001
#define OPCODE_JMP			0b11110010
#define OPCODE_CALLI		0b11110011
#define OPCODE_CALLM		0b11110100
#define OPCODE_CALL			0b11110101
#define OPCODE_RET			0b11110110

#define OPCODE_ALLOC		0b11110111
#define OPCODE_FREE			0b11111000
#define OPCODE_SYSCALL		0b11111001
#define OPCODE_REFER		0b11111010

#define OPCODE_STOP			0b11111101
#define OPCODE_EXT			0b11111111


#define OPCODE_NOP_STR		"nop"
#define OPCODE_LOAD_STR		"load"
#define OPCODE_MOVE_STR		"move"
#define OPCODE_PUSH_STR		"push"
#define OPCODE_POP_STR		"pop"
#define OPCODE_INC_STR		"inc"
#define OPCODE_DEC_STR		"dec"
#define OPCODE_CLR_STR		"clr"
#define OPCODE_ADDI_STR		"addi"
#define OPCODE_ADD_STR		"add"
#define OPCODE_SUBI_STR		"subi"
#define OPCODE_SUB_STR		"sub"
#define OPCODE_MULI_STR		"muli"
#define OPCODE_MUL_STR		"mul"
#define OPCODE_DIVI_STR		"divi"
#define OPCODE_DIV_STR		"div"
#define OPCODE_ANDI_STR		"andi"
#define OPCODE_AND_STR		"and"
#define OPCODE_ORI_STR		"ori"
#define OPCODE_OR_STR		"or"
#define OPCODE_XORI_STR		"xori"
#define OPCODE_XOR_STR		"xor"
#define OPCODE_LSLI_STR		"lsli"
#define OPCODE_LSL_STR		"lsl"
#define OPCODE_LSRI_STR		"lsri"
#define OPCODE_LSR_STR		"lsr"
#define OPCODE_ROLI_STR		"roli"
#define OPCODE_ROL_STR		"rol"
#define OPCODE_RORI_STR		"rori"
#define OPCODE_ROR_STR		"ror"
#define OPCODE_NOT_STR		"not"
#define OPCODE_TSTI_STR		"tsti"
#define OPCODE_TST_STR		"tst"
#define OPCODE_CMPI_STR		"cmpi"
#define OPCODE_CMP_STR		"cmp"

#define OPCODE_FLOAD_STR	"fload"
#define OPCODE_FCAST_STR	"fcast"
#define OPCODE_FROUND_STR	"fround"
#define OPCODE_FADD_STR		"fadd"
#define OPCODE_FSUB_STR		"fsub"
#define OPCODE_FMUL_STR		"fmul"
#define OPCODE_FDIV_STR		"fdiv"
#define OPCODE_FMOD_STR		"fmod"
#define OPCODE_FSQRT_STR	"fsqrt"
#define OPCODE_FEXP_STR		"fexp"
#define OPCODE_FLOG_STR		"flog"
#define OPCODE_FLOG10_STR	"flog10"
#define OPCODE_FCEIL_STR	"fceil"
#define OPCODE_FFLOOR_STR	"ffloor"
#define OPCODE_FPOW_STR		"fpow"
#define OPCODE_FSIN_STR		"fsin"
#define OPCODE_FCOS_STR		"fcos"
#define OPCODE_FTAN_STR		"ftan"

#define OPCODE_BRA_STR		"bra"
#define OPCODE_BEQ_STR		"beq"
#define OPCODE_BNE_STR		"bne"
#define OPCODE_BCB_STR		"bcb"
#define OPCODE_BLT_STR		"blt"
#define OPCODE_BGT_STR		"bgt"
#define OPCODE_BLE_STR		"ble"
#define OPCODE_BGE_STR		"bge"
#define OPCODE_JMPI_STR		"jmpi"
#define OPCODE_JMPM_STR		"jmpm"
#define OPCODE_JMP_STR		"jmp"
#define OPCODE_CALLI_STR	"calli"
#define OPCODE_CALLM_STR	"callm"
#define OPCODE_CALL_STR		"call"
#define OPCODE_RET_STR		"ret"

#define OPCODE_ALLOC_STR	"alloc"
#define OPCODE_FREE_STR		"free"
#define OPCODE_SYSCALL_STR	"syscall"
#define OPCODE_STOP_STR		"stop"
#define OPCODE_REFER_STR	"refer"
#define OPCODE_EXT_STR		"ext"

#define OPCODE_DATA_STR		"data"

#define WIDTH_8				0b00
#define WIDTH_16			0b01
#define WIDTH_32			0b10
#define WIDTH_64			0b11
#define WIDTH_UNKNOWN		0xFF

void load_opcodes_values(stree_node_t *node);
void load_opcodes_strings(btree_node_t **node);

#endif