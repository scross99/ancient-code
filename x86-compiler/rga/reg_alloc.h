#ifndef REG_ALLOC_H
#define REG_ALLOC_H

#include "../reduced.h"

void reg_alloc(rd_instr * code,
	rd_vlist * vlist,
	rd_vlist * mem_list,
	unsigned int reg_offset,
	unsigned int reg_num,
	unsigned int match_types,
	unsigned int assign_type);

rd_vlist * mem_alloc(rd_instr * code,
	rd_vlist * vlist,
	unsigned int match_types,
	unsigned int ret_type,
	unsigned int type_size);

#endif
