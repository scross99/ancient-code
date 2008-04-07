#include "reg_alloc.h"
#include "../struct/bitset.h"
#include "../flow/flow.h"
#include "sets.h"
#include "struct.h"
#include "../rdgen.h"
#include "../debug.h"

rga_node_list * precoloured;
rga_node_list * initial;
rga_node_list * simplifyWorklist;
rga_node_list * freezeWorklist;
rga_node_list * spillWorklist;
rga_node_list * spilledNodes;
rga_node_list * coalescedNodes;
rga_node_list * colouredNodes;
rga_node_list * selectStack;

rga_move_list * coalescedMoves;
rga_move_list * constrainedMoves;
rga_move_list * frozenMoves;
rga_move_list * worklistMoves;
rga_move_list * activeMoves;

rga_node ** nodes = 0;
bitset ** adjSet = 0;
rga_node_map ** adjList = 0;
unsigned int * degree = 0;
rga_move_map ** moveList = 0;
rga_node ** alias = 0;
unsigned char * colour = 0;
unsigned int num_nodes = 0;
unsigned int rga_reg_offset = 0;
unsigned int num_registers = 0;
unsigned int rga_match_types = 0;

void debug(){
	puts("Simplify Worklist:");
	rga_node_list_show(simplifyWorklist, RGA_SIMPLIFY_WORKLIST);
	puts("Freeze Worklist:");
	rga_node_list_show(freezeWorklist, RGA_FREEZE_WORKLIST);
	puts("Spill Worklist:");
	rga_node_list_show(spillWorklist, RGA_SPILL_WORKLIST);
	puts("Coalesced Nodes:");
	rga_node_list_show(coalescedNodes, RGA_COALESCED_NODES);
	puts("Select Stack:");
	rga_node_list_show(selectStack, RGA_SELECT_STACK);

	puts("Worklist Moves:");
	rga_move_list_show(worklistMoves, RGA_WORKLIST_MOVES);
	puts("Coalesced Moves:");
	rga_move_list_show(coalescedMoves, RGA_COALESCED_MOVES);
	puts("Frozen Moves:");
	rga_move_list_show(frozenMoves, RGA_FROZEN_MOVES);
	puts("Active Moves:");
	rga_move_list_show(activeMoves, RGA_ACTIVE_MOVES);
}

void mem_add_edge(rga_node * u, rga_node * v){
	if(!bitset_check(adjSet[u->id], v->id) && !bitset_check(adjSet[v->id], u->id) && u != v){
		bitset_set(adjSet[u->id], v->id);
		bitset_set(adjSet[v->id], u->id);
		adjList[u->id] = rga_node_map_add(adjList[u->id], v);
		adjList[v->id] = rga_node_map_add(adjList[v->id], u);
	}
}

void add_edge(rga_node * u, rga_node * v){
	if(!bitset_check(adjSet[u->id], v->id) && !bitset_check(adjSet[v->id], u->id) && u != v){
		bitset_set(adjSet[u->id], v->id);
		bitset_set(adjSet[v->id], u->id);
		#ifdef DEBUG_MODE
		printf("Edge from v%u to v%u\n", u->id, v->id);
		#endif
		if(u->set != RGA_PRECOLOURED){
			adjList[u->id] = rga_node_map_add(adjList[u->id], v);
			degree[u->id]++;
		}
		if(v->set != RGA_PRECOLOURED){
			adjList[v->id] = rga_node_map_add(adjList[v->id], u);
			degree[v->id]++;
		}
	}
}

