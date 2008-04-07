#include "func.h"
#include <iostream>
#include <cstdio>
#include <cstring>

func * func_list = 0;

void func_load(const char * filename){
	
}

func * func_add(){
	func * f = new func;
	f->next = func_list;
	f->name = 0;
	f->num_param = f->ret_type = 0;
	f->param = f->code = 0;
	f->global = f->external = false;
	func_list = f;
	return f;
}

func * func_include(const char * name){
	if(func_get(name) != 0){
		return 0;
	}
	func * f = func_add();
	f->name = name;
	f->external = true;
	return f;
}

func * func_get(const char * name){
	func * f = func_list;
	while(f != 0){
		if(strcmp(name, f->name) == 0){
			return f;
		}
		f = f->next;
	}
	return 0;
}

void function_compile(func * f);

void func_compile(){
	func * f = func_list;
	while(f != 0){
		if(f->external == false){
			function_compile(f);
		}
		f = f->next;
	}
}

