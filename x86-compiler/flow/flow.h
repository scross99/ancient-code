#ifndef FLOW_H
#define FLOW_H

#include "flow_struct.h"

void flow_liveness(flow_graph * graph);

flow_graph * flow_generate_graph(rd_instr * code, rd_vlist * vlist, unsigned int reg_offset, unsigned int reg_num, unsigned int match_types);

#endif