void initialise(rd_vlist * vlist, unsigned int reg_num){
	precoloured      = rga_node_list_new();
	initial          = rga_node_list_new();
	simplifyWorklist = rga_node_list_new();
	freezeWorklist   = rga_node_list_new();
	spillWorklist    = rga_node_list_new();
	spilledNodes     = rga_node_list_new();
	coalescedNodes   = rga_node_list_new();
	colouredNodes    = rga_node_list_new();
	selectStack      = rga_node_list_new();

	coalescedMoves   = rga_move_list_new();
	constrainedMoves = rga_move_list_new();
	frozenMoves      = rga_move_list_new();
	worklistMoves    = rga_move_list_new();
	activeMoves      = rga_move_list_new();

	num_nodes = vlist->num_vars + reg_num;
	
	nodes = new rga_node *[num_nodes];
	adjSet = new bitset *[num_nodes];
	adjList = new rga_node_map *[num_nodes];
	degree = new unsigned int[num_nodes];
	moveList = new rga_move_map *[num_nodes];
	alias = new rga_node *[num_nodes];
	colour = new unsigned char[num_nodes];

	rd_var * v;
	unsigned int i;

	for(i = 0; i < vlist->num_vars; i++){
		v = rd_vlist_find(vlist, i);
		nodes[i] = rga_node_new(v);
		#ifdef DEBUG_MODE
		printf("Var %u has v%u\n", i, nodes[i]->id);
		#endif
		rga_node_list_push(initial, nodes[i]);
		nodes[i]->set = RGA_INITIAL;
		colour[i] = 0;
	}

	for(i = 0; i < reg_num; i++){
		colour[vlist->num_vars + i] = (i+1);
		nodes[vlist->num_vars + i] = rga_node_new(0);
		#ifdef DEBUG_MODE
		printf("Reg %u has v%u\n", vlist->num_vars + i, nodes[vlist->num_vars + i]->id);
		#endif
		nodes[vlist->num_vars + i]->set = RGA_PRECOLOURED;
		rga_node_list_push(precoloured, nodes[vlist->num_vars + i]);
	}
	
	for(i = 0; i < num_nodes; i++){
		degree[i] = 0;
		adjList[i] = 0;
		adjSet[i] = bitset_new(num_nodes);
		moveList[i] = 0;
		alias[i] = 0;
	}
}

void clean_up(){
	delete precoloured;
	delete initial;
	delete simplifyWorklist;
	delete freezeWorklist;
	delete spillWorklist;
	delete spilledNodes;
	delete coalescedNodes;
	delete colouredNodes;
	delete selectStack;

	delete coalescedMoves;
	delete constrainedMoves;
	delete frozenMoves;
	delete worklistMoves;
	delete activeMoves;

	for(unsigned i = 0; i < num_nodes; i++){
		delete nodes[i];
		if(adjList[i] != 0){
			delete adjList[i];
		}
		bitset_delete(adjSet[i]);
		if(moveList[i] != 0){
			delete moveList[i];
		}
	}

	delete[] nodes;
	delete[] adjSet;
	delete[] adjList;
	delete[] degree;
	delete[] moveList;
	delete[] alias;
	delete[] colour;

	rga_reset();
}

void build(flow_graph * graph){
	flow_node * node = graph->end;
	bitset * live;
	rga_move * move;
	rd_instr * ins;
	unsigned int def_pos = 0, live_pos = 0;
	unsigned int p1_id = 0, p2_id = 0;
	while(node != 0){
		ins = node->data;
		live = bitset_copy(node->liveout);
		if(ins->type == RD_SET && (ins->p1->type == RD_VAR || ins->p1->type == RD_REG) && (ins->p2->type == RD_VAR || ins->p2->type == RD_REG)){			
			if(ins->p1->type == RD_VAR){
				p1_id = ins->p1->id;
			}else{
				p1_id = num_nodes - num_registers + ins->p1->id;
			}
			if(ins->p2->type == RD_VAR){
				p2_id = ins->p2->id;
			}else{
				p2_id = num_nodes - num_registers + ins->p2->id;
			}

			bitset_sub(live, node->use);
			if(!rga_move_list_exists(worklistMoves, nodes[p1_id], nodes[p2_id])){
				move = rga_move_new(nodes[p1_id], nodes[p2_id]);
				moveList[p1_id] = rga_move_map_add(moveList[p1_id], move);
				moveList[p2_id] = rga_move_map_add(moveList[p2_id], move);
				#ifdef DEBUG_MODE
				printf("Added move from v%u to v%u\n", p1_id, p2_id);
				#endif
				rga_move_list_push(worklistMoves, move);
				move->set = RGA_WORKLIST_MOVES;
			}
		}else if(ins->type == RD_INTERFERE){
			add_edge(nodes[ins->p1->id], nodes[num_nodes - num_registers + ins->p2->id]);
		}
		bitset_or(live, node->def);
		#ifdef DEBUG_MODE
		printf("Live out:");
		bitset_show(live);
		#endif
		while((def_pos = bitset_first_on(node->def, def_pos)) != ~0){
			while((live_pos = bitset_first_on(live, live_pos)) != ~0){
				add_edge(nodes[live_pos], nodes[def_pos]);
				live_pos++;
			}
			live_pos = 0;
			def_pos++;
		}
		def_pos = live_pos = 0;
		node = node->prev;
	}
}

