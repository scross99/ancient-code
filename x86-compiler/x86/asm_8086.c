#include "asm_8086.h"
#include <iostream>
#include <cstdio>
#include <cstring>
#include "instr.h"
#include "../func.h"
#include "../debug.h"

FILE * asm_fh;
asm_func * asm_functions;
const char * asm_current_function;
bool asm_use_esp = false;

asm_instr * asm_8086_gen_instructions(rd_instr * ins, rd_vlist * mem_list);
bool asm_8086_write(asm_instr * i);


void asm_function(func * fnc, rd_instr * rdcode, rd_vlist * mem_list){
	asm_func * f = new asm_func;
	f->fnc = fnc;
	f->code = asm_8086_gen_instructions(rdcode, mem_list);
	f->next = asm_functions;
	asm_functions = f;
}

//determine the smallest size of a variable in 4 byte blocks
unsigned int asm_var_size(rd_var * v){
	if((v->size % 4) == 0){
		return v->size;
	}else{
		return ((v->size / 4) + 1) * 4;
	}
}

//allocate memory on the stack
unsigned int asm_mem_alloc(rd_vlist * list){
	unsigned int * tbl = new unsigned int[list->num_vars];
	rd_var * v = list->start;
	unsigned int mem_loc = 0;
	while(v != 0){
		#ifdef DEBUG_MODE
		printf("Var %u given relative address %u of size %u\n", v->id, mem_loc, asm_var_size(v));
		#endif
		v->id = mem_loc;
		mem_loc += asm_var_size(v);
		v = v->next;
	}
	return mem_loc;
}

void asm_write(const char * filename, rd_vlist * const_list){
	asm_fh = fopen(filename, "w");
	asm_func * f = asm_functions;
	func * fnc = func_list;
	rd_var * v = const_list->start;
	unsigned int i;

	while(fnc != 0){
		if(fnc->global){
			fprintf(asm_fh, "global %s\n\n", fnc->name);
		}else if(fnc->external){
			fprintf(asm_fh, "extern %s\n\n", fnc->name);
		}
		fnc = fnc->next;
	}

	fprintf(asm_fh, "section .data\n\n");

	while(v != 0){
		fprintf(asm_fh, "const%u ", v->id);
		switch(v->size){
			case 1:
				fprintf(asm_fh, "db ");
				break;
			case 2:
				fprintf(asm_fh, "dw ");
				break;
			case 4:
				fprintf(asm_fh, "dd ");
				break;
			case 8:
				fprintf(asm_fh, "dq ");
				break;
			default:
				fprintf(asm_fh, "db ");
				break;
		}
		if(v->type == RD_CONST_FLOAT){
			fprintf(asm_fh, "%f", v->fval);
		}else if(v->type == RD_CONST_STR){
			i = strlen(v->str);
			for(i = 0; i < strlen(v->str); i++){
				fprintf(asm_fh, "%u,", v->str[i]);
			}
			fprintf(asm_fh, "0");
		}
		fprintf(asm_fh, "\n");
		v = v->next;
	}

	fprintf(asm_fh, "\n");

	fprintf(asm_fh, "section .text\n\n");

	f = asm_functions;

	while(f != 0){
		asm_current_function = f->fnc->name;
		asm_8086_write(f->code);
		fprintf(asm_fh, "\n");
		f = f->next;
	}

	fclose(asm_fh);
}

rd_var * asm_8086_gen_var(rd_var * v){
	return v;
}

rd_var * asm_8086_register(unsigned int id){
	return rd_register(id);
}

rd_var * asm_8086_section(unsigned int id){
	return rd_section(id);
}

asm_instr * asm_8086_instr_list(){
	asm_instr * ins = new asm_instr;
	ins->type = ASM_NONE;
	ins->next = ins->prev = 0;
	ins->p1 = ins->p2 = ins->p3 = 0;
	return ins;
}

asm_instr * asm_8086_instr(asm_instr * list, unsigned int type, rd_var * p1, rd_var * p2, rd_var * p3){
	if(type == ASM_MOV){
		if(rd_equal(p1, p2)){
			return list;
		}
	}
	asm_instr * ins = new asm_instr;
	ins->type = type;
	ins->prev = list;
	list->next = ins;
	ins->next = 0;
	ins->p1 = p1;
	ins->p2 = p2;
	ins->p3 = p3;
	return ins;
}

