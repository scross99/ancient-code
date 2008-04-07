#ifndef STRUCT_H
#define STRUCT_H

#include "../reduced.h"

typedef struct rga_node{
	unsigned int id;
	unsigned int set;
	rd_var * var;
	rga_node * prev;
	rga_node * next;
} rga_node;

typedef struct rga_node_list{
	rga_node * end;
} rga_node_list;

typedef struct rga_node_map{
	rga_node * node;
	rga_node_map * prev;
	rga_node_map * next;
} rga_node_map;

typedef struct rga_move{
	unsigned int set;
	rga_node * a;
	rga_node * b;
	rga_move * prev;
	rga_move * next;
} rga_move;

typedef struct rga_move_list{
	rga_move * end;
} rga_move_list;

typedef struct rga_move_map{
	rga_move * move;
	rga_move_map * prev;
	rga_move_map * next;
} rga_move_map;

void rga_reset();

rga_node * rga_node_new(rd_var * var);
void rga_node_free(rga_node * node);

rga_node_list * rga_node_list_new();
void rga_node_list_push(rga_node_list * list, rga_node * node);
rga_node * rga_node_list_pop(rga_node_list * list);
void rga_node_list_remove(rga_node_list * list, rga_node * node);
bool rga_node_list_isempty(rga_node_list * list);
void rga_node_list_show(rga_node_list * list, unsigned int set);

rga_node_map * rga_node_map_copy(rga_node_map * map);
rga_node_map * rga_node_map_add(rga_node_map * map, rga_node * node);
rga_node_map * rga_node_map_remove(rga_node_map * map, rga_node * node);
rga_node_map * rga_node_map_combine(rga_node_map * map, rga_node_map * add);



rga_move * rga_move_new(rga_node * a, rga_node * b);
void rga_move_free(rga_move * move);

rga_move_list * rga_move_list_new();
void rga_move_list_push(rga_move_list * list, rga_move * move);
rga_move * rga_move_list_pop(rga_move_list * list);
bool rga_move_list_exists(rga_move_list * list, rga_node * a, rga_node * b);
void rga_move_list_remove(rga_move_list * list, rga_move * move);
bool rga_move_list_isempty(rga_move_list * list);
void rga_move_list_show(rga_move_list * list, unsigned int set);

rga_move_map * rga_move_map_add(rga_move_map * map, rga_move * move);
rga_move_map * rga_move_map_remove(rga_move_map * map, rga_move * move);
rga_move_map * rga_move_map_combine(rga_move_map * map, rga_move_map * add);

#endif