unsigned int num_node_moves(rga_node * node){
	rga_move_map * map = moveList[node->id];
	unsigned int count = 0;
	while(map != 0){
		if(map->move->set == RGA_ACTIVE_MOVES || map->move->set == RGA_WORKLIST_MOVES){
			count++;
		}
		map = map->prev;
	}
	#ifdef DEBUG_MODE
	printf("Node v%u has %u moves\n", node->id, count);
	#endif
	return count;
}

void make_worklist(){
	rga_node * node;
	while(node = rga_node_list_pop(initial)){
		if(degree[node->id] >= num_registers){
			rga_node_list_push(spillWorklist, node);
			node->set = RGA_SPILL_WORKLIST;
		}else if(num_node_moves(node) != 0){
			rga_node_list_push(freezeWorklist, node);
			node->set = RGA_FREEZE_WORKLIST;
		}else{
			rga_node_list_push(simplifyWorklist, node);
			node->set = RGA_SIMPLIFY_WORKLIST;
		}
	}
}

void enable_moves(rga_node * node){
	//enable moves for the node
	rga_move_map * map = moveList[node->id];
	while(map != 0){
		if(map->move->set == RGA_ACTIVE_MOVES){
			rga_move_list_remove(activeMoves, map->move);
			rga_move_list_push(worklistMoves, map->move);
			map->move->set = RGA_WORKLIST_MOVES;
			#ifdef DEBUG_MODE
			printf("Move between v%u and v%u has been enabled\n", map->move->a->id, map->move->b->id);
			#endif
		}
		map = map->prev;
	}

	//enable moves for the adjacent nodes
	rga_node_map * list = adjList[node->id];
	while(list != 0){
		if(list->node->set != RGA_SELECT_STACK && list->node->set != RGA_COALESCED_NODES){
			map = moveList[list->node->id];
			while(map != 0){
				if(map->move->set == RGA_ACTIVE_MOVES){
					rga_move_list_remove(activeMoves, map->move);
					rga_move_list_push(worklistMoves, map->move);
					map->move->set = RGA_WORKLIST_MOVES;
					#ifdef DEBUG_MODE
					printf("Move between v%u and v%u has been enabled\n", map->move->a->id, map->move->b->id);
					#endif
				}
				map = map->prev;
			}
		}
		list = list->prev;
	}
}

void decrement_degree(rga_node * node){
	unsigned int d = degree[node->id]--;
	if(d == num_registers){
		#ifdef DEBUG_MODE
		printf("\tNode v%u is now low degree (%u)\n", node->id, degree[node->id]);
		#endif
		enable_moves(node);
		rga_node_list_remove(spillWorklist, node);
		if(num_node_moves(node) != 0){
			rga_node_list_push(freezeWorklist, node);
			node->set = RGA_FREEZE_WORKLIST;
		}else{
			rga_node_list_push(simplifyWorklist, node);
			node->set = RGA_SIMPLIFY_WORKLIST;
		}
	}
}

void simplify(){
	rga_node * node = rga_node_list_pop(simplifyWorklist);
	#ifdef DEBUG_MODE
	printf("Simplified node v%u\n", node->id);
	#endif
	rga_node_list_push(selectStack, node);
	node->set = RGA_SELECT_STACK;
	rga_node_map * list = adjList[node->id];
	while(list != 0){
		if(list->node->set != RGA_SELECT_STACK && list->node->set != RGA_COALESCED_NODES){
			decrement_degree(list->node);
		}
		list = list->prev;
	}
}

rga_node * get_alias(rga_node * node){
	if(node->set == RGA_COALESCED_NODES){
		return get_alias(alias[node->id]);
	}else{
		return node;
	}
}

