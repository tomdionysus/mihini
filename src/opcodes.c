#include "stree.h"
#include "opcodes.h"

void load_opcodes_values(stree_node_t *node) {
	stree_set(node, OPCODE_NOP_STR, (void*)(uint64_t)OPCODE_NOP);

	// Integer/Data
	stree_set(node, OPCODE_LOAD_STR, (void*)(uint64_t)OPCODE_LOAD);
	stree_set(node, OPCODE_MOVE_STR, (void*)(uint64_t)OPCODE_MOVE);
	stree_set(node, OPCODE_PUSH_STR, (void*)(uint64_t)OPCODE_PUSH);
	stree_set(node, OPCODE_POP_STR, (void*)(uint64_t)OPCODE_POP);
	stree_set(node, OPCODE_INC_STR, (void*)(uint64_t)OPCODE_INC);
	stree_set(node, OPCODE_DEC_STR, (void*)(uint64_t)OPCODE_DEC);
	stree_set(node, OPCODE_CLR_STR, (void*)(uint64_t)OPCODE_CLR);
	stree_set(node, OPCODE_ADDI_STR, (void*)(uint64_t)OPCODE_ADDI);
	stree_set(node, OPCODE_ADD_STR, (void*)(uint64_t)OPCODE_ADD);
	stree_set(node, OPCODE_SUBI_STR, (void*)(uint64_t)OPCODE_SUBI);
	stree_set(node, OPCODE_SUB_STR, (void*)(uint64_t)OPCODE_SUB);
	stree_set(node, OPCODE_MULI_STR, (void*)(uint64_t)OPCODE_MULI);
	stree_set(node, OPCODE_MUL_STR, (void*)(uint64_t)OPCODE_MUL);
	stree_set(node, OPCODE_DIVI_STR, (void*)(uint64_t)OPCODE_DIVI);
	stree_set(node, OPCODE_DIV_STR, (void*)(uint64_t)OPCODE_DIV);
	stree_set(node, OPCODE_ANDI_STR, (void*)(uint64_t)OPCODE_ANDI);
	stree_set(node, OPCODE_AND_STR, (void*)(uint64_t)OPCODE_AND);
	stree_set(node, OPCODE_ORI_STR, (void*)(uint64_t)OPCODE_ORI);
	stree_set(node, OPCODE_OR_STR, (void*)(uint64_t)OPCODE_OR);
	stree_set(node, OPCODE_XORI_STR, (void*)(uint64_t)OPCODE_XORI);
	stree_set(node, OPCODE_XOR_STR, (void*)(uint64_t)OPCODE_XOR);
	stree_set(node, OPCODE_LSLI_STR, (void*)(uint64_t)OPCODE_LSLI);
	stree_set(node, OPCODE_LSL_STR, (void*)(uint64_t)OPCODE_LSL);
	stree_set(node, OPCODE_LSRI_STR, (void*)(uint64_t)OPCODE_LSRI);
	stree_set(node, OPCODE_LSR_STR, (void*)(uint64_t)OPCODE_LSR);
	stree_set(node, OPCODE_ROLI_STR, (void*)(uint64_t)OPCODE_ROLI);
	stree_set(node, OPCODE_ROL_STR, (void*)(uint64_t)OPCODE_ROL);
	stree_set(node, OPCODE_RORI_STR, (void*)(uint64_t)OPCODE_RORI);
	stree_set(node, OPCODE_ROR_STR, (void*)(uint64_t)OPCODE_ROR);
	stree_set(node, OPCODE_NOT_STR, (void*)(uint64_t)OPCODE_NOT);
	stree_set(node, OPCODE_TST_STR, (void*)(uint64_t)OPCODE_TST);
	stree_set(node, OPCODE_CMPI_STR, (void*)(uint64_t)OPCODE_CMPI);
	stree_set(node, OPCODE_CMP_STR, (void*)(uint64_t)OPCODE_CMP);

	// Floating Point
	stree_set(node, OPCODE_FLOAD_STR, (void*)(uint64_t)OPCODE_FLOAD);
	stree_set(node, OPCODE_FCAST_STR, (void*)(uint64_t)OPCODE_FCAST);
	stree_set(node, OPCODE_FROUND_STR, (void*)(uint64_t)OPCODE_FROUND);
	stree_set(node, OPCODE_FADD_STR, (void*)(uint64_t)OPCODE_FADD);
	stree_set(node, OPCODE_FSUB_STR, (void*)(uint64_t)OPCODE_FSUB);
	stree_set(node, OPCODE_FMUL_STR, (void*)(uint64_t)OPCODE_FMUL);
	stree_set(node, OPCODE_FDIV_STR, (void*)(uint64_t)OPCODE_FDIV);
	stree_set(node, OPCODE_FMOD_STR, (void*)(uint64_t)OPCODE_FMOD);
	stree_set(node, OPCODE_FSQRT_STR, (void*)(uint64_t)OPCODE_FSQRT);
	stree_set(node, OPCODE_FEXP_STR, (void*)(uint64_t)OPCODE_FEXP);
	stree_set(node, OPCODE_FLOG_STR, (void*)(uint64_t)OPCODE_FLOG);
	stree_set(node, OPCODE_FLOG10_STR, (void*)(uint64_t)OPCODE_FLOG10);
	stree_set(node, OPCODE_FCEIL_STR, (void*)(uint64_t)OPCODE_FCEIL);
	stree_set(node, OPCODE_FFLOOR_STR, (void*)(uint64_t)OPCODE_FFLOOR);
	stree_set(node, OPCODE_FPOW_STR, (void*)(uint64_t)OPCODE_FPOW);
	stree_set(node, OPCODE_FSIN_STR, (void*)(uint64_t)OPCODE_FSIN);
	stree_set(node, OPCODE_FCOS_STR, (void*)(uint64_t)OPCODE_FCOS);
	stree_set(node, OPCODE_FTAN_STR, (void*)(uint64_t)OPCODE_FTAN);

	// Virtual instruction DATA (assembler only)
	stree_set(node, OPCODE_DATA_STR, (void*)(uint64_t)OPCODE_DATA);

	// Flow Control
	stree_set(node, OPCODE_BRA_STR, (void*)(uint64_t)OPCODE_BRA);
	stree_set(node, OPCODE_BEQ_STR, (void*)(uint64_t)OPCODE_BEQ);
	stree_set(node, OPCODE_BNE_STR, (void*)(uint64_t)OPCODE_BNE);
	stree_set(node, OPCODE_BCB_STR, (void*)(uint64_t)OPCODE_BCB);
	stree_set(node, OPCODE_BLT_STR, (void*)(uint64_t)OPCODE_BLT);
	stree_set(node, OPCODE_BGT_STR, (void*)(uint64_t)OPCODE_BGT);
	stree_set(node, OPCODE_BLE_STR, (void*)(uint64_t)OPCODE_BLE);
	stree_set(node, OPCODE_BGE_STR, (void*)(uint64_t)OPCODE_BGE);
	stree_set(node, OPCODE_JMPI_STR, (void*)(uint64_t)OPCODE_JMPI);
	stree_set(node, OPCODE_JMPM_STR, (void*)(uint64_t)OPCODE_JMPM);
	stree_set(node, OPCODE_JMP_STR, (void*)(uint64_t)OPCODE_JMP);
	stree_set(node, OPCODE_CALLI_STR, (void*)(uint64_t)OPCODE_CALLI);
	stree_set(node, OPCODE_CALLM_STR, (void*)(uint64_t)OPCODE_CALLM);
	stree_set(node, OPCODE_CALL_STR, (void*)(uint64_t)OPCODE_CALL);
	stree_set(node, OPCODE_RET_STR, (void*)(uint64_t)OPCODE_RET);

	// System
	stree_set(node, OPCODE_ALLOC_STR, (void*)(uint64_t)OPCODE_ALLOC);
	stree_set(node, OPCODE_FREE_STR, (void*)(uint64_t)OPCODE_FREE);
	stree_set(node, OPCODE_SYSCALL_STR, (void*)(uint64_t)OPCODE_SYSCALL);
	stree_set(node, OPCODE_REFER_STR, (void*)(uint64_t)OPCODE_REFER);
	stree_set(node, OPCODE_STOP_STR, (void*)(uint64_t)OPCODE_STOP);
	stree_set(node, OPCODE_EXT_STR, (void*)(uint64_t)OPCODE_EXT);
}

