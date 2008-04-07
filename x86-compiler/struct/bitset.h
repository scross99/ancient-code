#ifndef BITSET_H
#define BITSET_H

typedef struct bitset{
	unsigned char * val;
	unsigned int size;
	unsigned int true_size;
} bitset;

bitset * bitset_new(unsigned int size);
bitset * bitset_copy(bitset * set);
bool bitset_equal(bitset * a, bitset * b); //a == b
void bitset_and(bitset * a, bitset * b); //a &= b
void bitset_or(bitset * a, bitset * b); //a |= b
void bitset_xor(bitset * a, bitset * b); //a ^= b
void bitset_sub(bitset * a, bitset * b); //a &= (a ^ b)
void bitset_set(bitset * set, unsigned int n); //set[n] = true
void bitset_unset(bitset * set, unsigned int n); //set[n] = false
bool bitset_check(bitset * set, unsigned int n); //set[n] == true?
unsigned int bitset_first_on(bitset * set, unsigned int n);
unsigned int bitset_first_off(bitset * set, unsigned int n);
bool bitset_all_on(bitset * set);
void bitset_reset(bitset * set); //set = 0
void bitset_show(bitset * set);
void bitset_delete(bitset * set);

#endif