bool coalesce_ok(rga_node * t, rga_node * r){
	rga_node_map * list = adjList[t->id];
	while(list != 0){
		if(list->node->set != RGA_SELECT_STACK && list->node->set != RGA_COALESCED_NODES){
			if(degree[list->node->id] >= num_registers &&
				list->node->set != RGA_PRECOLOURED &&
				!(bitset_check(adjSet[list->node->id], r->id) || bitset_check(adjSet[r->id], list->node->id))){
				return false;
			}
		}
		list = list->prev;
	}
	return true;
}

bool coalesce_conservative(rga_node * u, rga_node * v){
	unsigned int k = 0;
	rga_node_map * list = adjList[u->id];
	while(list != 0){
		if(list->node->set != RGA_SELECT_STACK && list->node->set != RGA_COALESCED_NODES){
			if(degree[list->node->id] >= num_registers){
				k++;
			}
		}
		list = list->prev;
	}

	list = adjList[v->id];
	while(list != 0){
		if(list->node->set != RGA_SELECT_STACK && list->node->set != RGA_COALESCED_NODES){
			if(degree[list->node->id] >= num_registers){
				k++;
			}
		}
		list = list->prev;
	}
	if(k < num_registers){
		return true;
	}else{
		return false;
	}
}

void add_work_list(rga_node * u){
	if(u->set != RGA_PRECOLOURED && num_node_moves(u) == 0 && degree[u->id] < num_registers){
		rga_node_list_remove(freezeWorklist, u);
		rga_node_list_push(simplifyWorklist, u);
		u->set == RGA_SIMPLIFY_WORKLIST;
	}
}

void combine_edge(rga_node * u, rga_node * v){
	if(!bitset_check(adjSet[u->id], v->id) && !bitset_check(adjSet[v->id], u->id) && u != v){
		bitset_set(adjSet[u->id], v->id);
		bitset_set(adjSet[v->id], u->id);
		if(u->set != RGA_PRECOLOURED){
			adjList[u->id] = rga_node_map_add(adjList[u->id], v);
		}
		if(v->set != RGA_PRECOLOURED){
			adjList[v->id] = rga_node_map_add(adjList[v->id], u);
			degree[v->id]++;
			#ifdef DEBUG_MODE
			printf("Degree of v%u ++ (now = %u)\n", v->id, degree[v->id]);
			#endif
		}
	}else{
		decrement_degree(u);
	}
}

void combine(rga_node * u, rga_node * v){
	rga_node_map * list = adjList[v->id];
	
	if(v->set == RGA_FREEZE_WORKLIST){
		rga_node_list_remove(freezeWorklist, v);
	}else{
		rga_node_list_remove(spillWorklist, v);
	}
	
	rga_node_list_push(coalescedNodes, v);
	v->set = RGA_COALESCED_NODES;
	alias[v->id] = u;
	moveList[u->id] = rga_move_map_combine(moveList[u->id], moveList[v->id]);
	enable_moves(v);
	while(list != 0){
		if(list->node->set != RGA_SELECT_STACK && list->node->set != RGA_COALESCED_NODES){
			combine_edge(list->node, u);
		}
		list = list->prev;
	}
	if(degree[u->id] >= num_registers && u->set == RGA_FREEZE_WORKLIST){
		rga_node_list_remove(freezeWorklist, u);
		rga_node_list_push(spillWorklist, u);
		u->set = RGA_SPILL_WORKLIST;
	}
}

void coalesce(){
	rga_move * move = rga_move_list_pop(worklistMoves);
	rga_node * x = get_alias(move->a);
	rga_node * y = get_alias(move->b);
	rga_node * z;
	if(y->set == RGA_PRECOLOURED){
		z = x;
		x = y;
		y = z;
	}
	if(x == y){
		rga_move_list_push(coalescedMoves, move);
		move->set = RGA_COALESCED_MOVES;
		add_work_list(x);
	}else if(y->set == RGA_PRECOLOURED ||
		(bitset_check(adjSet[x->id], y->id) || bitset_check(adjSet[x->id], y->id))){

		#ifdef DEBUG_MODE
		printf("Move from v%u to v%u has been removed because it is constrained\n", x->id, y->id);
		#endif
		
		rga_move_list_push(constrainedMoves, move);
		move->set = RGA_CONSTRAINED_MOVES;
		add_work_list(x);
		add_work_list(y);
	}else if((x->set == RGA_PRECOLOURED && coalesce_ok(y,x)) ||
		(x->set != RGA_PRECOLOURED && coalesce_conservative(x,y))){

		#ifdef DEBUG_MODE
		printf("Move from v%u to v%u has been coalesced\n", x->id, y->id);
		#endif

		rga_move_list_push(coalescedMoves, move);
		move->set = RGA_COALESCED_MOVES;
		combine(x,y);
		add_work_list(x);
	}else{
		#ifdef DEBUG_MODE
		printf("Move from v%u to v%u is not ready\n", x->id, y->id);
		#endif

		rga_move_list_push(activeMoves, move);
		move->set = RGA_ACTIVE_MOVES;
	}
}