void load_opcodes_strings(btree_node_t **node) {
	btree_add(node, OPCODE_NOP, OPCODE_NOP_STR);

	// Integer/Data
	btree_add(node, OPCODE_LOAD, OPCODE_LOAD_STR);
	btree_add(node, OPCODE_MOVE, OPCODE_MOVE_STR);
	btree_add(node, OPCODE_PUSH, OPCODE_PUSH_STR);
	btree_add(node, OPCODE_POP, OPCODE_POP_STR);
	btree_add(node, OPCODE_INC, OPCODE_INC_STR);
	btree_add(node, OPCODE_DEC, OPCODE_DEC_STR);
	btree_add(node, OPCODE_CLR, OPCODE_CLR_STR);
	btree_add(node, OPCODE_ADDI, OPCODE_ADDI_STR);
	btree_add(node, OPCODE_ADD, OPCODE_ADD_STR);
	btree_add(node, OPCODE_SUBI, OPCODE_SUBI_STR);
	btree_add(node, OPCODE_SUB, OPCODE_SUB_STR);
	btree_add(node, OPCODE_MULI, OPCODE_MULI_STR);
	btree_add(node, OPCODE_MUL, OPCODE_MUL_STR);
	btree_add(node, OPCODE_DIVI, OPCODE_DIVI_STR);
	btree_add(node, OPCODE_DIV, OPCODE_DIV_STR);
	btree_add(node, OPCODE_ANDI, OPCODE_ANDI_STR);
	btree_add(node, OPCODE_AND, OPCODE_AND_STR);
	btree_add(node, OPCODE_ORI, OPCODE_ORI_STR);
	btree_add(node, OPCODE_OR, OPCODE_OR_STR);
	btree_add(node, OPCODE_XORI, OPCODE_XORI_STR);
	btree_add(node, OPCODE_XOR, OPCODE_XOR_STR);
	btree_add(node, OPCODE_LSLI, OPCODE_LSLI_STR);
	btree_add(node, OPCODE_LSL, OPCODE_LSL_STR);
	btree_add(node, OPCODE_LSRI, OPCODE_LSRI_STR);
	btree_add(node, OPCODE_LSR, OPCODE_LSR_STR);
	btree_add(node, OPCODE_ROLI, OPCODE_ROLI_STR);
	btree_add(node, OPCODE_ROL, OPCODE_ROL_STR);
	btree_add(node, OPCODE_RORI, OPCODE_RORI_STR);
	btree_add(node, OPCODE_ROR, OPCODE_ROR_STR);
	btree_add(node, OPCODE_NOT, OPCODE_NOT_STR);
	btree_add(node, OPCODE_TST, OPCODE_TST_STR);
	btree_add(node, OPCODE_CMPI, OPCODE_CMPI_STR);
	btree_add(node, OPCODE_CMP, OPCODE_CMP_STR);

	// Floating point
	btree_add(node, OPCODE_FLOAD, OPCODE_FLOAD_STR);
	btree_add(node, OPCODE_FCAST, OPCODE_FCAST_STR);
	btree_add(node, OPCODE_FROUND, OPCODE_FROUND_STR);
	btree_add(node, OPCODE_FADD, OPCODE_FADD_STR);
	btree_add(node, OPCODE_FSUB, OPCODE_FSUB_STR);
	btree_add(node, OPCODE_FMUL, OPCODE_FMUL_STR);
	btree_add(node, OPCODE_FDIV, OPCODE_FDIV_STR);
	btree_add(node, OPCODE_FMOD, OPCODE_FMOD_STR);
	btree_add(node, OPCODE_FSQRT, OPCODE_FSQRT_STR);
	btree_add(node, OPCODE_FEXP, OPCODE_FEXP_STR);
	btree_add(node, OPCODE_FLOG, OPCODE_FLOG_STR);
	btree_add(node, OPCODE_FLOG10, OPCODE_FLOG10_STR);
	btree_add(node, OPCODE_FCEIL, OPCODE_FCEIL_STR);
	btree_add(node, OPCODE_FFLOOR, OPCODE_FFLOOR_STR);
	btree_add(node, OPCODE_FPOW, OPCODE_FPOW_STR);
	btree_add(node, OPCODE_FSIN, OPCODE_FSIN_STR);
	btree_add(node, OPCODE_FCOS, OPCODE_FCOS_STR);
	btree_add(node, OPCODE_FTAN, OPCODE_FTAN_STR);

	// Flow Control
	btree_add(node, OPCODE_BRA, OPCODE_BRA_STR);
	btree_add(node, OPCODE_BEQ, OPCODE_BEQ_STR);
	btree_add(node, OPCODE_BNE, OPCODE_BNE_STR);
	btree_add(node, OPCODE_BCB, OPCODE_BCB_STR);
	btree_add(node, OPCODE_BLT, OPCODE_BLT_STR);
	btree_add(node, OPCODE_BGT, OPCODE_BGT_STR);
	btree_add(node, OPCODE_BLE, OPCODE_BLE_STR);
	btree_add(node, OPCODE_BGE, OPCODE_BGE_STR);
	btree_add(node, OPCODE_JMPI, OPCODE_JMPI_STR);
	btree_add(node, OPCODE_JMPM, OPCODE_JMPM_STR);
	btree_add(node, OPCODE_JMP,OPCODE_JMP_STR);
	btree_add(node, OPCODE_CALLI, OPCODE_CALLI_STR);
	btree_add(node, OPCODE_CALLM, OPCODE_CALLM_STR);
	btree_add(node, OPCODE_CALL, OPCODE_CALL_STR);
	btree_add(node, OPCODE_RET, OPCODE_RET_STR);

	// Specials
	btree_add(node, OPCODE_ALLOC, OPCODE_ALLOC_STR);
	btree_add(node, OPCODE_FREE, OPCODE_FREE_STR);
	btree_add(node, OPCODE_SYSCALL, OPCODE_SYSCALL_STR);
	btree_add(node, OPCODE_STOP, OPCODE_STOP_STR);
	btree_add(node, OPCODE_REFER, OPCODE_REFER_STR);
	btree_add(node, OPCODE_EXT, OPCODE_EXT_STR);
}