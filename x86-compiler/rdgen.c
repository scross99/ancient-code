#include <cstdio>
#include <cstring>
#include "rdgen.h"
#include "func.h"

typedef struct rdg_inc_list{
	rd_var * var;
	bool inc;
	rdg_inc_list * next;
} rdg_inc_list;

rdg_inc_list * rdg_ilist = 0;
rd_instr * rdgen_first_instr = 0;
rd_instr * rdgen_current_instr = 0;
rd_vlist * rdgen_vlist, * rdgen_memlist, * rdgen_constlist;
unsigned int rdgen_sec_pos = 0;
unsigned int rdgen_param_num; //uint used to count parameters so that they can be cleared after a function call
char rdgen_if_complete = 1; //flag used in the generation of if statements
unsigned int rdgen_loop_level = 0;

unsigned int * rdgen_float_table = 0;

rd_var * rdgen_recur(tree * code);

void rdgen_reset(){
	rdgen_first_instr = 0;
	rdgen_current_instr = 0;
	rdgen_vlist = 0;
	rdgen_memlist = 0;
	rdgen_constlist = 0;
	rdgen_sec_pos = 0;
	rdgen_param_num = 0;
	rdgen_if_complete = 1;
	rdgen_loop_level = 0;
}

//add an instruction
void rdgen_instruction(unsigned int type, rd_var * p1, rd_var * p2, rd_var * p3){
	if(p1 != 0 && p1->type == RD_VAR){
		p1->count += 1 << (3 * rdgen_loop_level);
		p1->instances++;
	}
	if(p2 != 0 && p2->type == RD_VAR){
		p2->count += 1 << (3 * rdgen_loop_level);
		p2->instances++;
	}
	if(p3 != 0 && p3->type == RD_VAR){
		p3->count += 1 << (3 * rdgen_loop_level);
		p3->instances++;
	}
	rdgen_current_instr = rd_new_instruction(type, p1, p2, p3, rdgen_current_instr);
}

//start a new section
void rdgen_section(unsigned int id){
	rdgen_current_instr = rd_new_instruction(RD_SECTION, rd_section(id), 0, 0, rdgen_current_instr);
}

//create a temporary variable
rd_var * rdgen_tmpvar(){
	return rd_vlist_tmp(rdgen_vlist);
}

//create a memory variable
rd_var * rdgen_memvar(unsigned int type){
	return rd_vlist_mem(rdgen_memlist, type);
}

//create a variable
rd_var * rdgen_var(unsigned int id, unsigned int type){
	rd_var * v;
	if(type == TYPE_SINT){
		return rd_vlist_var(rdgen_vlist, id);
	}else if(type == TYPE_FLOAT){
		if(rdgen_float_table[id] != ~0){
			return rd_vlist_mem_find(rdgen_memlist, rdgen_float_table[id]);
		}else{
			v = rdgen_memvar(RD_MEM_FLOAT);
			rdgen_float_table[id] = v->id;
			return v;
		}
	}else if(type == TYPE_STRING){
		return rd_vlist_var(rdgen_vlist, id);
	}else{
		return 0;
	}
}

void rdgen_start(unsigned int num_vars, rd_vlist * const_list){
	rdgen_vlist = rd_varlist(num_vars); //create the var list
	rdgen_memlist = rd_varlist(0); //create the memory list
	rdgen_constlist = const_list; //the constant list
	rdgen_section(rdgen_sec_pos++); //create main section
	rdgen_first_instr = rdgen_current_instr;
	rdgen_float_table = new unsigned int [num_vars];
	for(unsigned int i = 0; i < num_vars; i++){
		rdgen_float_table[i] = ~0;
	}
}


void rdgen(tree * code){
	rdgen_recur(code);
}

void rdgen_end(){
	delete [] rdgen_float_table;
}


//add post increment/decrement operations
void rdgen_add_inc_list(rd_var * v, bool inc){
	rdg_inc_list * i = new rdg_inc_list;
	if(rdg_ilist != 0){
		rdg_ilist->next = i;
	}else{
		rdg_ilist = i;
	}
	i->next = 0;
	i->var = v;
	i->inc = inc;
}