void freeze_moves(rga_node * u){
	rga_move_map * map = moveList[u->id];
	rga_node * x, * y, * v;
	while(map != 0){
		if(map->move->set == RGA_ACTIVE_MOVES){
			x = map->move->a;
			y = map->move->b;
			if(get_alias(y) == get_alias(u)){
				v = get_alias(x);
			}else{
				v = get_alias(y);
			}
			#ifdef DEBUG_MODE
			printf("\tMove from v%u to v%u has been frozen\n", u->id, v->id);
			#endif
			rga_move_list_remove(activeMoves, map->move);
			rga_move_list_push(frozenMoves, map->move);
			map->move->set = RGA_FROZEN_MOVES;
			if(num_node_moves(v) == 0 && degree[v->id] < num_registers && v->set != RGA_PRECOLOURED){
				rga_node_list_remove(freezeWorklist, v);
				rga_node_list_push(simplifyWorklist, v);
				v->set = RGA_SIMPLIFY_WORKLIST;
			}
		}
		map = map->prev;
	}
}

void freeze(){
	rga_node * node = rga_node_list_pop(freezeWorklist);
	#ifdef DEBUG_MODE
	printf("Node v%u has been frozen and will now not be considered for moves\n", node->id);
	#endif
	rga_node_list_push(simplifyWorklist, node);
	node->set = RGA_SIMPLIFY_WORKLIST;
	freeze_moves(node);
}

double spill_priority(rga_node * node){
	return double(node->var->count) / double(degree[node->id]);
}

void select_spill(){
	rga_node * lowest_node = spillWorklist->end;
	double lowest_spill = spill_priority(lowest_node);
	rga_node * node = lowest_node->prev;
	#ifdef DEBUG_MODE
	printf("Node v%u has priority %f\n", lowest_node->id, lowest_spill);
	#endif
	double spill;
	while(node != 0){
		spill = spill_priority(node);
		#ifdef DEBUG_MODE
		printf("Node v%u has priority %f\n", node->id, spill);
		#endif
		if(spill < lowest_spill){
			lowest_node = node;
			lowest_spill = spill;
		}
		node = node->prev;
	}
	rga_node_list_remove(spillWorklist, lowest_node);
	#ifdef DEBUG_MODE
	printf("Node v%u has been selected as a possible spill\n", lowest_node->id);
	#endif
	rga_node_list_push(simplifyWorklist, lowest_node);
	lowest_node->set = RGA_SIMPLIFY_WORKLIST;
	freeze_moves(lowest_node);
}

void assign_colours(){
	rga_node * node;
	rga_node_map * map;
	rga_node * w;
	bitset * used_colours = bitset_new(num_registers);
	while(node = rga_node_list_pop(selectStack)){
		bitset_reset(used_colours);
		map = adjList[node->id];
		while(map != 0){
			w = get_alias(map->node);
			if(w->set == RGA_COLOURED_NODES || w->set == RGA_PRECOLOURED){
				bitset_set(used_colours, colour[w->id]-1);
			}
			map = map->prev;
		}
		if(bitset_all_on(used_colours)){
			rga_node_list_push(spilledNodes, node);
			#ifdef DEBUG_MODE
			printf("Node v%u has become an actual spill\n", node->id);
			#endif
			node->set = RGA_SPILLED_NODES;
		}else{
			rga_node_list_push(colouredNodes, node);
			node->set = RGA_COLOURED_NODES;
			colour[node->id] = bitset_first_off(used_colours, 0) + 1;
			#ifdef DEBUG_MODE
			printf("v%u = reg%u\n", node->id, colour[node->id]-1);
			#endif
		}
	}
	w = coalescedNodes->end;
	while(w != 0){
		colour[w->id] = colour[get_alias(w)->id];
		w = w->prev;
	}
}

