#include "reduced.h"
#include <iostream>

rd_instr * rd_new_instruction(unsigned int type, rd_var * p1, rd_var * p2, rd_var * p3, rd_instr * prev){
	rd_instr * i = new rd_instr;
	i->type = type;
	i->p1 = p1;
	i->p2 = p2;
	i->p3 = p3;
	i->prev = prev;
	i->next = 0;
	if(prev != 0){
		prev->next = i;
	}
	return i;
}

void rd_remove_instruction(rd_instr * ins){
	if(ins == 0){
		return;
	}
	if(ins->prev != 0){
		ins->prev->next = ins->next;
	}
	if(ins->next != 0){
		ins->next->prev = ins->prev;
	}
	delete ins;
}


rd_vlist * rd_varlist(unsigned int num_vars){
	rd_vlist * vlist = new rd_vlist;
	vlist->start = vlist->end = 0;
	vlist->num_vars = num_vars;
	return vlist;
}

void rd_vlist_add(rd_vlist * vlist, rd_var * v){
	v->prev = v->next = 0;
	if(vlist->start == 0){
		vlist->start = vlist->end = v;
	}else{
		vlist->end->next = v;
		v->prev = vlist->end;
		vlist->end = v;
	}
}

void rd_vlist_add(rd_vlist * vlist, unsigned int id, unsigned int type, unsigned int count, unsigned int instances, unsigned int size){
	rd_var * v = new rd_var;
	v->id = id;
	v->type = type;
	v->count = count;
	v->instances = instances;
	v->size = size;
	rd_vlist_add(vlist, v);
}

rd_var * rd_vlist_find(rd_vlist * vlist, unsigned int id){
	rd_var * v;
	v = vlist->start;
	while(v != 0){
		if(v->id == id){
			return v;
		}
		v = v->next;
	}
	return 0;
}

rd_var * rd_vlist_var(rd_vlist * vlist, unsigned int id){
	rd_var * v = rd_vlist_find(vlist, id);
	if(v != 0){
		return v;
	}
	v = new rd_var;
	v->prev = v->next = 0;
	v->type = RD_VAR;
	v->id = id;
	v->count = 0;
	v->instances = 0;
	v->size = SIZE_REG;
	rd_vlist_add(vlist, v);
	return v;
}

rd_var * rd_vlist_tmp(rd_vlist * vlist){
	rd_var * v = new rd_var;
	v->prev = v->next = 0;
	v->type = RD_VAR;
	v->id = vlist->num_vars++;
	v->count = 0;
	v->instances = 0;
	v->size = SIZE_REG;
	rd_vlist_add(vlist, v);
	return v;
}

rd_var * rd_vlist_mem_find(rd_vlist * vlist, unsigned int mem_id){
	rd_var * v = vlist->start;
	while(v != 0){
		if(v->id == mem_id){
			return v;
		}
		v = v->next;
	}
	return 0;
}

rd_var * rd_vlist_mem(rd_vlist * vlist, unsigned int type){
	rd_var * v = new rd_var;
	v->prev = v->next = 0;
	v->type = type;
	v->id = vlist->num_vars++;
	v->count = 0;
	v->instances = 0;
	if(type == RD_MEM_FLOAT){
		v->size = SIZE_FLOAT;
	}else{
		v->size = SIZE_REG;
	}
	rd_vlist_add(vlist, v);
	return v;
}

rd_var * rd_vlist_const_float(double fval, rd_vlist * vlist){
	rd_var * v = new rd_var;
	v->prev = v->next = 0;
	v->type = RD_CONST_FLOAT;
	v->id = vlist->num_vars++;
	v->fval = fval;
	v->count = 0;
	v->instances = 0;
	v->size = SIZE_FLOAT;
	rd_vlist_add(vlist, v);
	return v;
}

rd_var * rd_vlist_const_string(const char * sval, rd_vlist * vlist){
	rd_var * v = new rd_var;
	v->prev = v->next = 0;
	v->type = RD_CONST_STR;
	v->id = vlist->num_vars++;
	v->str = sval;
	v->count = 0;
	v->instances = 0;
	v->size = SIZE_CHAR;
	rd_vlist_add(vlist, v);
	return v;
}

rd_var * rd_mem_sint(unsigned int id){
	rd_var * v = new rd_var;
	v->prev = v->next = 0;
	v->type = RD_MEM_SINT;
	v->id = id;
	v->count = 0;
	v->instances = 0;
	v->size = SIZE_REG;
	return v;
}

void rd_deref(rd_var * v, signed int offset){
	v->type = RD_MEM_DEREF;
	v->sint = offset;
}

rd_var * rd_register(unsigned int id){
	rd_var * v = new rd_var;
	v->type = RD_REG;
	v->id = id;
	v->count = 0;
	v->instances = 0;
	v->size = SIZE_REG;
	v->prev = v->next = 0;
	return v;
}

rd_var * rd_section(unsigned int id){
	rd_var * v = new rd_var;
	v->type = RD_SEC;
	v->id = id;
	v->count = 0;
	v->instances = 0;
	v->prev = v->next = 0;
	v->size = 0;
	return v;
}

rd_var * rd_name(const char * name){
	rd_var * v = new rd_var;
	v->type = RD_CONST_NAME;
	v->str = name;
	v->count = 0;
	v->instances = 0;
	v->prev = v->next = 0;
	v->size = 0;
	return v;
}

rd_var * rd_str(char * str){
	rd_var * v = new rd_var;
	v->type = RD_CONST_STR;
	v->str = str;
	v->count = 0;
	v->instances = 0;
	v->prev = v->next = 0;
	v->size = SIZE_CHAR;
	return v;
}

rd_var * rd_sint(signed int sint){
	rd_var * v = new rd_var;
	v->type = RD_CONST_SINT;
	v->sint = sint;
	v->count = 0;
	v->instances = 0;
	v->prev = v->next = 0;
	v->size = SIZE_SINT;
	return v;
}

bool rd_is_mem(rd_var * var){
	if(var->type == RD_MEM_SINT || var->type == RD_MEM_STR || var->type == RD_MEM_FLOAT || var->type == RD_MEM_DEREF){
		return true;
	}else{
		return false;
	}
}

bool rd_equal(rd_var * a, rd_var * b){
	if(a->type != b->type){
		return false;
	}
	switch(a->type){
		case RD_VAR:
		case RD_REG:
		case RD_SEC:
		case RD_MEM_SINT:
		case RD_MEM_FLOAT:
		case RD_MEM_STR:
			if(a->id == b->id){
				return true;
			}
			break;
	}
	return false;
}
