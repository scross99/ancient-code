#include <cstdio>
#include <cstring>
#include "flow_struct.h"


void flow_unionsub(bitset * set, bitset * a, bitset * b){
	if(set->size != a->size || a->size != b->size){
		return;
	}
	unsigned int i = 0;
	while(i < set->size){
		set->val[i] |= (a->val[i] ^ b->val[i]) & a->val[i];
		i++;
	}
}

flow_node * flow_node_create(rd_instr * data, unsigned int set_size){
	flow_node * node = new flow_node;
	node->data = data;
	node->def = bitset_new(set_size);
	node->use = bitset_new(set_size);
	node->livein = bitset_new(set_size);
	node->liveout = bitset_new(set_size);
	node->prev = node->snext = node->cnext = 0;
	return node;
}

void flow_node_remove(flow_node * node, flow_node * next_node){
	if(next_node != 0){
		next_node->prev = node->prev;
	}
	delete node;
}

flow_graph * flow_graph_create(){
	flow_graph * graph = new flow_graph;
	graph->start = graph->end = 0;
	return graph;
}

void flow_graph_add(flow_graph * graph, flow_node * node){
	if(graph->start == 0){
		graph->start = graph->end = node;
	}else{
		node->prev = graph->end;
		graph->end = node;
	}
}

flow_node * flow_graph_find(flow_graph * graph, rd_instr * data){
	flow_node * node = graph->end;
	while(node != 0){
		if(node->data == data){
			return node;
		}
		node = node->prev;
	}
	return 0;
}

void flow_node_show(flow_node * node){
	if(node->prev != 0){
		flow_node_show(node->prev);
	}
	puts("NODE");
	printf("Defined:");
	bitset_show(node->def);
	printf("Used:");
	bitset_show(node->use);
	printf("Live in:");
	bitset_show(node->livein);
	printf("Live out:");
	bitset_show(node->liveout);
	puts("");
}
