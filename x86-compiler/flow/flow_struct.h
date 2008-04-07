#ifndef FLOW_STRUCT_H
#define FLOW_STRUCT_H

#include "../struct/bitset.h"
#include "../reduced.h"
#include <iostream>

typedef struct flow_node{
	rd_instr * data; //pointer to associated instruction
	bitset * def; //variables defined at this node
	bitset * use; //variables used at this node
	bitset * livein; //live-in variables
	bitset * liveout; //live-out variables
	flow_node * prev; //previous node on list
	flow_node * snext; //next node straight after
	flow_node * cnext; //next node on conditional jump
} flow_node;

typedef struct flow_graph{
	flow_node * start;
	flow_node * end;
} flow_graph;

typedef struct flow_section{
	unsigned int id;
	rd_instr * section;
	flow_section * next;
} flow_section;

void flow_unionsub(bitset * set, bitset * a, bitset * b); //set += a - b

flow_node * flow_node_create(rd_instr * data, unsigned int set_size);
void flow_node_remove(flow_node * node, flow_node * next_node);
void flow_node_show(flow_node * node);

flow_graph * flow_graph_create();
void flow_graph_add(flow_graph * graph, flow_node * node);
flow_node * flow_graph_find(flow_graph * graph, rd_instr * data);

#endif