void rewrite_program(flow_graph * graph, rd_vlist * vlist, rd_vlist * mem_list){
	flow_node * fnode;
	rga_node * node;
	rd_instr * ins;
	rd_var * tmp;
	rd_var * mem;
	unsigned int loop_level = 0;
	while(node = rga_node_list_pop(spilledNodes)){
		fnode = graph->end;
		loop_level = 0;
		mem = rd_vlist_mem(mem_list, RD_MEM_SINT);
		while(fnode != 0){
			if(fnode->data->type == RD_LOOPSTART){
				loop_level++;
			}else if(fnode->data->type == RD_LOOPEND){
				loop_level--;
			}
			if(bitset_check(fnode->liveout, node->id) && bitset_check(fnode->def, node->id) && bitset_check(fnode->use, node->id)){
				if(fnode->data->type == RD_SET){
					fnode = fnode->prev;
					continue;
				}

				tmp = rd_vlist_tmp(vlist);
				tmp->instances = 3;
				tmp->count = 1000 << (3 * loop_level);
				if(fnode->data->p1 != 0 && fnode->data->p1->type & rga_match_types && fnode->data->p1->id == node->id){
					fnode->data->p1 = tmp;
				}else if(fnode->data->p2 != 0 && fnode->data->p2->type & rga_match_types && fnode->data->p2->id == node->id){
					fnode->data->p2 = tmp;
				}else{
					fnode->data->p3 = tmp;
				}

				if(fnode->data->prev->type == RD_SET && fnode->data->prev->p1 == mem){
					fnode->data->prev->p1 = tmp;
				}else{
					ins = rd_new_instruction(RD_SET, tmp, mem, 0, 0);
					ins->prev = fnode->data->prev;
					ins->next = fnode->data;
					if(ins->prev != 0){
						ins->prev->next = ins;
					}
					ins->next->prev = ins;
				}

				ins = rd_new_instruction(RD_SET, mem, tmp, 0, 0);
				ins->next = fnode->data->next;
				ins->prev = fnode->data;
				if(ins->next != 0){
					ins->next->prev = ins;
				}
				ins->prev->next = ins;
			}else if(bitset_check(fnode->liveout, node->id) && bitset_check(fnode->def, node->id)){
				if(fnode->data->type == RD_SET){
					fnode->data->p1 = mem;
					fnode = fnode->prev;
					continue;
				}

				tmp = rd_vlist_tmp(vlist);
				tmp->instances = 2;
				tmp->count = 1000 << (3 * loop_level);
				if(fnode->data->p1 != 0 && fnode->data->p1->type & rga_match_types && fnode->data->p1->id == node->id){
					fnode->data->p1 = tmp;
				}else if(fnode->data->p2 != 0 && fnode->data->p2->type & rga_match_types && fnode->data->p2->id == node->id){
					fnode->data->p2 = tmp;
				}else{
					fnode->data->p3 = tmp;
				}
				ins = rd_new_instruction(RD_SET, mem, tmp, 0, 0);
				ins->next = fnode->data->next;
				ins->prev = fnode->data;
				if(ins->next != 0){
					ins->next->prev = ins;
				}
				ins->prev->next = ins;
			}else if(bitset_check(fnode->use, node->id)){
				if(fnode->data->type == RD_SET){
					fnode->data->p2 = mem;
					fnode = fnode->prev;
					continue;
				}

				tmp = rd_vlist_tmp(vlist);
				tmp->instances = 2;
				tmp->count = 1000 << (3 * loop_level);
				if(fnode->data->p1 != 0 && fnode->data->p1->type & rga_match_types && fnode->data->p1->id == node->id){
					fnode->data->p1 = tmp;
				}else if(fnode->data->p2 != 0 && fnode->data->p2->type & rga_match_types && fnode->data->p2->id == node->id){
					fnode->data->p2 = tmp;
				}else{
					fnode->data->p3 = tmp;
				}
				
				if(fnode->data->prev->type == RD_SET && fnode->data->prev->p1 == mem){
					fnode->data->prev->p1 = tmp;
				}else{
					ins = rd_new_instruction(RD_SET, tmp, mem, 0, 0);
					ins->prev = fnode->data->prev;
					ins->next = fnode->data;
					if(ins->prev != 0){
						ins->prev->next = ins;
					}
					ins->next->prev = ins;
				}
			}
			fnode = fnode->prev;
		}
	}
}

