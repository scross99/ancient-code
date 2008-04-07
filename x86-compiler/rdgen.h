#ifndef RDGEN_H
#define RDGEN_H

#include "tree.h"
#include "reduced.h"
#include "var.h"

typedef struct rdgen_cmp{
	rd_instr * cmp_ins;
	unsigned int jump_type;
	bool sign; //true = positive, false = negative
	unsigned int jmp_id;
	rdgen_cmp * prev;
	rdgen_cmp * next;
} rdgen_cmp;

typedef struct rdgen_cond{
	rdgen_cmp * start;
	rdgen_cmp * end;
} rdgen_cond;

void rdgen_reset();

void rdgen_start(unsigned int num_vars, rd_vlist * const_list);

void rdgen(tree * code);

void rdgen_end();

#include <stdio.h>

void rdgen_show();

extern rd_instr * rdgen_first_instr;
extern rd_vlist * rdgen_vlist;
extern rd_vlist * rdgen_memlist;

#endif
