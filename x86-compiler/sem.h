#ifndef SEM_H
#define SEM_H

typedef struct sem_error{
	unsigned int error_code;
	unsigned int line_num;
	const char * msg;
	sem_error * prev;
	sem_error * next;
} sem_error;


void sem_analysis();

extern sem_error * sem_error_list;


#endif
