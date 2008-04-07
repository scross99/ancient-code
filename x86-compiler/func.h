#ifndef FUNC_H
#define FUNC_H

typedef struct func{
	const char * name;
	unsigned int num_param;
	unsigned int num_vars;
	void * param;
	void * code;
	bool global;
	bool external;
	unsigned int ret_type;
	func * next;
} func;

extern func * func_list;

void func_load(const char * filename);

func * func_add();

func * func_include(const char * name);

func * func_get(const char * name);

void func_compile();


#endif