asm_instr * asm_8086_gen_instructions(rd_instr * ins, rd_vlist * mem_list){
	asm_instr * as = asm_8086_instr_list();
	asm_instr * as_start = as;

	unsigned int mem_size = asm_mem_alloc(mem_list);

	while(ins != 0){
		if(ins->type == RD_SECTION){
			as = asm_8086_instr(as, ASM_SECTION, asm_8086_section(ins->p1->id), 0, 0);
			if(ins->p1->id == 0 && mem_size != 0){
				as = asm_8086_instr(as, ASM_SUB, asm_8086_register(ESP), asm_8086_gen_var(rd_sint(mem_size)), 0);
			}
			ins = ins->next;
			continue;
		}
		switch(ins->type){
			case RD_GETPARAM:
				if(ins->p1->type == RD_MEM_SINT || ins->p1->type == RD_MEM_FLOAT){
					ins->p1->id = mem_size + 4 + ins->p2->sint * 4;
				}else{
					as = asm_8086_instr(as, ASM_MOV, asm_8086_gen_var(ins->p1), asm_8086_gen_var(rd_mem_sint(mem_size + 4 + ins->p2->sint * 4)), 0);
				}
				break;
			case RD_SET:
				if(!rd_equal(ins->p1, ins->p2)){
					if((rd_is_mem(ins->p1) || ins->p1->type == RD_CONST_FLOAT) &&
						(rd_is_mem(ins->p2) || ins->p2->type == RD_CONST_FLOAT)){
						as = asm_8086_instr(as, ASM_FLD, asm_8086_gen_var(ins->p2), 0, 0);
						as = asm_8086_instr(as, ASM_FSTP, asm_8086_gen_var(ins->p1), 0, 0);
					}else{
						if((ins->p1->type == RD_REG && ins->p1->id == EBP) ||
							(ins->p2->type == RD_REG && ins->p2->id == EBP)){
							as = asm_8086_instr(as, ASM_ESPMOV, asm_8086_gen_var(ins->p1), asm_8086_gen_var(ins->p2), 0);
						}else{
							as = asm_8086_instr(as, ASM_MOV, asm_8086_gen_var(ins->p1), asm_8086_gen_var(ins->p2), 0);
						}
					}
				}
				break;
			case RD_ADD:
				as = asm_8086_instr(as, ASM_ADD, asm_8086_gen_var(ins->p1), asm_8086_gen_var(ins->p2), 0);
				break;
			case RD_SUB:
				as = asm_8086_instr(as, ASM_SUB, asm_8086_gen_var(ins->p1), asm_8086_gen_var(ins->p2), 0);
				break;
			case RD_MUL:
				as = asm_8086_instr(as, ASM_MUL, asm_8086_gen_var(ins->p1), asm_8086_gen_var(ins->p2), 0);
				break;
			case RD_INC:
				as = asm_8086_instr(as, ASM_INC, asm_8086_gen_var(ins->p1), 0, 0);
				break;
			case RD_DEC:
				as = asm_8086_instr(as, ASM_DEC, asm_8086_gen_var(ins->p1), 0, 0);
				break;
			case RD_DIV:
				as = asm_8086_instr(as, ASM_DIV, asm_8086_gen_var(ins->p2), 0, 0);
				break;
			case RD_NOT:
				as = asm_8086_instr(as, ASM_NOT, asm_8086_gen_var(ins->p1), 0, 0);
				break;
			case RD_NEG:
				as = asm_8086_instr(as, ASM_NEG, asm_8086_gen_var(ins->p1), 0, 0);
				break;
			case RD_CMP:
				as = asm_8086_instr(as, ASM_CMP, asm_8086_gen_var(ins->p1), asm_8086_gen_var(ins->p2), 0);
				break;
			case RD_JE:
				as = asm_8086_instr(as, ASM_JE, asm_8086_gen_var(ins->p1), 0, 0);
				break;
			case RD_JNE:
				as = asm_8086_instr(as, ASM_JNE, asm_8086_gen_var(ins->p1), 0, 0);
				break;
			case RD_JG:
				as = asm_8086_instr(as, ASM_JG, asm_8086_gen_var(ins->p1), 0, 0);
				break;
			case RD_JL:
				as = asm_8086_instr(as, ASM_JL, asm_8086_gen_var(ins->p1), 0, 0);
				break;
			case RD_JGE:
				as = asm_8086_instr(as, ASM_JGE, asm_8086_gen_var(ins->p1), 0, 0);
				break;
			case RD_JLE:
				as = asm_8086_instr(as, ASM_JLE, asm_8086_gen_var(ins->p1), 0, 0);
				break;
			case RD_JUMP:
				as = asm_8086_instr(as, ASM_JMP, asm_8086_gen_var(ins->p1), 0, 0);
				break;
			case RD_CALL:
				as = asm_8086_instr(as, ASM_CALL, asm_8086_gen_var(ins->p1), 0, 0);
				//as = asm_8086_instr(as, ASM_MOV, asm_8086_gen_var(ins->p2), asm_8086_register(ASM_VAR_EAX), 0);
				break;
			case RD_PARAM:
				as = asm_8086_instr(as, ASM_PUSH, asm_8086_gen_var(ins->p1), 0, 0);
				break;
			case RD_CLRPARAM:
				ins->p1->sint *= 4; //each parameter is 4 bytes (a dword) on the stack
				as = asm_8086_instr(as, ASM_ADD, asm_8086_register(ESP), asm_8086_gen_var(ins->p1), 0);
				break;
			case RD_CDQ:
				as = asm_8086_instr(as, ASM_CDQ, 0, 0, 0);
				break;
			case RD_FPUSH:
				as = asm_8086_instr(as, ASM_FLD, asm_8086_gen_var(ins->p1), 0, 0);
				break;
			case RD_FPOP:
				as = asm_8086_instr(as, ASM_FSTP, asm_8086_gen_var(ins->p1), 0, 0);
				break;
			case RD_FPUSHI:
				as = asm_8086_instr(as, ASM_FILD, asm_8086_gen_var(ins->p1), 0, 0);
				break;
			case RD_FPOPI:
				as = asm_8086_instr(as, ASM_FISTP, asm_8086_gen_var(ins->p1), 0, 0);
				break;
			case RD_FADD:
				as = asm_8086_instr(as, ASM_FADD, asm_8086_gen_var(ins->p1), 0, 0);
				break;
			case RD_FSUB:
				as = asm_8086_instr(as, ASM_FSUB, asm_8086_gen_var(ins->p1), 0, 0);
				break;
			case RD_FMUL:
				as = asm_8086_instr(as, ASM_FMUL, asm_8086_gen_var(ins->p1), 0, 0);
				break;
			case RD_FDIV:
				as = asm_8086_instr(as, ASM_FDIV, asm_8086_gen_var(ins->p1), 0, 0);
				break;
			case RD_RETURN:
				if(mem_size != 0){
					as = asm_8086_instr(as, ASM_ADD, asm_8086_register(ESP), asm_8086_gen_var(rd_sint(mem_size)), 0);
				}
				as = asm_8086_instr(as, ASM_RET, 0, 0, 0);
				break;
			default:
				//nothing...
				break;
		}
		ins = ins->next;
	}
	return as_start;
}

