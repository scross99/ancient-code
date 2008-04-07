#include "bitset.h"
#include <iostream>
#include <cstdio>
#include <cstring>

bitset * bitset_new(unsigned int size){
	bitset * set = new bitset;
	set->true_size = size;
	set->size = (size >> 3)+1;
	set->val = new unsigned char[set->size];
	unsigned int i = 0;
	while(i < set->size){
		set->val[i] = 0;
		i++;
	}
	return set;
}

bitset * bitset_copy(bitset * set){
	bitset * res = new bitset;
	res->size = set->size;
	res->true_size = set->true_size;
	res->val = new unsigned char[set->size];
	for(unsigned int i = 0; i < set->size; i++){
		res->val[i] = set->val[i];
	}
	return res;
}

bool bitset_equal(bitset * a, bitset * b){
	if(a->true_size != b->true_size){
		return false;
	}
	unsigned int i = 0;
	while(i < a->size){
		if(a->val[i] != b->val[i]){
			return false;
		}
		i++;
	}
	return true;
}

void bitset_and(bitset * a, bitset * b){
	if(a->true_size != b->true_size){
		return;
	}
	unsigned int i = 0;
	while(i < a->size){
		a->val[i] &= b->val[i];
		i++;
	}
}

void bitset_or(bitset * a, bitset * b){
	if(a->true_size != b->true_size){
		return;
	}
	unsigned int i = 0;
	while(i < a->size){
		a->val[i] |= b->val[i];
		i++;
	}
}

void bitset_xor(bitset * a, bitset * b){
	if(a->true_size != b->true_size){
		return;
	}
	unsigned int i = 0;
	while(i < a->size){
		a->val[i] ^= b->val[i];
		i++;
	}
}

void bitset_sub(bitset * a, bitset * b){
	if(a->true_size != b->true_size){
		return;
	}
	unsigned int i = 0;
	while(i < a->size){
		a->val[i] &= a->val[i] ^ b->val[i];
		i++;
	}
}

void bitset_set(bitset * set, unsigned int n){
	unsigned int c = (n >> 3);
	n -= (c << 3);
	set->val[c] |= (1 << n);
}

void bitset_unset(bitset * set, unsigned int n){
	unsigned int c = (n >> 3);
	n -= (c << 3);
	set->val[c] &= ~(1 << n);
}

bool bitset_check(bitset * set, unsigned int n){
	unsigned int c = (n >> 3);
	n -= (c << 3);
	if(set->val[c] & (1 << n)){
		return true;
	}else{
		return false;
	}
}

unsigned int bitset_first_on(bitset * set, unsigned int n){
	for(unsigned int i = n; i < set->true_size; i++){
		if(bitset_check(set, i)){
			return i;
		}
	}
	return ~0;
}

unsigned int bitset_first_off(bitset * set, unsigned int n){
	for(unsigned int i = n; i < set->true_size; i++){
		if(!bitset_check(set, i)){
			return i;
		}
	}
	return ~0;
}

bool bitset_all_on(bitset * set){
	unsigned int i = 0;
	unsigned int size = set->true_size >> 3;
	while(i < size){
		if(set->val[i] != (unsigned char) ~0){
			return false;
		}
		i++;
	}
	i <<= 3;
	while(i < set->true_size){
		if(!bitset_check(set, i)){
			return false;
		}
		i++;
	}
	return true;
}

void bitset_reset(bitset * set){
	unsigned int i = 0;
	while(i < set->size){
		set->val[i] = 0;
		i++;
	}
}

void bitset_show(bitset * set){
	for(unsigned int i = 0; i < set->true_size; i++){
		if(bitset_check(set, i)){
			printf(" %u", i);
		}
	}
	puts("");
}

void bitset_delete(bitset * set){
	delete[] set->val;
	delete set;
}