void rdgen_clear_inc_list(){
	rdg_inc_list * i;
	while(rdg_ilist != 0){
		if(rdg_ilist->inc){
			rdgen_instruction(RD_INC, rdg_ilist->var, 0, 0);
		}else{
			rdgen_instruction(RD_DEC, rdg_ilist->var, 0, 0);
		}
		i = rdg_ilist;
		rdg_ilist = rdg_ilist->next;
		delete i;
	}
}




//generates the correct instructions for the comparison
rd_instr * rdgen_cond_cmp(tree * cond){
	rd_var * tmp = rdgen_tmpvar();
	rd_instr * ins;
	switch(cond->obj_type){
		case TREE_ISEQUAL:
		case TREE_NOTEQUAL:
		case TREE_GT:
		case TREE_LT:
		case TREE_GTOREQUAL:
		case TREE_LTOREQUAL:
			ins = rd_new_instruction(RD_SET, tmp, rdgen_recur(cond->left), 0, 0);
			ins = rd_new_instruction(RD_CMP, tmp, rdgen_recur(cond->right), 0, ins);
			break;
		default:
			ins = rd_new_instruction(RD_SET, tmp, rdgen_recur(cond), 0, 0);
			ins = rd_new_instruction(RD_CMP, tmp, rd_sint(0), 0, ins);
	}
	tmp->count = 2 << (3 * rdgen_loop_level);
	tmp->instances = 2;
	return ins;
}

unsigned int rdgen_cond_jump_type(tree * cond){
	switch(cond->obj_type){
		case TREE_ISEQUAL:
			return RD_JE;
		case TREE_NOTEQUAL:
			return RD_JNE;
		case TREE_GT:
			return RD_JG;
		case TREE_LT:
			return RD_JL;
		case TREE_GTOREQUAL:
			return RD_JGE;
		case TREE_LTOREQUAL:
			return RD_JLE;
		default:
			return RD_JNE;
	}
}

unsigned int rdgen_cond_opposite(unsigned int jump_type){
	switch(jump_type){
		case RD_JE:
			return RD_JNE;
		case RD_JNE:
			return RD_JE;
		case RD_JG:
			return RD_JLE;
		case RD_JL:
			return RD_JGE;
		case RD_JGE:
			return RD_JL;
		case RD_JLE:
			return RD_JG;
	}
}

rdgen_cond * rdgen_basic_cond(rd_instr * cmp_ins, unsigned int jump_type){
	rdgen_cmp * cmp = new rdgen_cmp;
	cmp->cmp_ins = cmp_ins;
	cmp->jump_type = jump_type;
	cmp->sign = true;
	cmp->jmp_id = 0;
	cmp->prev = cmp->next = 0;
	rdgen_cond * cond = new rdgen_cond;
	cond->start = cond->end = cmp;
	return cond;
}

bool rdgen_cond_isbasic(rdgen_cond * cond){
	if(cond->start == cond->end){
		return true;
	}else{
		return false;
	}
}

void rdgen_cond_add(rdgen_cond * cond, rdgen_cmp * cmp){
	if(cond->start == 0){
		cond->start = cond->end = cmp;
		cmp->prev = 0;
	}else{
		cmp->prev = cond->end;
		cond->end->next = cmp;
		cond->end = cmp;
	}
	cmp->next = 0;
}

void rdgen_cond_combine(rdgen_cond * cond, rdgen_cond * add){
	rdgen_cmp * cmp = add->start;
	rdgen_cmp * next;
	while(cmp != 0){
		next = cmp->next;
		rdgen_cond_add(cond, cmp);
		cmp = next;
	}
	add->start = add->end = 0;
}

