# System Routines

All system routines should be called with the `syscall` instruction, and take their arguments from the A7 stack in the following format:

## 0x0 - VM_SYSCALL_DUMP_MEMORY
Arguments:
	* *none*

## 0x1 - VM_SYSCALL_DUMP_STATE
Arguments:
	* *none*

## 0x2 - VM_SYSCALL_PRINT
Arguments:
	* (value32) pointer to char string

## 0x3 - VM_SYSCALL_PUTC
Arguments:
	* (value8) ASCII char to print