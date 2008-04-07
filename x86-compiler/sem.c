#include "sem.h"
#include "tree.h"
#include "func.h"
#include "var.h"
#include <stdio.h>

sem_error * sem_error_list = 0;
func * sem_current_func = 0;
unsigned int sem_line_num = 1;
var * var_list;

void sem_error_(const char * msg){
	sem_error * err = new sem_error;
	err->msg = msg;
	err->line_num = sem_line_num;
	err->prev = sem_error_list;
	err->next = 0;
	if(sem_error_list != 0){
		sem_error_list->next = err;
	}
	sem_error_list = err;
}

tree * sem_set_type(tree * code, unsigned int val_type){
	if(code == 0){
		return 0;
	}
	if(code->val_type != val_type){
		code = build_typeconv(code, val_type);
	}
	return code;
}

tree * sem_var_type(tree * code, unsigned int val_type){
	if(code == 0){
		return 0;
	}
	if(code->obj_type != TREE_VAR){
		return code;
	}
	var * v = var_find(var_list, code->intdata);
	if(v != 0){
		if(v->type == TYPE_NONE){
			v->type = val_type;
			code->val_type = val_type;
		}
	}
	return code;
}

tree * sem_var(tree * code){
	var * v = var_find(var_list, code->stringdata);
	if(v != 0){
		code->intdata = v->id;
		code->val_type = v->type;
	}else{
		var_list = var_add(var_list, code->stringdata, TYPE_NONE);
		code->intdata = var_list->id;
		code->val_type = TYPE_NONE;
	}
	return code;
}