void assign_registers(rd_vlist * vlist, unsigned int assign_type){
	rd_var * v = vlist->start;
	while(v != 0){
		v->type = assign_type;
		v->id = rga_reg_offset + colour[v->id] - 1;
		v = v->next;
	}
}

void reg_alloc(rd_instr * code,
	rd_vlist * vlist,
	rd_vlist * mem_list,
	unsigned int reg_offset,
	unsigned int reg_num,
	unsigned int match_types,
	unsigned int assign_type){

	rga_reg_offset = reg_offset;
	num_registers = reg_num;
	rga_match_types = match_types;

	flow_graph * graph = flow_generate_graph(code, vlist, reg_offset, reg_num, match_types);
	initialise(vlist, reg_num);
	build(graph);
	#ifdef DEBUG_MODE
	debug();
	#endif
	make_worklist();
	while(true){
		if(!rga_node_list_isempty(simplifyWorklist)){
			simplify();
		}else if(!rga_move_list_isempty(worklistMoves)){
			coalesce();
		}else if(!rga_node_list_isempty(freezeWorklist)){
			freeze();
		}else if(!rga_node_list_isempty(spillWorklist)){
			select_spill();
		}else{
			break;
		}
	}
	assign_colours();
	if(!rga_node_list_isempty(spilledNodes)){
		rewrite_program(graph, vlist, mem_list);
		#ifdef DEBUG_MODE
		rdgen_show();
		#endif
		clean_up();
		reg_alloc(code, vlist, mem_list, reg_offset, reg_num, match_types, assign_type);
	}else{
		assign_registers(vlist, assign_type);	
		clean_up();
	}
}

