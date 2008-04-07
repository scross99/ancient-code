#include "var.h"
#include <iostream>
#include <cstdio>
#include <cstring>

unsigned int var_next_id = 0;

var * var_add(var * list, char * name, unsigned int type){
	var * v = new var;
	v->varname = name;
	v->next = list;
	v->id = var_next_id++;
	v->count = 1;
	v->type = type;

	return v;
}

unsigned int var_id(var * list){
	if(list == 0){
		return 0;
	}
	return list->id;
}

void var_inc_count(var * list){
	list->count++;
}

unsigned int var_length(var * list){
	unsigned int count = 0;
	while(list != 0){
		list = list->next;
		count++;
	}
	return count;
}

var * var_find(var * list, unsigned int id){
	while(list != 0){
		if(list->id == id){
			return list;
		}
		list = list->next;
	}
	return 0;
}

var * var_find(var * list, char * name){
	while(list != 0){
		if(strcmp(list->varname, name) == 0){
			return list;
		}
		list = list->next;
	}
	return 0;
}

void var_destroy(var * list){
	var * temp;
	while(list != 0){
		temp = list;
		list = list->next;
		delete temp;
	}
	var_next_id = 0;
}