tree * sem_recur(tree * code){
	unsigned int i;
	tree * t, * p;
	func * f;

	if(code == 0){
		return 0;
	}else if(code->obj_type == TREE_IFSTMT){
		if(code->cond != 0){
			code->cond = sem_recur(code->cond);
		}
		if(code->right != 0){
			code->right = sem_recur(code->right);
		}
		code->left = sem_recur(code->left);
		code->val_type = TYPE_NONE;
	}else if(code->obj_type == TREE_WHILESTMT){
		code->left = sem_recur(code->left);
		code->cond = sem_recur(code->cond);
		code->val_type = TYPE_NONE;
	}else if(code->obj_type == TREE_FORSTMT){
		code->cond = sem_recur(code->cond);
		code->left = sem_recur(code->left);
		code->right = sem_recur(code->right);
		code->val_type = TYPE_NONE;
	}else if(code->obj_type == TREE_FNCALL){
		i = 0;
		t = code->left;
		while(t != 0){
			i++;
			t = t->right;
		}

		f = func_get(code->stringdata);
		if(f == 0){
			sem_error_("Function doesn't exist");
			return code;
		}else if(f->num_param != i){
			sem_error_("Function called with wrong number of parameters");
			return code;
		}

		t = code->left;
		p = (tree *) f->param;
		while(t != 0){
			t->left = sem_set_type(sem_recur(t->left), p->val_type);
			p = p->right;
			t = t->right;
		}

		code->val_type = f->ret_type;
	}else if(code->obj_type == TREE_CALLPARAM){
		code->left = sem_recur(code->left);
		code->val_type = code->left->val_type;
	}else if(code->obj_type == TREE_SPECPARAM){
		code->right = sem_recur(code->right);
		code->left = sem_var_type(sem_recur(code->left), code->val_type);
	}else if(code->obj_type == TREE_CODELINE){
		code->right = sem_recur(code->right);
		sem_line_num = code->intdata;
		code->left = sem_recur(code->left);
		code->val_type = TYPE_NONE;
	}else if(code->obj_type == TREE_TYPECONV){
		code->left = sem_recur(code->left);
		if(code->left->val_type == code->val_type){
			return code->left;
		}
	}else if(code->obj_type == TREE_VAR){
		code = sem_var(code);
	}else if(code->obj_type == TREE_INT){
		code->val_type = TYPE_SINT;
	}else if(code->obj_type == TREE_FLT){
		code->val_type = TYPE_FLOAT;
	}else if(code->obj_type == TREE_STR){
		code->val_type = TYPE_STRING;
	}else if(code->obj_type > TREE_DUAL){
		switch(code->obj_type){
			case TREE_ADD:
			case TREE_SUBTRACT:
			case TREE_MULTIPLY:
			case TREE_DIVIDE:
				code->left = sem_recur(code->left);
				code->right = sem_recur(code->right);
				if(code->obj_type == TREE_ADD && code->left->val_type == TYPE_STRING && code->right->val_type == TYPE_STRING){
					code->val_type = TYPE_STRING;
				}else if(code->left->val_type == TYPE_FLOAT || code->right->val_type == TYPE_FLOAT){
					code->left = sem_set_type(code->left, TYPE_FLOAT);
					code->right = sem_set_type(code->right, TYPE_FLOAT);
					code->val_type = TYPE_FLOAT;
				}else{
					code->left = sem_set_type(code->left, TYPE_SINT);
					code->right = sem_set_type(code->right, TYPE_SINT);
					code->val_type = TYPE_SINT;
				}
				break;
			case TREE_EQUALS:
				code->right = sem_recur(code->right);
				code->left = sem_var_type(sem_recur(code->left), code->right->val_type);
				code->right = sem_set_type(code->right, code->left->val_type);
				code->val_type = code->left->val_type;
				break;
			case TREE_ADDEQUALS:
			case TREE_SUBEQUALS:
			case TREE_MULEQUALS:
			case TREE_DIVEQUALS:
				code->left = sem_set_type(sem_recur(code->left), TYPE_SINT);
				code->right = sem_set_type(sem_recur(code->right), TYPE_SINT);
				code->val_type = TYPE_SINT;
				break;
			case TREE_COMMA:
				code->left = sem_recur(code->left);
				code->right = sem_recur(code->right);
				code->val_type = code->right->val_type;
				break;
			case TREE_ISEQUAL:
			case TREE_GT:
			case TREE_LT:
			case TREE_GTOREQUAL:
			case TREE_LTOREQUAL:
			case TREE_NOTEQUAL:
			case TREE_LOG_AND:
			case TREE_LOG_OR:
				code->left = sem_recur(code->left);
				code->right = sem_recur(code->right);
				code->val_type = TYPE_BOOL;
				break;
			case TREE_SUBSCR:
				code->left = sem_set_type(sem_recur(code->left), TYPE_STRING);
				code->right = sem_set_type(sem_recur(code->right), TYPE_SINT);
				code->val_type = TYPE_STRING;
				break;
			case TREE_SUBSCR_SET:
				code->left = sem_var_type(sem_recur(code->left), TYPE_STRING);
				if(code->left->val_type != TYPE_STRING){
					sem_error_("Subscript set operation requires variable to be of type string");
				}
				code->cond = sem_set_type(sem_recur(code->cond), TYPE_SINT);
				code->right = sem_set_type(sem_recur(code->right), TYPE_STRING);
				code->val_type = TYPE_STRING;
				break;
		}
	}else if(code->obj_type > TREE_UNARY){
		switch(code->obj_type){
			case TREE_BRACKET:
				code->left = sem_recur(code->left);
				code->val_type = code->left->val_type;
				break;
			case TREE_RETURN:
				code->left = sem_set_type(sem_recur(code->left), sem_current_func->ret_type);
				code->val_type = TYPE_NONE;
				break;
			case TREE_POSTINC:
			case TREE_POSTDEC:
			case TREE_PREINC:
			case TREE_PREDEC:
				code->left = sem_set_type(sem_recur(code->left), TYPE_SINT);
				code->val_type = TYPE_SINT;
				break;
			case TREE_NEG:
				code->left = sem_set_type(sem_recur(code->left), TYPE_SINT);
				code->val_type = TYPE_SINT;
				break;
			case TREE_NOT:
				code->left = sem_recur(code->left);
				code->val_type = TYPE_BOOL;
				break;
		}
	}
	return code;
}

void sem_analysis(){
	sem_current_func = func_list;
	while(sem_current_func != 0){
		var_list = 0;
		sem_current_func->param = (void *) sem_recur((tree *) sem_current_func->param);
		sem_current_func->code = (void *) sem_recur((tree *) sem_current_func->code);
		sem_current_func->num_vars = var_length(var_list);
		var_destroy(var_list);
		sem_current_func = sem_current_func->next;
	}
}