rd_vlist * mem_alloc(rd_instr * code,
	rd_vlist * vlist,
	unsigned int match_types,
	unsigned int ret_type,
	unsigned int type_size){

	//start local variables
	flow_graph * graph;
	flow_node * node;

	rga_node * x, * y;
	rga_node_map * list;
	rga_move * move;

	rd_vlist * new_list;
	rd_instr * ins;
	rd_var * v;

	bitset * live;

	unsigned int def_pos, live_pos, set_size, last_id, i;

	bitset * used_colours;
	//end local variables
	
	//perform flow analysis
	graph = flow_generate_graph(code, vlist, 0, 0, match_types);

	//allocate space for node and move information
	initial          = rga_node_list_new();
	coalescedNodes   = rga_node_list_new();
	colouredNodes    = rga_node_list_new();

	coalescedMoves   = rga_move_list_new();
	constrainedMoves = rga_move_list_new();
	worklistMoves    = rga_move_list_new();
	
	nodes = new rga_node *[vlist->num_vars];
	adjSet = new bitset *[vlist->num_vars];
	adjList = new rga_node_map *[vlist->num_vars];
	alias = new rga_node *[vlist->num_vars];
	colour = new unsigned char[vlist->num_vars];

	//initialise memory variables
	for(i = 0; i < vlist->num_vars; i++){
		nodes[i] = rga_node_new(0);
		rga_node_list_push(initial, nodes[i]);
		nodes[i]->set = RGA_INITIAL;
		colour[i] = 0;
		adjList[i] = 0;
		adjSet[i] = bitset_new(vlist->num_vars);
		alias[i] = 0;
	}

	//build the graph
	node = graph->end;
	def_pos = live_pos = 0;
	while(node != 0){
		ins = node->data;
		live = bitset_copy(node->liveout);
		if(ins->type == RD_SET && ins->p1->type & match_types && ins->p2->type & match_types){
			bitset_sub(live, node->use);
			if(!rga_move_list_exists(worklistMoves, nodes[ins->p1->id], nodes[ins->p2->id])){
				move = rga_move_new(nodes[ins->p1->id], nodes[ins->p2->id]);
				rga_move_list_push(worklistMoves, move);
				move->set = RGA_WORKLIST_MOVES;
			}
		}
		bitset_or(live, node->def);
		while((def_pos = bitset_first_on(node->def, def_pos)) != ~0){
			while((live_pos = bitset_first_on(live, live_pos)) != ~0){
				mem_add_edge(nodes[live_pos], nodes[def_pos]);
				live_pos++;
			}
			live_pos = 0;
			def_pos++;
		}
		def_pos = 0;
		node = node->prev;
	}

	//coalesce phase
	while(!rga_move_list_isempty(worklistMoves)){
		move = rga_move_list_pop(worklistMoves);
		x = get_alias(move->a);
		y = get_alias(move->b);
		if(x == y){
			rga_move_list_push(coalescedMoves, move);
			move->set = RGA_COALESCED_MOVES;
		}else if(bitset_check(adjSet[x->id], y->id) || bitset_check(adjSet[x->id], y->id)){
			rga_move_list_push(constrainedMoves, move);
			move->set = RGA_CONSTRAINED_MOVES;
		}else{
			rga_move_list_push(coalescedMoves, move);
			move->set = RGA_COALESCED_MOVES;
			list = adjList[y->id];
			rga_node_list_remove(initial, y);
			rga_node_list_push(coalescedNodes, y);
			y->set = RGA_COALESCED_NODES;
			alias[y->id] = x;
			while(list != 0){
				if(list->node->set != RGA_COALESCED_NODES){
					mem_add_edge(list->node, x);
				}
				list = list->prev;
			}
		}
	}

	//simplify and select phase
	new_list = rd_varlist(0);
	set_size = 0;
	last_id = 0;
	used_colours = bitset_new(vlist->num_vars);
	while(x = rga_node_list_pop(initial)){
		bitset_reset(used_colours);
		list = adjList[x->id];
		while(list != 0){
			y = get_alias(list->node);
			if(y->set == RGA_COLOURED_NODES){
				bitset_set(used_colours, colour[y->id]);
			}
			list = list->prev;
		}
		rga_node_list_push(colouredNodes, x);
		x->set = RGA_COLOURED_NODES;
		colour[x->id] = bitset_first_off(used_colours, 0);
		if(rd_vlist_find(new_list, colour[x->id]) == 0){
			rd_vlist_add(new_list, colour[x->id], ret_type, 0, 0, type_size);
		}
		if(colour[x->id] > last_id){
			last_id = colour[x->id];
		}
		#ifdef DEBUG_MODE
		printf("mem%u gets %u\n", x->id, colour[x->id]);
		#endif
	}
	bitset_delete(used_colours);

	new_list->num_vars = last_id + 1;

	//set the colour of coalesced nodes
	x = coalescedNodes->end;
	while(x != 0){
		colour[x->id] = colour[get_alias(x)->id];
		//rd_vlist_add(new_list, colour[x->id], ret_type, 0, 0, type_size);
		#ifdef DEBUG_MODE		
		printf("mem%u gets %u\n", x->id, colour[x->id]);
		#endif
		x = x->prev;
	}

	node = graph->end;
	while(node != 0){
		if(node->data->p1 != 0){
			if(node->data->p1->type & match_types){
				v = rd_vlist_find(new_list, colour[node->data->p1->id]);
				if(v != 0){
					node->data->p1 = v;
				}
			}
		}
		if(node->data->p2 != 0){
			if(node->data->p2->type & match_types){
				v = rd_vlist_find(new_list, colour[node->data->p2->id]);
				if(v != 0){
					node->data->p2 = v;
				}
			}
		}
		if(node->data->p3 != 0){
			if(node->data->p3->type & match_types){
				v = rd_vlist_find(new_list, colour[node->data->p3->id]);
				if(v != 0){
					node->data->p3 = v;
				}
			}
		}
		node = node->prev;
	}

	//clean up
	delete initial;
	delete coalescedNodes;
	delete colouredNodes;

	delete coalescedMoves;
	delete constrainedMoves;
	delete worklistMoves;

	for(i = 0; i < vlist->num_vars; i++){
		delete nodes[i];
		if(adjList[i] != 0){
			delete adjList[i];
		}
		bitset_delete(adjSet[i]);
	}

	delete[] nodes;
	delete[] adjSet;
	delete[] adjList;
	delete[] alias;
	delete[] colour;

	rga_reset();

	return new_list;
}


