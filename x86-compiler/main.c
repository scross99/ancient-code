#define ASM
#define FRAME
#include "debug.h"
#include "main.h"
#include "reduced.h"
#include "rdgen.h"
#include "var.h"
#include "func.h"
#include "sem.h"
#include "./rga/reg_alloc.h"
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>


rd_vlist * const_list = 0;

//the input FILE stream
extern FILE *yyin;

void reset(){
	rdgen_reset();
}

void function_compile(func * f){
	#ifdef DEBUG_MODE
	printf("Function %s:\n", f->name);
	tree_reconstruct((tree *) f->code);
	#endif
	rdgen_start(f->num_vars, const_list);
	rdgen((tree *) f->param);
	rdgen((tree *) f->code);
	#ifdef DEBUG_MODE
	rdgen_show();
	#endif
	frame(rdgen_first_instr, rdgen_vlist, rdgen_memlist);
	#ifdef DEBUG_MODE
	rdgen_show();
	#endif
	reg_alloc(rdgen_first_instr, rdgen_vlist, rdgen_memlist, 0, 8, RD_VAR, RD_REG);
	//reg_alloc(rdgen_first_instr, rdgen_memlist, rdgen_memlist, 10, 1, RD_MEM_FLOAT, RD_REG);
	#ifdef DEBUG_MODE
	rdgen_show();
	#endif
	rdgen_memlist = mem_alloc(rdgen_first_instr, rdgen_memlist, RD_MEM_SINT | RD_MEM_STR | RD_MEM_FLOAT, RD_MEM_SINT, 4);
	#ifdef DEBUG_MODE
	rdgen_show();
	#endif
	asm_function(f, rdgen_first_instr, rdgen_memlist);
	#ifdef DEBUG_MODE
	printf("Function %s has been compiled\n", f->name);
	#endif
	rdgen_end();
	reset();
}

int main(int argc, char ** argv){
    ++argv, --argc;  // skip program name
    if (argc > 0){
		yyin = fopen( argv[0], "r" );
		if(yyin == 0){
			printf("Input file \"%s\" does not exist.\n", argv[0]);
			return 0;
		}
    }else{
		printf("No input file was specified.\n");
		return 0;
	}
	const_list = rd_varlist(0); //create the constant list
	yyparse();
	sem_analysis();
	if(sem_error_list != 0){
		sem_error * err = sem_error_list;
		while(err->prev != 0){
			err = err->prev;
		}
		while(err != 0){
			printf("Error: %s on line %u\n", err->msg, err->line_num);
			err = err->next;
		}
	}else{
		func_compile();
		asm_write("code.s", const_list);
		system("nasm -f elf code.s");
		system("g++ -o code code.o lib.o -s");
		remove("code.o");
	}
	return 0;
	
}

