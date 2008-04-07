#ifndef TREE_H
#define TREE_H

//Base type values
#define TREE_VALS 0
#define TREE_INT 1
#define TREE_STR 2
#define TREE_FLT 3
#define TREE_VAR 5

//Unary Operators
#define TREE_UNARY 100
#define TREE_PREINC 101                  // ++var
#define TREE_PREDEC 102                  // --var
#define TREE_POSTINC 103                 // var++
#define TREE_POSTDEC 104                 // var--
#define TREE_NOT 105                     // !exp
#define TREE_NEG 106                     // -exp
#define TREE_COM 107                     // ~exp
#define TREE_BRACKET 108                 // ( ... )

#define TREE_RETURN 110                  // return exp
#define TREE_BREAK 111                   // break
#define TREE_CONTINUE 112                // continue

//Dual Operators
#define TREE_DUAL 200
#define TREE_ADD 201                     // +
#define TREE_SUBTRACT 202                // -
#define TREE_MULTIPLY 203                // *
#define TREE_DIVIDE 204                  // /

#define TREE_ASSIGNMENT 220
#define TREE_EQUALS 221                  // =
#define TREE_ADDEQUALS 222               // +=
#define TREE_SUBEQUALS 223               // -=
#define TREE_MULEQUALS 224               // *=
#define TREE_DIVEQUALS 225               // /=

#define TREE_CONDITIONAL 250
#define TREE_ISEQUAL 251                 // ==
#define TREE_GT 252                      // >
#define TREE_LT 253                      // <
#define TREE_GTOREQUAL 254               // >=
#define TREE_LTOREQUAL 255               // <=
#define TREE_NOTEQUAL 256                // !=

#define TREE_LOG_AND 261                 // &&
#define TREE_LOG_OR 262                  // ||

#define TREE_COMMA 271                   // ,
#define TREE_SUBSCR 272                  // var[index]
#define TREE_SUBSCR_SET 273              // var[index] = value

//Function Calls
#define TREE_FUNCTION 300
#define TREE_FNCALL 301                  // func(PARAMS)
#define TREE_CALLPARAM 302               // call f(param1, param2, ...)

#define TREE_SPECPARAM 310               // spec f(param1, param2, ...)

//Code lines
#define TREE_LINE 400
#define TREE_CODELINE 401

//Statements
#define TREE_STMT 500
#define TREE_IFSTMT 501                  // if(cond){ action; }
#define TREE_WHILESTMT 502               // while(cond){ action; }
#define TREE_FORSTMT 503                 // for(start; cond; end){ action; }

//Type conversions
#define TREE_TYPECONV 600

//Function calls are stored as stringdata -> name, left -> param
//Call and spec params are stored as left -> paramdata, right -> nextparam
//Lines are stored as left -> code on that line, right -> last line, intdata -> line number
//If statements are stored as cond -> condition, left -> code if true, right -> next if statement
//While statements are stored as cond -> condition, left -> action
//For statements are stored as cond -> condition, left -> start, right -> action + end

//Value Types
#define TYPE_NONE 0 //For elements which do not return a value (eg. if,while)
#define TYPE_BOOL 1 //Boolean
#define TYPE_SINT 2 //Signed Integer
#define TYPE_UINT 3 //Unsigned Integer
#define TYPE_STRING  4 //String
#define TYPE_FLOAT 5 //Floating point number


typedef struct tree{
	unsigned int obj_type;
	unsigned int val_type;
	union {
		int intdata;
		double floatdata;
		char * stringdata;
		tree * cond;
	};
	tree * left;
	tree * right;
} tree;

int yyparse();

tree * build_sint(int val);

tree * build_float(double val);

tree * build_string(char * str);

tree * build_var(char * varname);

tree * build_var(unsigned int id, unsigned int type);

tree * build_unary(tree * left, unsigned int type);

tree * build_dual(tree * left, tree * right, unsigned int type);

tree * build_func_call(char * fn_name, tree * param);

tree * build_callparam(tree * data, tree * nextparam);

tree * build_specparam(tree * data, unsigned int val_type, tree * nextparam);

tree * build_line(tree * data, tree * lastline, int line_number);

tree * build_if(tree * condition, tree * code_if_true, tree * last_condition);

tree * build_else(tree * code, tree * last_condition);

tree * build_while(tree * condition, tree * action);

tree * build_for(tree * start, tree * condition, tree * end, tree * action, int line_number);

tree * build_typeconv(tree * code, unsigned int val_type);

tree * build_subscr_set(tree * var, tree * index, tree * val);

void build_function(char * name, tree * params, tree * treecode, bool global, bool external, unsigned int ret_type);

unsigned int tree_reconstruct(tree * code);

#endif