void rdgen_cond_mark(rdgen_cond * cond, unsigned int id, bool sign){
	rdgen_cmp * cmp = cond->end;
	if(cmp->sign == sign){
		cmp->sign = !sign;
		cmp->jmp_id = 0;
	}
	cmp = cmp->prev;
	while(cmp != 0){
		if(cmp->jmp_id == 0 && cmp->sign == sign){
			cmp->jmp_id = id;
		}
		cmp = cmp->prev;
	}
	cmp = new rdgen_cmp;
	cmp->cmp_ins = 0;
	cmp->jump_type = 0;
	cmp->jmp_id = id;
	cmp->sign = sign;
	cmp->prev = cmp->next = 0;
	rdgen_cond_add(cond, cmp);
}

unsigned int rdgen_cond_count(rdgen_cond * cond, bool sign){
	rdgen_cmp * cmp = cond->end->prev;
	unsigned int count = 0;
	while(cmp != 0){
		if(cmp->sign == sign && cmp->jmp_id == 0 && cmp->cmp_ins != 0){
			count++;
		}
		cmp = cmp->prev;
	}
	return count;
}

rdgen_cond * rdgen_cond_and(rdgen_cond * a, rdgen_cond * b, unsigned int * num_marks){
	unsigned int count_a, count_b;
	if(rdgen_cond_isbasic(a) && rdgen_cond_isbasic(b)){
		a->start->sign = false;
		b->start->sign = false;
		rdgen_cond_combine(a, b);
		return a;
	}else if(rdgen_cond_isbasic(a)){
		a->start->sign = false;
		rdgen_cond_combine(a, b);
		return a;
	}else if(rdgen_cond_isbasic(b)){
		b->start->sign = false;
		rdgen_cond_combine(b, a);
		return b;
	}else{
		count_a = rdgen_cond_count(a, true);
		count_b = rdgen_cond_count(b, true);
		if(count_a > count_b){
			if(count_b != 0){
				rdgen_cond_mark(b, (*num_marks)++, true);
			}
			rdgen_cond_combine(b, a);
			return b;
		}else{
			if(count_a != 0){
				rdgen_cond_mark(a, (*num_marks)++, true);
			}
			rdgen_cond_combine(a, b);
			return a;
		}
	}
}

rdgen_cond * rdgen_cond_or(rdgen_cond * a, rdgen_cond * b, unsigned int * num_marks){
	unsigned int count_a, count_b;
	if(rdgen_cond_isbasic(a) && rdgen_cond_isbasic(b)){
		rdgen_cond_combine(a, b);
		return a;
	}else if(rdgen_cond_isbasic(a)){
		rdgen_cond_combine(a, b);
		return a;
	}else if(rdgen_cond_isbasic(b)){
		rdgen_cond_combine(b, a);
		return b;
	}else{
		count_a = rdgen_cond_count(a, false);
		count_b = rdgen_cond_count(b, false);
		if(count_a > count_b){
			if(count_b != 0){
				rdgen_cond_mark(b, (*num_marks)++, false);
			}
			rdgen_cond_combine(b, a);
			return b;
		}else{
			if(count_a != 0){
				rdgen_cond_mark(a, (*num_marks)++, false);
			}
			rdgen_cond_combine(a, b);
			return a;
		}
	}
}

rdgen_cond * rdgen_cond_not(rdgen_cond * cond){
	rdgen_cmp * cmp = cond->start;
	while(cmp != 0){
		if(cmp->cmp_ins != 0){
			cmp->jump_type = rdgen_cond_opposite(cmp->jump_type);
			cmp->sign = !(cmp->sign);
		}else{
			cmp->sign = !(cmp->sign);
		}
		cmp = cmp->next;
	}
	return cond;
}

rdgen_cond * rdgen_condition(tree * cond, unsigned int * num_marks){
	switch(cond->obj_type){
		case TREE_LOG_AND:
			return rdgen_cond_and(rdgen_condition(cond->left, num_marks), rdgen_condition(cond->right, num_marks), num_marks);
		case TREE_LOG_OR:
			return rdgen_cond_or(rdgen_condition(cond->left, num_marks), rdgen_condition(cond->right, num_marks), num_marks);
		case TREE_NOT:
			return rdgen_cond_not(rdgen_condition(cond->left, num_marks));
		case TREE_BRACKET:
			return rdgen_condition(cond->left, num_marks);
		default:
			return rdgen_basic_cond(rdgen_cond_cmp(cond), rdgen_cond_jump_type(cond));
	}
}

