#include "frame_8086.h"
#include <iostream>
#include "instr.h"
#include "../func.h"

rd_instr * frame_insert_before(rd_instr * code, rd_instr * ins){
	ins->next = code;
	ins->prev = code->prev;
	if(code->prev != 0){
		code->prev->next = ins;
	}
	code->prev = ins;
	return code;
}

rd_instr * frame_insert_after(rd_instr * code, rd_instr * ins){
	ins->prev = code;
	ins->next = code->next;
	if(code->next != 0){
		code->next->prev = ins;
	}
	code->next = ins;
	return ins;
}

void frame(rd_instr * code, rd_vlist * vlist, rd_vlist * mem_list){
	rd_var * v, * w;
	rd_var * ebx = rd_vlist_tmp(vlist);
	rd_var * ebp = rd_vlist_tmp(vlist);
	rd_var * esi = rd_vlist_tmp(vlist);
	rd_var * edi = rd_vlist_tmp(vlist);
	unsigned int loop_level = 0;
	ebx->count = ebp->count = esi->count = edi->count = 0; //ensures that these temporaries are the most likely to be spilled
	rd_instr * ins;

	ins = rd_new_instruction(RD_DEFINED, rd_register(ESP), 0, 0, 0);
	code = frame_insert_after(code, ins);

	ins = rd_new_instruction(RD_SET, ebp, rd_register(EBP), 0, 0);
	code = frame_insert_after(code, ins);

	ins = rd_new_instruction(RD_SET, rd_register(EBP), rd_register(ESP), 0, 0);
	code = frame_insert_after(code, ins);

	ins = rd_new_instruction(RD_SET, ebx, rd_register(EBX), 0, 0);
	code = frame_insert_after(code, ins);

	ins = rd_new_instruction(RD_SET, esi, rd_register(ESI), 0, 0);
	code = frame_insert_after(code, ins);

	ins = rd_new_instruction(RD_SET, edi, rd_register(EDI), 0, 0);
	code = frame_insert_after(code, ins);

	if(code->next == 0){
		code = rd_new_instruction(RD_USED, rd_register(EBP), 0, 0, code);
		code = rd_new_instruction(RD_USED, rd_register(ESP), 0, 0, code);
		code = rd_new_instruction(RD_SET, rd_register(EBX), ebx, 0, code);
		code = rd_new_instruction(RD_SET, rd_register(ESI), esi, 0, code);
		code = rd_new_instruction(RD_SET, rd_register(EDI), edi, 0, code);
		code = rd_new_instruction(RD_SET, rd_register(EBP), ebp, 0, code);
		code = rd_new_instruction(RD_RETURN, 0, 0, 0, code);
		return;
	}

	code = code->next;
	while(code != 0){
		if(code->type == RD_LOOPSTART){
			loop_level++;
		}else if(code->type == RD_LOOPEND){
			loop_level--;
		}
		switch(code->type){
		case RD_SUBSCR:
			v = code->p1;

			func_include("string_subscr");

			ins = rd_new_instruction(RD_PARAM, code->p3, 0, 0, 0);
			code = frame_insert_before(code, ins);

			ins = rd_new_instruction(RD_PARAM, code->p2, 0, 0, 0);
			code = frame_insert_before(code, ins);

			ins = rd_new_instruction(RD_DEFINED, rd_register(EAX), 0, 0, 0);
			code = frame_insert_before(code, ins);

			ins = rd_new_instruction(RD_DEFINED, rd_register(ECX), 0, 0, 0);
			code = frame_insert_before(code, ins);

			ins = rd_new_instruction(RD_DEFINED, rd_register(EDX), 0, 0, 0);
			code = frame_insert_before(code, ins);

			ins = rd_new_instruction(RD_CALL, rd_name("string_subscr"), 0, 0, 0);
			code = frame_insert_before(code, ins);

			ins = rd_new_instruction(RD_CLRPARAM, rd_sint(2), 0, 0, 0);
			code = frame_insert_before(code, ins);

			code->p1 = code->p2 = code->p3 = 0;

			ins = rd_new_instruction(RD_SET, v, rd_register(EAX), 0, 0);
			code = frame_insert_after(code, ins);

			ins = rd_new_instruction(RD_USED, rd_register(ECX), 0, 0, 0);
			code = frame_insert_after(code, ins);

			ins = rd_new_instruction(RD_USED, rd_register(EDX), 0, 0, 0);
			code = frame_insert_after(code, ins);
			break;
		case RD_SUBSCR_SET:
			func_include("string_subscr_set");

			ins = rd_new_instruction(RD_PARAM, code->p3, 0, 0, 0);
			code = frame_insert_before(code, ins);

			ins = rd_new_instruction(RD_PARAM, code->p2, 0, 0, 0);
			code = frame_insert_before(code, ins);

			ins = rd_new_instruction(RD_PARAM, code->p1, 0, 0, 0);
			code = frame_insert_before(code, ins);

			ins = rd_new_instruction(RD_DEFINED, rd_register(EAX), 0, 0, 0);
			code = frame_insert_before(code, ins);

			ins = rd_new_instruction(RD_DEFINED, rd_register(ECX), 0, 0, 0);
			code = frame_insert_before(code, ins);

			ins = rd_new_instruction(RD_DEFINED, rd_register(EDX), 0, 0, 0);
			code = frame_insert_before(code, ins);

			ins = rd_new_instruction(RD_CALL, rd_name("string_subscr_set"), 0, 0, 0);
			code = frame_insert_before(code, ins);

			ins = rd_new_instruction(RD_CLRPARAM, rd_sint(3), 0, 0, 0);
			code = frame_insert_before(code, ins);

			code->p1 = code->p2 = code->p3 = 0;

			ins = rd_new_instruction(RD_USED, rd_register(EAX), 0, 0, 0);
			code = frame_insert_after(code, ins);

			ins = rd_new_instruction(RD_USED, rd_register(ECX), 0, 0, 0);
			code = frame_insert_after(code, ins);

			ins = rd_new_instruction(RD_USED, rd_register(EDX), 0, 0, 0);
			code = frame_insert_after(code, ins);
			break;
		case RD_CALL:
			v = code->p2;

			code->p2 = 0;

			ins = rd_new_instruction(RD_DEFINED, rd_register(EAX), 0, 0, 0);
			code = frame_insert_before(code, ins);

			ins = rd_new_instruction(RD_DEFINED, rd_register(ECX), 0, 0, 0);
			code = frame_insert_before(code, ins);

			ins = rd_new_instruction(RD_DEFINED, rd_register(EDX), 0, 0, 0);
			code = frame_insert_before(code, ins);

			if(code->next != 0 && code->next->type == RD_CLRPARAM){
				code = code->next;
			}

			ins = rd_new_instruction(RD_SET, v, rd_register(EAX), 0, 0);
			code = frame_insert_after(code, ins);

			ins = rd_new_instruction(RD_USED, rd_register(ECX), 0, 0, 0);
			code = frame_insert_after(code, ins);

			ins = rd_new_instruction(RD_USED, rd_register(EDX), 0, 0, 0);
			code = frame_insert_after(code, ins);
			break;
		case RD_DIV:
			v = code->p1;
			w = rd_vlist_tmp(vlist);

			v->count += 1 << (loop_level * 3);
			w->count = 100000; //ensures that spilling of this variable will not occur

			ins = rd_new_instruction(RD_DEFINED, rd_register(EDX), 0, 0, 0);
			code = frame_insert_before(code, ins);

			ins = rd_new_instruction(RD_SET, rd_register(EAX), v, 0, 0);
			code = frame_insert_before(code, ins);

			ins = rd_new_instruction(RD_CDQ, 0, 0, 0, 0);
			code = frame_insert_before(code, ins);

			ins = rd_new_instruction(RD_SET, w, code->p2, 0, 0);
			code = frame_insert_before(code, ins);

			code->p1 = 0;
			code->p2 = w;

			ins = rd_new_instruction(RD_USED, rd_register(EDX), 0, 0, 0);
			code = frame_insert_after(code, ins);

			ins = rd_new_instruction(RD_SET, v, rd_register(EAX), 0, 0);
			code = frame_insert_after(code, ins);
			break;
		case RD_FADD:
		case RD_FSUB:
		case RD_FMUL:
		case RD_FDIV:
			v = code->p1;

			ins = rd_new_instruction(RD_FPUSH, v, 0, 0, 0);
			code = frame_insert_before(code, ins);

			code->p1 = code->p2;
			code->p2 = 0;

			ins = rd_new_instruction(RD_FPOP, v, 0, 0, 0);
			code = frame_insert_after(code, ins);
			break;
		case RD_RETURN:
			v = code->p1;
			code->p1 = 0;

			ins = rd_new_instruction(RD_USED, rd_register(ESP), 0, 0, 0);
			code = frame_insert_before(code, ins);

			ins = rd_new_instruction(RD_USED, rd_register(EBP), 0, 0, 0);
			code = frame_insert_before(code, ins);

			ins = rd_new_instruction(RD_SET, rd_register(EBX), ebx, 0, 0);
			code = frame_insert_before(code, ins);

			ins = rd_new_instruction(RD_SET, rd_register(ESI), esi, 0, 0);
			code = frame_insert_before(code, ins);

			ins = rd_new_instruction(RD_SET, rd_register(EDI), edi, 0, 0);
			code = frame_insert_before(code, ins);

			ins = rd_new_instruction(RD_SET, rd_register(EBP), ebp, 0, 0);
			code = frame_insert_before(code, ins);

			ins = rd_new_instruction(RD_SET, rd_register(EAX), v, 0, 0);
			code = frame_insert_before(code, ins);
			break;
		}
		if(code->next == 0){
			if(code->type == RD_RETURN){
				break;
			}
			code = rd_new_instruction(RD_USED, rd_register(EBP), 0, 0, code);
			code = rd_new_instruction(RD_USED, rd_register(ESP), 0, 0, code);
			code = rd_new_instruction(RD_SET, rd_register(EBX), ebx, 0, code);
			code = rd_new_instruction(RD_SET, rd_register(ESI), esi, 0, code);
			code = rd_new_instruction(RD_SET, rd_register(EDI), edi, 0, code);
			code = rd_new_instruction(RD_SET, rd_register(EBP), ebp, 0, code);
			code = rd_new_instruction(RD_RETURN, 0, 0, 0, code);
			break;
		}
		code = code->next;
	}
}

