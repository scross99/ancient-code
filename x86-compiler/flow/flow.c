#include "flow.h"

unsigned int flow_reg_offset = 0;
unsigned int flow_num_registers = 0;

bool flow_liveness_changed = true;
bitset * flow_backup_livein = 0;
bitset * flow_backup_liveout = 0;
unsigned int flow_match_types = 0;

flow_graph * flow_liveness_graph;
rd_vlist * flow_vlist;
flow_section * flow_section_start;
flow_section * flow_csection;

bool flow_instructions_removed = true;

//sets a bit in the bitset, checking the value is within the correct bounds
void flow_reg_set(bitset * set, unsigned int reg_id){
	if(reg_id >= flow_reg_offset && reg_id < (flow_reg_offset + flow_num_registers)){
		bitset_set(set, flow_vlist->num_vars + reg_id - flow_reg_offset);
	}
}

//calculates liveness from definitions and uses
void flow_liveness(flow_graph * graph){
	flow_liveness_changed = true;
	flow_node * node;
	//stop when the liveness graph stops changing
	while(flow_liveness_changed){
		flow_liveness_changed = false;
		node = graph->end;
		while(node != 0){
			if(!flow_liveness_changed){
				//save original values
				flow_backup_livein = bitset_copy(node->livein);
				flow_backup_liveout = bitset_copy(node->liveout);
			}
		
			bitset_or(node->livein, node->use); //live-in += use

			flow_unionsub(node->livein, node->liveout, node->def); //live-in += out - def

			if(node->snext != 0){
				bitset_or(node->liveout, node->snext->livein); //live-out += next->live-in
			}

			if(node->cnext != 0){
				bitset_or(node->liveout, node->cnext->livein); //live-out += next->live-in
			}
		
			//checks to see if the liveness graph has changed using saved values
			if(!flow_liveness_changed){
				if(!bitset_equal(node->livein, flow_backup_livein) || !bitset_equal(node->liveout, flow_backup_liveout)){
					flow_liveness_changed = true;
				}
			}
			node = node->prev;
		}
	}
}

//calculates the definitions and uses for a single node (instruction)
flow_node * flow_generate_graph_node(rd_instr * ins){
	flow_section * s = flow_section_start->next;
	flow_node * node = flow_graph_find(flow_liveness_graph, ins);
	if(node != 0){
		return node;
	}
	node = flow_node_create(ins, flow_vlist->num_vars + flow_num_registers);
	flow_graph_add(flow_liveness_graph, node);
	
	//SET def, use
	if(ins->type == RD_SET){
		if(ins->p1->type & flow_match_types){
			bitset_set(node->def, ins->p1->id);
		}else if(ins->p1->type == RD_REG){
			flow_reg_set(node->def, ins->p1->id);
		}
		if(ins->p2->type & flow_match_types){
			bitset_set(node->use, ins->p2->id);
		}else if(ins->p2->type == RD_REG){
			flow_reg_set(node->use, ins->p2->id);
		}
	}else if(ins->type == RD_USED){ //used by other components to indicate a subtle variable usage
		flow_reg_set(node->use, ins->p1->id);
	}else if(ins->type == RD_DEFINED){ //used by other components to indicate a subtle variable definition
		flow_reg_set(node->def, ins->p1->id);
	//GETPARAM use
	}else if(ins->type == RD_GETPARAM){
		if(ins->p1->type & flow_match_types){
			bitset_set(node->def, ins->p1->id);
		}else if(ins->p1->type == RD_REG){
			flow_reg_set(node->def, ins->p1->id);
		}
	//FPUSH use, FPUSHI use
	}else if(ins->type == RD_FPUSH || ins->type == RD_FPUSHI){
		if(ins->p1->type & flow_match_types){
			bitset_set(node->use, ins->p1->id);
		}
	//FPOP def, FPOPI def
	}else if(ins->type == RD_FPOP || ins->type == RD_FPOPI){
		if(ins->p1->type & flow_match_types){
			bitset_set(node->def, ins->p1->id);
		}
	//FADD use, FSUB use, FMUL use, FDIV use
	}else if(ins->type == RD_FADD || ins->type == RD_FSUB || ins->type == RD_FMUL || ins->type == RD_FDIV){
		if(ins->p1->type & flow_match_types){
			bitset_set(node->use, ins->p1->id);
		}
	//PARAM use
	//CMP use, use
	//ALL OTHERS def|use, use, use
	}else if(ins->type != RD_INTERFERE){
		if(ins->p1 != 0){
			if(ins->p1->type & flow_match_types){
				if(ins->type != RD_PARAM && ins->type != RD_CMP){
					bitset_set(node->def, ins->p1->id);
				}
				bitset_set(node->use, ins->p1->id);
			}else if(ins->p1->type == RD_REG){
				if(ins->type != RD_PARAM && ins->type != RD_CMP){
					flow_reg_set(node->def, ins->p1->id);
				}
				flow_reg_set(node->use, ins->p1->id);
			}
		}
		if(ins->p2 != 0){
			if(ins->p2->type & flow_match_types){
				bitset_set(node->use, ins->p2->id);
			}else if(ins->p2->type == RD_REG){
				flow_reg_set(node->use, ins->p2->id);
			}
		}
		if(ins->p3 != 0){
			if(ins->p3->type & flow_match_types){
				bitset_set(node->use, ins->p3->id);
			}else if(ins->p3->type == RD_REG){
				flow_reg_set(node->use, ins->p3->id);
			}
		}
	}

	if(ins->type == RD_JUMP){
		//search section list for correct section to jump to
		while(s != 0){
			if(s->id == ins->p1->id){
				node->snext = flow_generate_graph_node(s->section);
				return node;
			}
			s = s->next;
		}
	}else if(ins->type > RD_CONDJUMPSTART && ins->type < RD_CONDJUMPEND){
		//search section list for correct section to jump to
		while(s != 0){
			if(s->id == ins->p1->id){
				node->cnext = flow_generate_graph_node(s->section);
				break;
			}
			s = s->next;
		}
	}

	if(ins->next != 0){
		node->snext = flow_generate_graph_node(ins->next);
	}
	return node;
}

