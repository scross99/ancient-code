#ifndef REDUCED_H
#define REDUCED_H

//---REDUCED CODE INSTRUCTIONS---

//Set Operations
#define RD_SETOP 0

//Standard
#define RD_SET 1

//Floating point
#define RD_FPUSH  51
#define RD_FPOP   52
#define RD_FPUSHI 53
#define RD_FPOPI  54

//Mathematical Operations
#define RD_MATH 100

//Signed integer
#define RD_ADD 101
#define RD_SUB 102
#define RD_MUL 103
#define RD_DIV 104
#define RD_INC 105
#define RD_DEC 106
#define RD_NEG 107

//String
#define RD_SUBSCR 131
#define RD_SUBSCR_SET 132

//Floating point
#define RD_FADD 171
#define RD_FSUB 172
#define RD_FMUL 173
#define RD_FDIV 174
#define RD_FINC 175
#define RD_FDEC 176
#define RD_FNEG 177

//Condition Evaluating Operations
#define RD_CONDOP    200
#define RD_CMP       201

//Conditional Jumps
#define RD_CONDJUMPSTART 210
#define RD_JE        211
#define RD_JNE       212
#define RD_JG        213
#define RD_JL        214
#define RD_JGE       215
#define RD_JLE       216
#define RD_CONDJUMPEND 219

//Binary operations
#define RD_BINOP  300
#define RD_AND    301
#define RD_OR     302
#define RD_XOR    303
#define RD_NOT    304
#define RD_LSHIFT 305
#define RD_RSHIFT 306

//Jump/Call operations
#define RD_JUMPOP     400
#define RD_JUMP       401
#define RD_PARAM      402
#define RD_CALL       403
#define RD_CLRPARAM   404
#define RD_SECTION    405

#define RD_GETPARAM   410
#define RD_SINTPARAM  411
#define RD_FLOATPARAM 412
#define RD_STRPARAM   413

#define RD_RETURN     420

//Type conversions
#define RD_TYPECONV  500
#define RD_I_TO_F    501 //signed integer to floating point number
#define RD_F_TO_I    502 //floating point number to signed integer
#define RD_S_TO_I    503 //string to signed integer
#define RD_I_TO_S    504 //signed integer to string
#define RD_S_TO_F    505 //string to floating point number
#define RD_F_TO_S    506 //floating point number to string

//Notices
#define RD_LOOPSTART    600 //indicates the start of the body of a loop (for optimization)
#define RD_LOOPEND      601 //indicates the end of the body of a loop (for optimization)
#define RD_INTERFERE    602 //specifies that the variable (first param) interferes with the register (second param)

#define RD_USED         610 //indicates the register (first param) is used here
#define RD_DEFINED      611 //indicates the register (first param) is defined here

//Custom instructions
#define RD_CUSTOM       700 //instructions > RD_CUSTOM are defined by the platform specific frame

//---INSTRUCTIONS END---

//RD Var Types
#define RD_VAR              (1 << 0) //variables which can be assigned to registers
#define RD_SEC              (1 << 1) //a section/label
#define RD_REG              (1 << 2) //a platform register
#define RD_MEM_SINT         (1 << 3) //integer variables stored in memory
#define RD_CONST_SINT       (1 << 4) //integer constants
#define RD_MEM_FLOAT        (1 << 5)
#define RD_CONST_FLOAT      (1 << 6)
#define RD_MEM_STR          (1 << 7)
#define RD_CONST_STR        (1 << 8)
#define RD_CONST_NAME       (1 << 9)
#define RD_MEM_DEREF        (1 << 10)

#define SIZE_REG 4
#define SIZE_SINT 4
#define SIZE_UINT 4
#define SIZE_CHAR 1
#define SIZE_FLOAT 4

//Container for variables/temporary stores/constants
typedef struct rd_var{
	unsigned int type;
	unsigned int id;
	union{
		const char * str;
		signed int sint;
		double fval;
		unsigned int uint;
	};
	unsigned long int count;
	unsigned int instances;
	unsigned int size;
	rd_var * prev;
	rd_var * next;
} rd_var;

//RD Variable List
typedef struct rd_vlist{
	rd_var * start;
	rd_var * end;
	unsigned int num_vars;
} rd_vlist;

//RD Instruction
typedef struct rd_instr{
	unsigned int type;
	rd_var * p1;
	rd_var * p2;
	rd_var * p3;
	rd_instr * prev;
	rd_instr * next;
} rd_instr;


rd_instr * rd_new_instruction(unsigned int type, rd_var * p1, rd_var * p2, rd_var * p3, rd_instr * prev);

void rd_remove_instruction(rd_instr * ins);

//creates a variable list
rd_vlist * rd_varlist(unsigned int num_vars);

//add a variable (of any type) to the list
void rd_vlist_add(rd_vlist * vlist, unsigned int id, unsigned int type, unsigned int count, unsigned int instances, unsigned int size);

//find a variable in the list
rd_var * rd_vlist_find(rd_vlist * vlist, unsigned int id);

//adds a variable to the list
rd_var * rd_vlist_var(rd_vlist * vlist, unsigned int id);

//adds a temporary variable to the list
rd_var * rd_vlist_tmp(rd_vlist * vlist);

//find a memory location in a variable list
rd_var * rd_vlist_mem_find(rd_vlist * vlist, unsigned int mem_id);

//add a memory location of the specified type
rd_var * rd_vlist_mem(rd_vlist * vlist, unsigned int type);

//adds a floating point constant memory location
rd_var * rd_vlist_const_float(double fval, rd_vlist * vlist);

//adds a string constant memory location
rd_var * rd_vlist_const_string(const char * sval, rd_vlist * vlist);

//reference to a memory location by id
rd_var * rd_mem_sint(unsigned int id);

//dereference a memory location by using a non-memory variable and an offset
void rd_deref(rd_var * v, signed int offset);

//reference to a CPU register by id
rd_var * rd_register(unsigned int id);

//reference to a section by id
rd_var * rd_section(unsigned int id);

//makes a reference string name (for function calls)
rd_var * rd_name(const char * name);

//makes a constant string type variable
rd_var * rd_str(char * str);

//makes a constant signed integer type variable
rd_var * rd_sint(signed int sint);

bool rd_is_mem(rd_var * var);

//checks if two variables are the same
bool rd_equal(rd_var * a, rd_var * b);

#endif
