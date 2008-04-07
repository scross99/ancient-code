#ifndef VAR_H
#define VAR_H

//Double linked list storing variables
typedef struct var{
	char * varname;
	unsigned int id;
	unsigned int count;
	unsigned int type;
	var * next;
} var;

var * var_add(var * list, char * name, unsigned int type);
unsigned int var_id(var * list);
void var_inc_count(var * list);
unsigned int var_length(var * list);
var * var_find(var * list, unsigned int id);
var * var_find(var * list, char * name);
void var_destroy(var * list);

#endif