void rdgen_cmps(tree * tree_cond, unsigned int false_pos){
	unsigned int pos;
	unsigned int * num_marks = new unsigned int;
	*num_marks = 1;
	rdgen_cond * cond = rdgen_condition(tree_cond, num_marks);
	rdgen_cond_mark(cond, 0, true);
	pos = rdgen_sec_pos;
	rdgen_sec_pos += *num_marks; //allocate space for all the marks
	rdgen_cmp * cmp = cond->start;
	while(cmp != 0){
		if(cmp->cmp_ins != 0){
			rdgen_current_instr->next = cmp->cmp_ins->prev;
			rdgen_current_instr->next->prev = rdgen_current_instr;
			rdgen_current_instr = cmp->cmp_ins;
			if(cmp->sign == true){
				rdgen_instruction(cmp->jump_type, rd_section(pos + cmp->jmp_id), 0, 0);
			}else{
				if(cmp->jmp_id == 0){
					rdgen_instruction(rdgen_cond_opposite(cmp->jump_type), rd_section(false_pos), 0, 0);
				}else{
					rdgen_instruction(rdgen_cond_opposite(cmp->jump_type), rd_section(pos + cmp->jmp_id), 0, 0);
				}
			}
		}else{
			if(cmp->jmp_id){
				rdgen_clear_inc_list();
			}
			rdgen_section(pos + cmp->jmp_id);
		}
		cmp = cmp->next;
	}
	delete num_marks;
}