//routines to reduce code size by removing unneccessary instructions
void flow_clean_instructions(rd_instr * code){
	flow_node * node;
	flow_node * next_node = 0;
	bitset * live;
	flow_instructions_removed = true;
	while(flow_instructions_removed){
		flow_instructions_removed = false;
		node = flow_liveness_graph->end;
		while(node != 0){
			//remove instructions where the defined variable is not used
			if(bitset_first_on(node->def, 0) < flow_vlist->num_vars){
				live = bitset_copy(node->liveout);
				bitset_sub(live, node->def);
				if(bitset_equal(live, node->liveout)){
					rd_remove_instruction(node->data);
					node = node->prev;
					flow_instructions_removed = true;
					continue;
				}
			}

			//remove sequential pop and push actions of the same variable
			if(node->snext != 0){
				if(node->data->type == RD_FPOP
					&& node->snext->data->type == RD_FPUSH
					&& rd_equal(node->data->p1, node->snext->data->p1)){
					rd_remove_instruction(node->snext->data);
					rd_remove_instruction(node->data);
					node = node->prev;
					flow_instructions_removed = true;
					continue;
				}
			}

			//remove conditional jump instructions which jump to the next instruction
			if(node->snext != 0 && node->snext == node->cnext){
				rd_remove_instruction(node->data);
				node = node->prev;
				flow_instructions_removed = true;
				//remove the comparison statement for the conditional jump
				if(node != 0 && node->data->type == RD_CMP){
					rd_remove_instruction(node->data);
					node = node->prev;
				}
				continue;
			}

			//remove unconditional jump instructions which jump to the next instruction
			if(node->data->type == RD_JUMP && node->snext->data == node->data->next){
				rd_remove_instruction(node->data);
				node = node->prev;
				flow_instructions_removed = true;
				continue;
			}

			//changes a = b, c = a  into c = b
			if(node->data->type == RD_SET
					&& node->data->prev->type == RD_SET
					&& node->data->p2 == node->data->prev->p1
					&& !bitset_check(node->liveout, node->data->p2->id)){
				node->data->prev->p1 = node->data->p1;
				rd_remove_instruction(node->data);
				node = node->prev->prev;
				flow_instructions_removed = true;
				continue;
			}

			node = node->prev;
		}

		//generate a new liveness graph
		if(flow_instructions_removed){
			node = flow_liveness_graph->end;
			//next_node = 0;
			while(node != 0){
				next_node = node;
				node = node->prev;
				if(next_node->data->type != RD_SECTION){
					delete next_node;
				}
			}
			flow_liveness_graph->start = flow_liveness_graph->end = 0;
			flow_generate_graph_node(code);
			flow_liveness(flow_liveness_graph);
		}
	}
}

flow_graph * flow_generate_graph(rd_instr * code, rd_vlist * vlist, unsigned int reg_offset, unsigned int reg_num, unsigned int match_types){
	rd_instr * ins = code;
	flow_section * sec;

	flow_reg_offset = reg_offset;
	flow_num_registers = reg_num;
	flow_match_types = match_types;

	flow_liveness_graph = flow_graph_create();
	flow_vlist = vlist;

	//Set up the list of sections
	flow_section_start = new flow_section;
	flow_csection = flow_section_start;
	flow_csection->section = 0;
	flow_csection->next = 0;

	//Generate the list of sections
	while(ins != 0){
		if(ins->type == RD_SECTION){
			sec = new flow_section;
			sec->id = ins->p1->id;
			sec->section = ins;
			flow_csection->next = sec;
			flow_csection = sec;
		}
		ins = ins->next;
	}

	flow_generate_graph_node(code);

	flow_liveness(flow_liveness_graph);

	flow_clean_instructions(code);

	return flow_liveness_graph;
}

