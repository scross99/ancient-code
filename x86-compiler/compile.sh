#!/bin/bash    
flex lex.l
bison -d parse.y      
g++ -o "./bin/compiler" main.c tree.c sem.c func.c var.c rdgen.c reduced.c "./flow/flow.c" "./flow/flow_struct.c" "./rga/reg_alloc.c" "./rga/struct.c" "./x86/asm_8086.c" "./x86/frame_8086.c" "./struct/bitset.c" parse.tab.c lex.yy.c -lfl -s