rd_var * rdgen_recur(tree * code){
	rd_var * res, * v; //result variable
	unsigned int pos;

	if(code == NULL){
		return 0;
	}else if(code->obj_type == TREE_IFSTMT){
		if(code->cond != 0){ //it is an If/Else If statement
			if(code->right != 0){ //test to see if there is a following else if/else statement
				pos = rdgen_sec_pos;
				rdgen_sec_pos += 2; //create ON_FALSE and AFTER sections

				rdgen_cmps(code->cond, pos);

				rdgen_recur(code->left); //compile ON_TRUE code

				rdgen_instruction(RD_JUMP, rd_section(pos+1), 0, 0); //JUMP to AFTER section

				rdgen_section(pos); //start ON_FALSE section

				rdgen_recur(code->right); //compile ON_FALSE code

				rdgen_section(pos+1); //start AFTER section

				return 0; //we are finished
			}else{
				pos = rdgen_sec_pos;
				rdgen_sec_pos += 1; //create AFTER section

				rdgen_cmps(code->cond, pos);

				rdgen_recur(code->left); //compile ON_TRUE code

				rdgen_section(pos); //start AFTER section

				return 0; //we are finished
			}
		}else{ //it is an Else statement
			rdgen_recur(code->left);
		}
	}else if(code->obj_type == TREE_WHILESTMT){
		pos = rdgen_sec_pos;
		rdgen_sec_pos += 2; //we want 2 sections (the start and end of the while loop)
		
		//while start
		rdgen_section(pos);

		rdgen_instruction(RD_LOOPSTART, 0, 0, 0);

		rdgen_loop_level++;

		//compile condition
		rdgen_cmps(code->cond, pos+1);

		 //compile loop code
		rdgen_recur(code->left);

		rdgen_loop_level--;

		rdgen_instruction(RD_LOOPEND, 0, 0, 0);

		//jump to start of loop
		rdgen_instruction(RD_JUMP, rd_section(pos), 0, 0);
		
		//while end
		rdgen_section(pos+1);

		return 0;
	}else if(code->obj_type == TREE_FORSTMT){
		pos = rdgen_sec_pos;
		rdgen_sec_pos += 2; //we want 2 sections (the start and end of the while loop)

		//compile start code
		rdgen_recur(code->left);

		rdgen_clear_inc_list();
		
		//while start
		rdgen_section(pos);

		rdgen_instruction(RD_LOOPSTART, 0, 0, 0);

		rdgen_loop_level++;

		//compile condition
		rdgen_cmps(code->cond, pos+1);

		 //compile loop code + end code
		rdgen_recur(code->right);

		rdgen_clear_inc_list();

		rdgen_loop_level--;

		rdgen_instruction(RD_LOOPEND, 0, 0, 0);

		//jump to start of loop
		rdgen_instruction(RD_JUMP, rd_section(pos), 0, 0);
		
		//while end
		rdgen_section(pos+1);

		return 0;
	}else if(code->obj_type == TREE_FNCALL){
		res = rdgen_tmpvar();
		pos = rdgen_param_num;
		rdgen_param_num  = 0;
		rdgen_recur(code->left); //make parameters
		rdgen_instruction(RD_CALL, rd_name(code->stringdata), res, 0); //create the CALL instruction
		if(rdgen_param_num > 0){
			rdgen_instruction(RD_CLRPARAM, rd_sint(rdgen_param_num), 0, 0);
		}
		rdgen_param_num = pos;
		return res;
	}else if(code->obj_type == TREE_CALLPARAM){
		rdgen_recur(code->right);
		res = rdgen_recur(code->left);
		rdgen_instruction(RD_PARAM, res, 0, 0);
		rdgen_param_num++;
		return 0;
	}else if(code->obj_type == TREE_SPECPARAM){
		rdgen_recur(code->right);
		rdgen_instruction(RD_GETPARAM, rdgen_recur(code->left), rd_sint(code->intdata), 0);
		return 0;
	}else if(code->obj_type == TREE_CODELINE){
		rdgen_recur(code->right);
		rdgen_recur(code->left);
		rdgen_clear_inc_list();
		return 0;
	}else if(code->obj_type == TREE_VAR){
		//printf("Var %u has type %u\n", code->intdata, code->val_type);
		return rdgen_var(code->intdata, code->val_type);
	}else if(code->obj_type == TREE_INT){
		return rd_sint(code->intdata);
	}else if(code->obj_type == TREE_FLT){
		return rd_vlist_const_float(code->floatdata, rdgen_constlist);
	}else if(code->obj_type == TREE_STR){
		func_include("string_copy");
		rdgen_instruction(RD_PARAM, rd_vlist_const_string(code->stringdata, rdgen_constlist), 0, 0);
		res = rdgen_tmpvar();
		rdgen_instruction(RD_CALL, rd_name("string_copy"), res, 0);
		rdgen_instruction(RD_CLRPARAM, rd_sint(1), 0, 0);
		return res;
	}else if(code->obj_type == TREE_TYPECONV){
		if(code->val_type == TYPE_SINT && code->left->val_type == TYPE_FLOAT){
			res = rdgen_tmpvar();
			v = rdgen_memvar(RD_MEM_SINT);
			rdgen_instruction(RD_FPUSH, rdgen_recur(code->left), 0, 0);
			rdgen_instruction(RD_FPOPI, v, 0, 0);
			rdgen_instruction(RD_SET, res, v, 0);
		}else if(code->val_type == TYPE_FLOAT && code->left->val_type == TYPE_SINT){
			res = rdgen_memvar(RD_MEM_FLOAT);
			v = rdgen_memvar(RD_MEM_SINT);
			rdgen_instruction(RD_SET, v, rdgen_recur(code->left), 0);
			rdgen_instruction(RD_FPUSHI, v, 0, 0);
			rdgen_instruction(RD_FPOP, res, 0, 0);
		}else if(code->val_type == TYPE_SINT && code->left->val_type == TYPE_STRING){
			res = rdgen_tmpvar();
			func_include("string_to_int");
			rdgen_instruction(RD_PARAM, rdgen_recur(code->left), 0, 0);
			rdgen_instruction(RD_CALL, rd_name("string_to_int"), res, 0);
			rdgen_instruction(RD_CLRPARAM, rd_sint(1), 0, 0);
		}else if(code->val_type == TYPE_STRING && code->left->val_type == TYPE_SINT){
			res = rdgen_tmpvar();
			func_include("string_from_int");
			rdgen_instruction(RD_PARAM, rdgen_recur(code->left), 0, 0);
			rdgen_instruction(RD_CALL, rd_name("string_from_int"), res, 0);
			rdgen_instruction(RD_CLRPARAM, rd_sint(1), 0, 0);
		}else if(code->val_type == TYPE_SINT && code->left->val_type == TYPE_NONE){
			res = rdgen_tmpvar();
			rdgen_instruction(RD_SET, res, rd_sint(0), 0);
		}else if(code->val_type == TYPE_FLOAT && code->left->val_type == TYPE_NONE){
			res = rdgen_memvar(RD_MEM_FLOAT);
			rdgen_instruction(RD_FPUSHI, rd_sint(0), 0, 0);
			rdgen_instruction(RD_FPOP, res, 0, 0);
		}else if(code->val_type == TYPE_STRING && code->left->val_type == TYPE_NONE){
			func_include("string_new");
			res = rdgen_tmpvar();
			rdgen_instruction(RD_PARAM, rd_sint(0), 0, 0);
			rdgen_instruction(RD_CALL, rd_name("string_new"), res, 0);
			rdgen_instruction(RD_CLRPARAM, rd_sint(1), 0, 0);
		}
		return res;
	}else if(code->obj_type > TREE_DUAL){
		switch(code->obj_type){
			case TREE_ADD:
				if(code->val_type == TYPE_SINT){
					res = rdgen_tmpvar();
					rdgen_instruction(RD_SET, res, rdgen_recur(code->left), 0);
					rdgen_instruction(RD_ADD, res, rdgen_recur(code->right), 0);
				}else if(code->val_type == TYPE_FLOAT){
					res = rdgen_memvar(RD_MEM_FLOAT);
					rdgen_instruction(RD_SET, res, rdgen_recur(code->left), 0);
					rdgen_instruction(RD_FADD, res, rdgen_recur(code->right), 0);
				}else if(code->val_type == TYPE_STRING){
					func_include("string_concat");
					res = rdgen_tmpvar();
					rdgen_instruction(RD_PARAM, rdgen_recur(code->right), 0, 0);
					rdgen_instruction(RD_PARAM, rdgen_recur(code->left), 0, 0);
					rdgen_instruction(RD_CALL, rd_name("string_concat"), res, 0);
					rdgen_instruction(RD_CLRPARAM, rd_sint(2), 0, 0);
				}else{
					res = rdgen_tmpvar();
					rdgen_instruction(RD_SET, res, rd_sint(0), 0);
				}
				return res;
			case TREE_SUBTRACT:
				if(code->val_type == TYPE_SINT){
					res = rdgen_tmpvar();
					rdgen_instruction(RD_SET, res, rdgen_recur(code->left), 0);
					rdgen_instruction(RD_SUB, res, rdgen_recur(code->right), 0);
				}else if(code->val_type == TYPE_FLOAT){
					res = rdgen_memvar(RD_MEM_FLOAT);
					rdgen_instruction(RD_SET, res, rdgen_recur(code->left), 0);
					rdgen_instruction(RD_FSUB, res, rdgen_recur(code->right), 0);
				}
				return res;
			case TREE_MULTIPLY:
				if(code->val_type == TYPE_SINT){
					res = rdgen_tmpvar();
					rdgen_instruction(RD_SET, res, rdgen_recur(code->left), 0);
					rdgen_instruction(RD_MUL, res, rdgen_recur(code->right), 0);
				}else if(code->val_type == TYPE_FLOAT){
					res = rdgen_memvar(RD_MEM_FLOAT);
					rdgen_instruction(RD_SET, res, rdgen_recur(code->left), 0);
					rdgen_instruction(RD_FMUL, res, rdgen_recur(code->right), 0);
				}
				return res;
			case TREE_DIVIDE:
				if(code->val_type == TYPE_SINT){
					res = rdgen_tmpvar();
					rdgen_instruction(RD_SET, res, rdgen_recur(code->left), 0);
					rdgen_instruction(RD_DIV, res, rdgen_recur(code->right), 0);
				}else if(code->val_type == TYPE_FLOAT){
					res = rdgen_memvar(RD_MEM_FLOAT);
					rdgen_instruction(RD_SET, res, rdgen_recur(code->left), 0);
					rdgen_instruction(RD_FDIV, res, rdgen_recur(code->right), 0);
				}
				return res;
			case TREE_EQUALS:
				res = rdgen_recur(code->left);
				rdgen_instruction(RD_SET, res, rdgen_recur(code->right), 0);
				return res;
			case TREE_ADDEQUALS:
				res = rdgen_recur(code->left);
				rdgen_instruction(RD_ADD, res, rdgen_recur(code->right), 0);
				return res;
			case TREE_SUBEQUALS:
				res = rdgen_recur(code->left);
				rdgen_instruction(RD_SUB, res, rdgen_recur(code->right), 0);
				return res;
			case TREE_MULEQUALS:
				res = rdgen_recur(code->left);
				rdgen_instruction(RD_MUL, res, rdgen_recur(code->right), 0);
				return res;
			case TREE_DIVEQUALS:
				res = rdgen_recur(code->left);
				rdgen_instruction(RD_DIV, res, rdgen_recur(code->right), 0);
				return res;
			case TREE_COMMA:
				rdgen_recur(code->left);
				return rdgen_recur(code->right);
			case TREE_ISEQUAL:
			case TREE_GT:
			case TREE_LT:
			case TREE_GTOREQUAL:
			case TREE_LTOREQUAL:
			case TREE_NOTEQUAL:
			case TREE_LOG_AND:
			case TREE_LOG_OR:
				pos = rdgen_sec_pos;
				rdgen_sec_pos += 2;

				res = rdgen_tmpvar();
				
				rdgen_cmps(code, pos);

				//set result = 1 (true)
				rdgen_instruction(RD_SET, res, rd_sint(1), 0);

				//jump to after section
				rdgen_instruction(RD_JUMP, rd_section(pos+1), 0, 0);

				//start FALSE section
				rdgen_section(pos);

				//set result = 0 (false)
				rdgen_instruction(RD_SET, res, rd_sint(0), 0);

				//start AFTER section
				rdgen_section(pos+1);
				
				return res;
			case TREE_SUBSCR:
				res = rdgen_tmpvar();
				rdgen_instruction(RD_SUBSCR, res, rdgen_recur(code->left), rdgen_recur(code->right));
				return res;
			case TREE_SUBSCR_SET:
				res = rdgen_tmpvar();
				rdgen_instruction(RD_SET, res, rdgen_recur(code->right), 0);
				rdgen_instruction(RD_SUBSCR_SET, rdgen_recur(code->left), rdgen_recur(code->cond), res);
				return res;
		}
		return 0;
	}else if(code->obj_type > TREE_UNARY){
		switch(code->obj_type){
			case TREE_BRACKET:
				res = rdgen_recur(code->left);
				return res;
			case TREE_RETURN:
				res = rdgen_tmpvar();
				rdgen_instruction(RD_SET, res, rdgen_recur(code->left), 0);
				rdgen_instruction(RD_RETURN, res, 0, 0);
				return 0;
			case TREE_PREINC:
				res = rdgen_recur(code->left);
				rdgen_instruction(RD_INC, res, 0, 0);
				return res;
			case TREE_PREDEC:
				res = rdgen_recur(code->left);
				rdgen_instruction(RD_DEC, res, 0, 0);
				return res;
			case TREE_POSTINC:
				res = rdgen_recur(code->left);
				rdgen_add_inc_list(res, true);
				return res;
			case TREE_POSTDEC:
				res = rdgen_recur(code->left);
				rdgen_add_inc_list(res, false);
				return res;
			case TREE_NEG:
				res = rdgen_recur(code->left);
				if(res->type == RD_CONST_SINT){
					res->sint *= -1;
					return res;
				}else if(res->type == RD_CONST_FLOAT){
					res->fval *= -1;
					return res;
				}
				res = rdgen_tmpvar();
				rdgen_instruction(RD_SET, res, rdgen_recur(code->left), 0);
				rdgen_instruction(RD_NEG, res, 0, 0);
				return res;
			case TREE_NOT:
				pos = rdgen_sec_pos;
				rdgen_sec_pos += 2;

				res = rdgen_tmpvar();
				
				rdgen_cmps(code, pos);

				//set result = 1 (true)
				rdgen_instruction(RD_SET, res, rd_sint(1), 0);

				//jump to after section
				rdgen_instruction(RD_JUMP, rd_section(pos+1), 0, 0);

				//start FALSE section
				rdgen_section(pos);

				//set result = 0 (false)
				rdgen_instruction(RD_SET, res, rd_sint(0), 0);

				//start AFTER section
				rdgen_section(pos+1);
				
				return res;
		}
		return 0;
	}else{
		return 0;
	}
}