void asm_write_varsize(rd_var * v){
	switch(v->size){
		case 1:
			fprintf(asm_fh, "byte");
			break;
		case 2:
			fprintf(asm_fh, "word");
			break;
		case 4:
			fprintf(asm_fh, "dword");
			break;
		case 8:
			fprintf(asm_fh, "qword");
			break;
	}
}

void asm_8086_write_var(rd_var * v){
	switch(v->type){
		case RD_MEM_SINT:
		case RD_MEM_FLOAT:
		case RD_MEM_STR:
			asm_write_varsize(v);
			if(asm_use_esp){
				fprintf(asm_fh, " [esp+%u]", v->id);
			}else{
				fprintf(asm_fh, " [ebp+%u]", v->id);
			}
			break;
		case RD_MEM_DEREF:
			v->type = RD_REG;
			fprintf(asm_fh, "[");
			asm_8086_write_var(v);
			fprintf(asm_fh, "+%u]", v->sint);
			break;
		case RD_REG:
			switch(v->id){
				case EAX:
					fprintf(asm_fh, "eax");
					break;
				case EBX:
					fprintf(asm_fh, "ebx");
					break;
				case ECX:
					fprintf(asm_fh, "ecx");
					break;
				case EDX:
					fprintf(asm_fh, "edx");
					break;
				case ESP:
					fprintf(asm_fh, "esp");
					break;
				case EBP:
					fprintf(asm_fh, "ebp");
					break;
				case ESI:
					fprintf(asm_fh, "esi");
					break;
				case EDI:
					fprintf(asm_fh, "edi");
					break;
				case ST0:
					fprintf(asm_fh, "ST0");
					break;
				case ST1:
					fprintf(asm_fh, "ST1");
					break;
				case ST2:
					fprintf(asm_fh, "ST2");
					break;
				case ST3:
					fprintf(asm_fh, "ST3");
					break;
				case ST4:
					fprintf(asm_fh, "ST4");
					break;
				case ST5:
					fprintf(asm_fh, "ST5");
					break;
				case ST6:
					fprintf(asm_fh, "ST6");
					break;
				case ST7:
					fprintf(asm_fh, "ST7");
					break;
			}
			break;
		case RD_CONST_SINT:
			fprintf(asm_fh, "dword %d", v->sint);
			break;
		case RD_CONST_FLOAT:
			fprintf(asm_fh, "dword [const%u]", v->id);
			break;
		case RD_CONST_STR:
			fprintf(asm_fh, "const%u", v->id);
			break;
		case RD_CONST_NAME:
			fprintf(asm_fh, "$%s", v->str);
			break;
		case RD_SEC:
			if(v->id == 0){
				fprintf(asm_fh, "$%s", asm_current_function);
			}else{
				fprintf(asm_fh, ".%u", v->id);
			}
			break;
	}
}

