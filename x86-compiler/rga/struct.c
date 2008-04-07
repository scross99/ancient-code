#include "struct.h"
#include "sets.h"
#include <iostream>
#include <cstdio>
#include <cstring>

unsigned int rga_id = 0;

rga_node * get_alias(rga_node * node);

void rga_reset(){
	rga_id = 0;
}

rga_node * rga_node_new(rd_var * var){
	rga_node * node = new rga_node;
	node->id = rga_id++;
	node->set = RGA_NONE;
	node->prev = node->next = 0;
	node->var = var;
	return node;
}

void rga_node_free(rga_node * node){
	if(node->prev != 0){
		node->prev->next = node->next;
	}
	if(node->next != 0){
		node->next->prev = node->prev;
	}
	node->prev = node->next = 0;
}

rga_node_list * rga_node_list_new(){
	rga_node_list * node_list = new rga_node_list;
	node_list->end = 0;
	return node_list;
}

void rga_node_list_push(rga_node_list * list, rga_node * node){
	if(list->end != 0){
		list->end->next = node;
		node->prev = list->end;
	}
	list->end = node;
}

rga_node * rga_node_list_pop(rga_node_list * list){
	if(list->end == 0){
		return 0;
	}
	rga_node * node = list->end;
	if(node->prev != 0){
		node->prev->next = 0;
	}
	list->end = node->prev;
	rga_node_free(node);
	return node;
}

void rga_node_list_remove(rga_node_list * list, rga_node * node){
	rga_node * n = list->end;
	while(n != 0){
		if(n->id == node->id){
			if(n == list->end){
				list->end = n->prev;
			}
			if(n->prev != 0){
				n->prev->next = n->next;
			}
			if(n->next != 0){
				n->next->prev = n->prev;
			}
			rga_node_free(n);
			return;
		}
		n = n->prev;
	}
	printf("v%u NOT FOUND\n", node->id);
}

bool rga_node_list_isempty(rga_node_list * list){
	if(list->end == 0){
		return true;
	}else{
		return false;
	}
}

void rga_node_list_show(rga_node_list * list, unsigned int set){
	rga_node * node = list->end;
	while(node != 0){
		printf(" v%u (v%u)", node->id, get_alias(node)->id);
		if(node->set != set){
			printf("(disagree:%u)", node->set);
		}
		node = node->prev;
	}
	puts("");
}

rga_node_map * rga_node_map_copy(rga_node_map * map){
	rga_node_map * copy = 0;
	while(map != 0){
		rga_node_map_add(copy, map->node);
		map = map->prev;
	}
	return copy;
}

rga_node_map * rga_node_map_add(rga_node_map * map, rga_node * node){
	rga_node_map * node_map = new rga_node_map;
	node_map->node = node;
	node_map->prev = map;
	node_map->next = 0;
	if(map != 0){
		map->next = node_map;
	}
	return node_map;
}

rga_node_map * rga_node_map_remove(rga_node_map * map, rga_node * node){
	rga_node_map * start = map;
	while(map != 0){
		if(map->node == node){
			if(map == start){
				start = start->prev;
			}
			if(map->prev != 0){
				map->prev->next = map->next;
			}
			if(map->next != 0){
				map->next->prev = map->prev;
			}
			delete map;
			break;
		}
		map = map->prev;
	}
	return start;
}

rga_node_map * rga_node_map_combine(rga_node_map * map, rga_node_map * add){
	while(add != 0){
		map = rga_node_map_remove(map, add->node);
		map = rga_node_map_add(map, add->node);
		add = add->prev;
	}
	return map;
}





rga_move * rga_move_new(rga_node * a, rga_node * b){
	rga_move * move = new rga_move;
	move->set = RGA_NONE;
	move->a = a;
	move->b = b;
	move->prev = move->next = 0;
}

void rga_move_free(rga_move * move){
	if(move->prev != 0){
		move->prev->next = move->next;
	}
	if(move->next != 0){
		move->next->prev = move->prev;
	}
	move->prev = move->next = 0;
}

rga_move_list * rga_move_list_new(){
	rga_move_list * list = new rga_move_list;
	list->end = 0;
	return list;
}

void rga_move_list_push(rga_move_list * list, rga_move * move){
	if(list->end != 0){
		list->end->next = move;
		move->prev = list->end;
	}
	list->end = move;
}

rga_move * rga_move_list_pop(rga_move_list * list){
	if(list->end == 0){
		return 0;
	}
	rga_move * move = list->end;
	if(move->prev != 0){
		move->prev->next = 0;
	}
	list->end = move->prev;
	rga_move_free(move);
	return move;
}

bool rga_move_list_exists(rga_move_list * list, rga_node * a, rga_node * b){
	rga_move * m = list->end;
	while(m != 0){
		if((m->a == a && m->b == b) || (m->a == b && m->b == a)){
			return true;
		}
		m = m->prev;
	}
	return false;
}

void rga_move_list_remove(rga_move_list * list, rga_move * move){
	rga_move * m = list->end;
	while(m != 0){
		if(m == move){
			if(m == list->end){
				list->end = m->prev;
			}
			if(m->prev != 0){
				m->prev->next = m->next;
			}
			if(m->next != 0){
				m->next->prev = m->prev;
			}
			rga_move_free(m);
			return;
		}
		m = m->prev;
	}
	printf("Move between v%u and v%u NOT FOUND\n", move->a->id, move->b->id);
}

bool rga_move_list_isempty(rga_move_list * list){
	if(list->end == 0){
		return true;
	}else{
		return false;
	}
}

void rga_move_list_show(rga_move_list * list, unsigned int set){
	rga_move * move = list->end;
	while(move != 0){
		printf(" (v%u (v%u) v%u (v%u))", move->a->id, get_alias(move->a)->id, move->b->id, get_alias(move->b)->id);
		if(move->set != set){
			printf("(disagree:%u)", move->set);
		}
		move = move->prev;
	}
	puts("");
}

rga_move_map * rga_move_map_add(rga_move_map * map, rga_move * move){
	map = rga_move_map_remove(map, move);
	rga_move_map * move_map = new rga_move_map;
	move_map->move = move;
	move_map->prev = map;
	move_map->next = 0;
	if(map != 0){
		map->next = move_map;
	}
	return move_map;
}

rga_move_map * rga_move_map_remove(rga_move_map * map, rga_move * move){
	rga_move_map * start = map;
	while(map != 0){
		if(map->move == move){
			if(map == start){
				start = start->prev;
			}
			if(map->prev != 0){
				map->prev->next = map->next;
			}
			if(map->next != 0){
				map->next->prev = map->prev;
			}
			delete map;
			return start;
		}
		map = map->prev;
	}
	return start;
}

rga_move_map * rga_move_map_combine(rga_move_map * map, rga_move_map * add){
	while(add != 0){
		map = rga_move_map_remove(map, add->move);
		map = rga_move_map_add(map, add->move);
		add = add->prev;
	}
	return map;
}