void rdgen_showvar(rd_var * v){
	if(v == 0){
		printf("NULL");
		return;
	}
	switch(v->type){
		case RD_VAR:
			printf("v%u", v->id);
			return;
		case RD_REG:
			printf("reg%u", v->id);
			return;
		case RD_SEC:
			printf("{Section %u}", v->id);
			return;
		case RD_MEM_SINT:
			printf("sint_mem%u", v->id);
			return;
		case RD_CONST_SINT:
			printf("%u", v->sint);
			return;
		case RD_MEM_FLOAT:
			printf("float_mem%u", v->id);
			return;
		case RD_CONST_STR:
			printf("STRING{%s}", v->str);
			return;
		case RD_CONST_NAME:
			printf("%s", v->str);
			return;
		default:
			printf("?");
	}
}

void rdgen_show(){
	//Shorten to local variables for simplicity
	rd_instr * ins = rdgen_first_instr;

	puts("START");

	while(ins != 0){
		if(ins->type == RD_SECTION){
			printf("Section %u", ins->p1->id);
			puts("");
			ins = ins->next;
			continue;
		}
		printf("    ");
		switch(ins->type){
			case RD_SET:
				printf("set ");
				break;
			case RD_ADD:
				printf("add ");
				break;
			case RD_SUB:
				printf("sub ");
				break;
			case RD_MUL:
				printf("mul ");
				break;
			case RD_INC:
				printf("inc ");
				break;
			case RD_DEC:
				printf("dec ");
				break;
			case RD_DIV:
				printf("div ");
				break;
			case RD_NOT:
				printf("not ");
				break;
			case RD_CMP:
				printf("cmp ");
				break;
			case RD_JE:
				printf("je ");
				break;
			case RD_JNE:
				printf("jne ");
				break;
			case RD_JG:
				printf("jg ");
				break;
			case RD_JL:
				printf("jl ");
				break;
			case RD_JGE:
				printf("jge ");
				break;
			case RD_JLE:
				printf("jle ");
				break;
			case RD_JUMP:
				printf("jump ");
				break;
			case RD_CALL:
				printf("call ");
				break;
			case RD_PARAM:
				printf("param ");
				break;
			case RD_CLRPARAM:
				printf("clrparam ");
				break;
			case RD_GETPARAM:
				printf("getparam ");
				break;
			case RD_FADD:
				printf("fadd ");
				break;
			case RD_FPUSH:
				printf("fpush ");
				break;
			case RD_FPUSHI:
				printf("fpushi ");
				break;
			case RD_FPOP:
				printf("fpop ");
				break;
			case RD_FPOPI:
				printf("fpopi ");
				break;
			default:
				printf("? ");
		}
		rdgen_showvar(ins->p1);
		printf(", ");
		rdgen_showvar(ins->p2);
		printf(", ");
		rdgen_showvar(ins->p3);
		puts("");
		ins = ins->next;
	}

	puts("END");
}
