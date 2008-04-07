#include "tree.h"
#include "sem.h"
#include "func.h"
#include <stdio.h>

tree * build_sint(int val){
	tree * t = new tree;
	t->obj_type = TREE_INT;
	t->intdata = val;
	return t;
}

tree * build_float(double val){
	tree * t = new tree;
	t->obj_type = TREE_FLT;
	t->floatdata = val;
	return t;
}

tree * build_string(char * str){
	tree * t = new tree;
	t->obj_type = TREE_STR;
	t->stringdata = str;
	return t;
}

tree * build_var(char * varname){
	tree * t = new tree;
	t->obj_type = TREE_VAR;
	t->stringdata = varname;
	return t;
}

tree * build_var(unsigned int id, unsigned int type){
	tree * t = new tree;
	t->obj_type = TREE_VAR;
	t->intdata = id;
	t->val_type = type;
	return t;
}

tree * build_unary(tree * left, unsigned int type){
	tree * t = new tree;
	t->obj_type = type;
	t->left = left;
	return t;
}

tree * build_dual(tree * left, tree * right, unsigned int type){
	tree * t = new tree;
	t->obj_type = type;
	t->left = left;
	t->right = right;
	return t;
}

tree * build_callparam_tree(tree * param, tree * parent){
	if(param == NULL){
		return NULL;
	}
	tree * t;
	if(param->obj_type == TREE_COMMA){
		if(parent != 0){
			parent->left = param->right;
			param->right = parent;
		}else{
			param->right = build_callparam(param->right, 0);
		}
		param->obj_type = TREE_CALLPARAM;
		t = build_callparam_tree(param->left, param);
		return t;
	}else{
		if(parent != 0){
			return parent;
		}else{
			return build_callparam(param, 0);
		}
	}
}

tree * build_func_call(char * fn_name, tree * param){
	tree * t = new tree;
	t->obj_type = TREE_FNCALL;
	t->stringdata = fn_name;
	t->left = build_callparam_tree(param, 0);
	return t;
}

tree * build_callparam(tree * data, tree * nextparam){
	tree * t = new tree;
	t->obj_type = TREE_CALLPARAM;
	t->left = data;
	t->right = nextparam;
	return t;
}

tree * build_specparam(tree * data, unsigned int val_type, tree * nextparam){
	tree * t = new tree;
	t->obj_type = TREE_SPECPARAM;
	t->val_type = val_type;
	t->left = data;
	t->right = nextparam;
	return t;
}

tree * build_line(tree * data, tree * lastline, int line_number){
	tree * t = new tree;
	t->obj_type = TREE_CODELINE;
	t->intdata = line_number;
	t->left = data;
	t->right = lastline;
	return t;
}

tree * build_if(tree * condition, tree * code_if_true, tree * last_condition){
	tree * i;
	tree * t = new tree;
	t->obj_type = TREE_IFSTMT;
	t->cond = condition;
	t->left = code_if_true;
	if(last_condition != 0){
		i = last_condition;
		while(i->right != 0){
			i = i->right;
		}
		i->right = t;
		return last_condition;
	}else{
		return t;
	}
}

tree * build_else(tree * code, tree * last_condition){
	tree * i;
	tree * t = new tree;
	t->obj_type = TREE_IFSTMT;
	t->cond = 0;
	t->left = code;
	if(last_condition != 0){
		i = last_condition;
		while(i->right != 0){
			i = i->right;
		}
		i->right = t;
		return last_condition;
	}else{
		return t;
	}
}

tree * build_while(tree * condition, tree * action){
	tree * t = new tree;
	t->obj_type = TREE_WHILESTMT;
	t->cond = condition;
	t->left = action;
	return t;
}

tree * build_for(tree * start, tree * condition, tree * end, tree * action, int line_number){
	end = build_line(end, action, line_number);
	tree * t = new tree;
	t->obj_type = TREE_FORSTMT;
	t->cond = condition;
	t->left = start;
	t->right = end;
	return t;
}

tree * build_typeconv(tree * code, unsigned int val_type){
	tree * t = new tree;
	t->obj_type = TREE_TYPECONV;
	t->val_type = val_type;
	t->left = code;
	return t;
}

tree * build_subscr_set(tree * var, tree * index, tree * val){
	tree * t = new tree;
	t->obj_type = TREE_SUBSCR_SET;
	t->left = var;
	t->cond = index;
	t->right = val;
	return t;
}

void build_function(char * name, tree * params, tree * treecode, bool global, bool external, unsigned int ret_type){
	unsigned int i = 0;
	unsigned int param_num = 0;
	tree * p = params;
	while(p != 0){
		p->intdata = param_num++;
		p = p->right;
	}

	func * f = func_add();
	f->name = name;
	f->param = (void *) params;
	while(params != 0){
		i++;
		params = params->right;
	}
	f->num_param = i;
	f->code = (void *) treecode;
	f->global = global;
	f->external = external;
	f->ret_type = ret_type;
}

unsigned int tree_next_uid = 0;

unsigned int tree_reconstruct(tree * code){
	if(code == 0){
		printf("%u: NULL\n", tree_next_uid);
	}else if(code->obj_type == TREE_IFSTMT){
		if(code->cond != 0){
			printf("IF(%u){\n", tree_reconstruct(code->cond));
		}
		printf("DO(%u)\n", tree_reconstruct(code->left));
		if(code->right != 0){
			printf("}ELSE{\n");
			tree_reconstruct(code->right);
		}
		if(code->cond != 0){
			printf("}\n");
		}
	}else if(code->obj_type == TREE_WHILESTMT){
		printf("WHILE(%u){\n", tree_reconstruct(code->cond));
		printf("DO(%u)\n", tree_reconstruct(code->left));
		printf("}\n");
	}else if(code->obj_type == TREE_FORSTMT){
		tree_reconstruct(code->left);
		printf("WHILE(%u){\n", tree_reconstruct(code->cond));
		printf("DO(%u)\n", tree_reconstruct(code->left));
		printf("}\n");
	}else if(code->obj_type == TREE_FNCALL){
		tree_reconstruct(code->left);
		printf("%u: %s()\n", tree_next_uid, code->stringdata);
	}else if(code->obj_type == TREE_CALLPARAM){
		tree_reconstruct(code->right);
		printf("%u: PARAM %u\n", tree_next_uid, tree_reconstruct(code->left));
	}else if(code->obj_type == TREE_CODELINE){
		tree_reconstruct(code->right);
		tree_reconstruct(code->left);
	}else if(code->obj_type == TREE_VAR){
		printf("%u: Var %u\n", tree_next_uid, code->intdata);
	}else if(code->obj_type == TREE_INT){
		printf("%u: %i\n", tree_next_uid, code->intdata);
	}else if(code->obj_type == TREE_TYPECONV){
		printf("%u: TYPECONV\n", tree_next_uid);
	}else if(code->obj_type > TREE_DUAL){
		printf("%u: %u DUAL(%u) %u\n", tree_next_uid, tree_reconstruct(code->left), code->obj_type, tree_reconstruct(code->right));
	}else if(code->obj_type > TREE_UNARY){
		printf("%u: UNARY %u\n", tree_next_uid, tree_reconstruct(code->left));
	}
	return tree_next_uid++;
}