bool asm_8086_write(asm_instr * i){
	i = i->next; //first element is just a place holder

	while(i != 0){
		if(i->type == ASM_SECTION){
			asm_8086_write_var(i->p1);
			fprintf(asm_fh, ":\n");
			i = i->next;
			continue;
		}
		fprintf(asm_fh, "\t");
		switch(i->type){
			case ASM_ESPMOV:
				asm_use_esp = true;
			case ASM_MOV:
				fprintf(asm_fh, "mov");
				break;
			case ASM_ADD:
				fprintf(asm_fh, "add");
				break;
			case ASM_SUB:
				fprintf(asm_fh, "sub");
				break;
			case ASM_MUL:
				fprintf(asm_fh, "imul");
				break;
			case ASM_DIV:
				fprintf(asm_fh, "idiv");
				break;
			case ASM_INC:
				fprintf(asm_fh, "inc");
				break;
			case ASM_DEC:
				fprintf(asm_fh, "dec");
				break;
			case ASM_NOT:
				fprintf(asm_fh, "not");
				break;
			case ASM_NEG:
				fprintf(asm_fh, "neg");
				break;
			case ASM_CMP:
				fprintf(asm_fh, "cmp");
				break;
			case ASM_JE:
				fprintf(asm_fh, "je");
				break;
			case ASM_JNE:
				fprintf(asm_fh, "jne");
				break;
			case ASM_JG:
				fprintf(asm_fh, "jg");
				break;
			case ASM_JL:
				fprintf(asm_fh, "jl");
				break;
			case ASM_JGE:
				fprintf(asm_fh, "jge");
				break;
			case ASM_JLE:
				fprintf(asm_fh, "jle");
				break;
			case ASM_JMP:
				fprintf(asm_fh, "jmp");
				break;
			case ASM_PUSH:
				fprintf(asm_fh, "push");
				break;
			case ASM_POP:
				fprintf(asm_fh, "pop");
				break;
			case ASM_CALL:
				fprintf(asm_fh, "call");
				break;
			case ASM_XPUSH:
				if(i->p1->type == RD_CONST_FLOAT){
					fprintf(asm_fh, "push qword [const%u]\n", i->p1->id);
				}else{
					fprintf(asm_fh, "push qword [ebp+%u]\n", i->p1->id);
				}
				i = i->next;
				continue;
			case ASM_CDQ:
				fprintf(asm_fh, "cdq");
				break;
			case ASM_RET:
				fprintf(asm_fh, "ret");
				break;
			case ASM_FLD:
				fprintf(asm_fh, "fld");
				break;
			case ASM_FILD:
				fprintf(asm_fh, "fild");
				break;
			case ASM_FSTP:
				fprintf(asm_fh, "fstp");
				break;
			case ASM_FISTP:
				fprintf(asm_fh, "fistp");
				break;
			case ASM_FADD:
				fprintf(asm_fh, "fadd");
				break;
			case ASM_FSUB:
				fprintf(asm_fh, "fsub");
				break;
			case ASM_FMUL:
				fprintf(asm_fh, "fmul");
				break;
			case ASM_FDIV:
				fprintf(asm_fh, "fdiv");
				break;
		}
		if(i->p1 != 0){
			fprintf(asm_fh, " ");
			asm_8086_write_var(i->p1);
			if(i->p2 != 0){
				fprintf(asm_fh, ", ");
				asm_8086_write_var(i->p2);
				if(i->p3 != 0){
					fprintf(asm_fh, ", ");
					asm_8086_write_var(i->p3);
				}
			}
		}
		fprintf(asm_fh, "\n");
		asm_use_esp = false;
		i = i->next;
	}
}

