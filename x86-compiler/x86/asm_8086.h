#ifndef ASM_8086_H
#define ASM_8086_H

#include "../reduced.h"
#include "../func.h"
#include "regs.h"

//ASM instructions
#define ASM_NONE 0
#define ASM_MOV 1
#define ASM_ADD 2
#define ASM_SUB 3
#define ASM_MUL 4
#define ASM_DIV 5
#define ASM_INC 6
#define ASM_DEC 7
#define ASM_NOT 8
#define ASM_NEG 9

#define ASM_CMP 10
#define ASM_JE 11
#define ASM_JNE 12
#define ASM_JG 13
#define ASM_JL 14
#define ASM_JGE 15
#define ASM_JLE 16
#define ASM_JMP 17

#define ASM_PUSH 20
#define ASM_POP 21
#define ASM_CALL 22
#define ASM_XPUSH 23 //push 64 bit values onto the stack in correct order

#define ASM_ESPMOV 25

#define ASM_RET 30

#define ASM_SECTION 50

#define ASM_CDQ 60

//Floating point instructions
#define ASM_FLD   100 //push float
#define ASM_FILD  101 //push integer
#define ASM_FSTP  105 //pop float
#define ASM_FISTP 106 //pop integer

#define ASM_FADD  120 //ST0 += float
#define ASM_FSUB  121 //ST0 -= float
#define ASM_FMUL  122 //STO *= float
#define ASM_FDIV  123 //STO /= float

typedef struct asm_instr{
	unsigned int type;
	rd_var * p1;
	rd_var * p2;
	rd_var * p3;
	asm_instr * prev;
	asm_instr * next;
} asm_instr;

typedef struct asm_func{
	func * fnc;
	asm_instr * code;
	asm_func * next;
} asm_func;

void asm_function(func * fnc, rd_instr * rdcode, rd_vlist * mem_list);

void asm_write(const char * filename, rd_vlist * const_list);


#endif
