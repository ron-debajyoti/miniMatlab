//%error-verbose
%define parse.error verbose


%{


// C declarations and definitions
#include <string.h>
#include <stdio.h>
#include "translator.h"
extern int yylex();
void yyerror(const char *s);

extern types TYPE;
extern bool gDebug;
vector <string> allstrings;

%}

%union
{
	char c_val;
	int intval;					// intval
	float floatval;				// floatval
	char* charval;				// charval
	int instr;					// instr
	symbolRow* symp;			// symp
	expression* exp;			// exp
	list<int>* nl;				//nl
	symbol* st;					//st
	statement* stat;			//stat
	unary* A;					//un
	char uop;					//uop
}

%token <symp> ID  PUNC  COMMENT;
%token <intval>	INT_NUM;
%token <charval> FLOAT_NUM;
%token <c_val> CHAR_CONST;
%token <charval> STRING_LITERAL;

%token MULT_COMMENT;
%token UNSIGNED_KW;
%token BREAK_KW;
%token RETURN_KW; 		
%token VOID_KW;
%token CASE_KW;
%token FLOAT_KW;
%token CHAR_KW;
%token SHORT_KW;
%token FOR_KW;
%token SIGNED_KW;
%token WHILE_KW;
%token GOTO_KW;
%token BOOL_KW;
%token CONTINUE_KW;
%token IF_KW;
%token DEFAULT_KW;
%token DO_KW;
%token INT_KW;
%token SWITCH_KW;
%token DOUBLE_KW;
%token LONG_KW;
%token ELSE_KW;
%token MATRIX_KW;
%token CONSTANT
%token ZERO_NO_CONST;


%token LEFT_SQ_BRACKT;
%token RIGHT_SQ_BRACKT;
%token LEFT_BRACKT;
%token RIGHT_BRACKT;
%token LEFT_CUR_BRACKT;
%token RIGHT_CUR_BRACKT;
%token STOP;
%token ARROW;
%token INCREMENT;
%token DECREMENT;
%token BITWISE_AND;
%token MULTIPLY;
%token ADD;
%token SUBTRACT;
%token TILDA;
%token NEGATION;
%token DIVIDE;
%token PERCENT;
%token SHL;
%token SHR;
%token LESS_THAN;
%token GREATER_THAN;
%token LESS_THAN_EQUALS;
%token GREATER_THAN_EQUALS;
%token EQUALITY;
%token NOT_EQUALS;
%token POWER;
%token BITWISE_OR;
%token AND;
%token LOGICAL_OR;
%token QUESTION;
%token COLON;
%token SEMICOLON;
%token EQUALS;
%token MULTIPLY_EQUALS;
%token DIVIDE_EQUALS;
%token MODULO_EQUALS;
%token ADD_EQUALS;
%token SUBTRACT_EQUALS;
%token SHL_EQUALS;
%token SHR_EQUALS;
%token AND_EQUALS;
%token POWER_EQUALS;
%token OR_EQUALS;
%token COMMA;
%token HASH;
%token UNIQUE;

%right THEN_KW ELSE_KW
%expect 1

%start translation_unit

// All Expressions 

%type <A> 
	postfix
	unary
	cast

%type <exp> 
	expression
	primary
	multiplicative
	additive
	shift
	relational
	equality
	and
	exclusive_or
	inclusive_or
	logical_and
	logical_or
	conditional
	assignment
	expression_statement

%type <uop> unary_operator
%type <symp> CONSTANT_expr initializer
%type <instr> M
%type <exp> N
%type <st> pointer
%type <symp> direct_declarator init_declarator declarator
%type <intval> argument_list

%type <stat> statement
	labeled 
	compound
	selection
	iteration
	jump
	block_item
	block_item_list


%%


primary
	: ID 
	{
		$$ = new expression();
		$$->symp = $1;
		$$->isbool = false;
		//printf("1\n");
	}
	| CONSTANT_expr
	{
		$$ = new expression();
		$$->symp = $1;
		//printf("2\n");
	}
	| STRING_LITERAL 
	{
		$$ = new expression();
		$$->symp = gentemp(T_POINTER, $1);
		$$->symp->type->ptr = new symbol(T_CHAR);
		$$->symp->initialize($1);
		allstrings.push_back($1);
		emit(EQUAL_STR,$$->symp->name,tostr(allstrings.size()-1));
		//printf("3\n");
	}
	| LEFT_BRACKT expression RIGHT_BRACKT 
	{
		$$=$2;
		//printf("4\n");
	}	
	;


CONSTANT_expr
	: INT_NUM
	{
		ostringstream ss;
		ss << $1;
		string p = ss.str();
		$$ = gentemp(T_INT, p);
		emit(EQUAL, $$->name, $1);
		//printf("5\n");
	}
	| FLOAT_NUM
	{
		$$ = gentemp(T_DOUBLE, *new string($1));
		emit(EQUAL, $$->name, *new string($1));
		//printf("6\n");
	}
	| CHAR_CONST
	{
		$$ = gentemp(T_CHAR);
		emit(EQUAL, $$->name, "a");
		//printf("7\n");
	}
	;





postfix
	: primary 
	{
		$$ = new unary ();
		$$->symp = $1->symp;
		$$->loc = $1->symp;
		$$->type = $1->symp->type;
		//printf("8\n");
	}
	| postfix LEFT_SQ_BRACKT expression RIGHT_SQ_BRACKT
	{
		$$ = new unary();		
		$$->symp = $1->symp;			// base is being copied
		$$->type = $1->type->ptr;		// type = type of element
		$$->loc = gentemp(T_INT);		// store computed address
		
		// New address = already computed + $3 * new width
		if ($1->datatype==T_MATRIX) 
		{	// if something already computed
			symbolRow* t = gentemp(T_INT);
			ostringstream ss;
			ss << sizeoftype($$->type);
			string p = ss.str();
 			emit(MULT, t->name, $3->symp->name, p);
			emit (ADDT, $$->loc->name, $1->loc->name, t->name);
		}
 		else 
 		{	// if computation is being done first time 
 			ostringstream ss;
			ss << sizeoftype($$->type);
			string p = ss.str();
	 		emit(MULT, $$->loc->name, $3->symp->name, p);
 		}

 		// Mark that it contains address of matrix and first time computation is done
		$$->datatype = T_MATRIX;
		//printf("9\n");
	} 
	| postfix LEFT_BRACKT argument_list RIGHT_BRACKT 
	{
		$$ = new unary();
		$$->symp = gentemp($1->type->datatype);
		emit(CALL, $$->symp->name, $1->symp->name, tostr($3));
		//printf("10\n");
	}
	| postfix LEFT_BRACKT RIGHT_BRACKT 	// nothing to add
	| postfix STOP ID 					// nothing to add
	| postfix ARROW ID 					// nothing to add
	| postfix INCREMENT 
	{
		$$ = new unary();

		// copy $1 to $$
		$$->symp = gentemp($1->symp->type->datatype);
		emit(EQUAL, $$->symp->name, $1->symp->name);

		// Increment $1
		emit(ADDT, $1->symp->name, $1->symp->name, "1");
		//printf("11\n");
	}
	| postfix DECREMENT 
	{
		$$ = new unary();

		// copy $1 to $$
		$$->symp = gentemp($1->symp->type->datatype);
		emit(EQUAL, $$->symp->name, $1->symp->name);

		// Decrement $1
		emit(SUB, $1->symp->name, $1->symp->name, "1");
		//printf("12\n");

	}
	| postfix UNIQUE 				// nothing to add
	;



argument_list
	: assignment
	{
		emit(PARAM, $1->symp->name);
		$$ = 1;
		//printf("13\n");
	} 
	| argument_list COMMA assignment 
	{
		emit(PARAM, $3->symp->name);
		$$ = $1+1;
		//printf("14\n");
	}
	;




unary
	: postfix 
	{
		$$ = $1;
		//printf("15\n");
	}
	| INCREMENT unary 
	{
		// Increment $1
		emit(ADDT, $2->symp->name, $2->symp->name, "1");
		// Use the same value
		$$ = $2;
		//printf("16\n");
	}
	| DECREMENT unary 
	{
		// Decrement $1
		emit(SUB, $2->symp->name, $2->symp->name, "1");
		// Use the same value
		$$ = $2;
		//printf("17\n");
	}
	| unary_operator cast 
	{
		$$ = new unary();
		switch ($1) 
		{
			case '&':
				$$->symp = gentemp(T_POINTER);
				$$->symp->type->ptr = $2->symp->type; 
				emit(AMPERSAND, $$->symp->name, $2->symp->name);
				break;
			case '*':
				debug("got pointer");
				$$->datatype = T_POINTER;
				debug($2->symp->name);
				$$->loc = gentemp($2->symp->type->ptr);
				emit(PTR_RIGHT, $$->loc->name, $2->symp->name);
				$$->symp = $2->symp;
				debug("here pointer");
				break;
			case '+':
				$$ = $2;
				break;
			case '-':
				$$->symp = gentemp($2->symp->type->datatype);
				emit (UMINUS, $$->symp->name, $2->symp->name);
				break;
			case '~':
				$$->symp = gentemp($2->symp->type->datatype);
				emit (BINARY_NOT, $$->symp->name, $2->symp->name);
				break;
			case '!':
				$$->symp = gentemp($2->symp->type->datatype);
				emit (LOGICAL_NOT, $$->symp->name, $2->symp->name);
				break;
			default:
				break;

			//printf("18\n");
		}
	}	
	;





unary_operator 
	: BITWISE_AND 
	{
	 	$$=B_AND;
	 	//printf("19\n");
	}
	| MULTIPLY 
	{
		$$=MULT;
		//printf("20\n");
	}
	| ADD 
	{
		$$=ADDT;
		//printf("21\n");
	}
	| SUBTRACT 
	{
		$$=SUB;
		//printf("22\n");
	}	
	;




cast
	: unary 
	{
		$$=$1;
		//printf("23\n");
	}
	;





multiplicative
	: multiplicative MULTIPLY cast 
	{
		if (typecheck ($1->symp, $3->symp) ) 
		{
			$$ = new expression();
			$$->symp = gentemp(T_INT);
			emit(MULT, $$->symp->name, $1->symp->name, $3->symp->name);
		}
		else cout << "Type Error"<< endl;

		//printf("24\n");
	}
	| multiplicative DIVIDE cast 
	{
		if (typecheck($1->symp, $3->symp) ) 
		{
			$$ = new expression();
			$$->symp = gentemp(T_INT);
			emit(DIV, $$->symp->name, $1->symp->name, $3->symp->name);
		}
		else cout << "Type Error"<< endl;

		//printf("25\n");
	}
	| multiplicative PERCENT cast 
	{
		if (typecheck ($1->symp, $3->symp) ) 
		{
			$$ = new expression();
			$$->symp = gentemp(T_INT);
			emit(MODULO, $$->symp->name, $1->symp->name, $3->symp->name);
		}
		else cout << "Type Error"<< endl;

		//printf("26\n");
	}
	| cast 
	{
		$$ = new expression();
		if ($1->datatype==T_MATRIX) 
		{ 	// For Matrix
			$$->symp = gentemp($1->loc->type);
			emit(MATRIX_RIGHT, $$->symp->name, $1->symp->name, $1->loc->name);
		}
		else if ($1->datatype==T_POINTER) 
		{ 	// Pointer
			$$->symp = $1->loc;
		}
		else 
		{ 	// otherwise rest all cases
			$$->symp = $1->symp;
		}

		//printf("27\n");
	}
	;






additive
	: multiplicative 
	{
	 	$$=$1;
	 	//printf("28\n");
	}
	| additive ADD multiplicative 
	{
		if (typecheck($1->symp, $3->symp)) 
		{
			$$ = new expression();
			$$->symp = gentemp(T_INT);
			emit (ADDT, $$->symp->name, $1->symp->name, $3->symp->name);
		}
		else cout << "Type Error"<< endl;

		//printf("29\n");
	}
	| additive SUBTRACT multiplicative 
	{
		if (typecheck($1->symp, $3->symp)) 
		{
			$$ = new expression();
			$$->symp = gentemp(T_INT);
			emit (SUB, $$->symp->name, $1->symp->name, $3->symp->name);
		}
		else cout << "Type Error" << endl;

		//printf("30\n");
	}
	;




shift 
	: additive 
	{
		$$=$1;
		//printf("31\n");
	}
	| shift SHL additive 
	{
		if($3->symp->type->datatype == T_INT) 
		{
			$$ = new expression();
			$$->symp = gentemp (T_INT);
			emit (L_SHIFT, $$->symp->name, $1->symp->name, $3->symp->name);
		}
		else cout << "Type Error"<< endl;

		//printf("32\n");
	}
	| shift SHR additive 
	{
		if($3->symp->type->datatype == T_INT) 
		{
			$$ = new expression();
			$$->symp = gentemp(T_INT);
			emit (R_SHIFT, $$->symp->name, $1->symp->name, $3->symp->name);
		}
		else cout << "Type Error"<< endl;

		//printf("33\n");
	}
	;





relational
	: shift 
	{
		$$=$1;
		//printf("34\n");
	}
	| relational LESS_THAN shift 
	{
		if(typecheck($1->symp, $3->symp)) 
		{
			$$ = new expression();
			$$->isbool = true;
			$$->trueList = makelist(nextInstruction());
			$$->falseList = makelist(nextInstruction()+1);
			emit(L_THAN, "", $1->symp->name, $3->symp->name);
			emit (GOTO_OP, "");
		}
		else cout << "Type Error"<< endl;

		//printf("35\n");
	}
	| relational GREATER_THAN shift 
	{
		if (typecheck ($1->symp, $3->symp) ) 
		{
			$$ = new expression();
			$$->isbool = true;
			$$->trueList = makelist(nextInstruction());
			$$->falseList = makelist(nextInstruction()+1);
			emit(G_THAN, "", $1->symp->name, $3->symp->name);
			emit (GOTO_OP, "");
		}
		else cout << "Type Error"<< endl;

		//printf("36\n");
	}
	| relational LESS_THAN_EQUALS shift
	{
		if (typecheck($1->symp, $3->symp) ) 
		{
			$$ = new expression();
			$$->isbool = true;
			$$->trueList = makelist(nextInstruction());
			$$->falseList = makelist(nextInstruction()+1);
			emit(L_EQUALS, "", $1->symp->name, $3->symp->name);
			emit (GOTO_OP, "");
		}
		else cout << "Type Error"<< endl;

		//printf("37\n");
	} 
	| relational GREATER_THAN_EQUALS shift 
	{
		if (typecheck($1->symp, $3->symp) ) 
		{
			$$ = new expression();
			$$->isbool = true;
			$$->trueList = makelist(nextInstruction());
			$$->falseList = makelist(nextInstruction()+1);
			emit(G_EQUALS, "", $1->symp->name, $3->symp->name);
			emit (GOTO_OP, "");
		}
		else cout << "Type Error"<< endl;


		//printf("38\n");
	}
	;




equality
	: relational 
	{
		$$=$1;
		//printf("39\n");
	}
	| equality EQUALITY relational 
	{
		if(typecheck($1->symp, $3->symp) ) 
		{
			// If any is bool has a value
			if ($1->isbool) 
			{
				$1->symp = gentemp(T_INT);
				backpatch($1->trueList, nextInstruction());
				emit(EQUAL, $1->symp->name, "true");
				emit (GOTO_OP, tostr(nextInstruction()+1));
				backpatch ($1->falseList, nextInstruction());
				emit (EQUAL, $1->symp->name, "false");
			}
			
			if($3->isbool) 
			{
				$3->symp = gentemp(T_INT);
				backpatch($3->trueList, nextInstruction());
				emit(EQUAL, $3->symp->name, "true");
				emit (GOTO_OP, tostr(nextInstruction()+1));
				backpatch ($3->falseList, nextInstruction());
				emit (EQUAL, $3->symp->name, "false");
			}

			$$ = new expression();
			$$->isbool = true;			
			$$->trueList = makelist(nextInstruction());
			$$->falseList = makelist(nextInstruction()+1);
			emit (EQUALITY_OP, "", $1->symp->name, $3->symp->name);
			emit (GOTO_OP, "");
		}
		else cout << "Type Error"<< endl;

		//printf("40\n");
	}
	| equality NOT_EQUALS relational 
	{
		if(typecheck ($1->symp, $3->symp) ) 
		{
			// If any is bool get its value
			if ($1->isbool) 
			{
				$1->symp = gentemp(T_INT);
				backpatch($1->trueList, nextInstruction());
				emit(EQUAL, $1->symp->name, "true");
				emit (GOTO_OP, tostr(nextInstruction()+1));
				backpatch ($1->falseList, nextInstruction());
				emit (EQUAL, $1->symp->name, "false");
			}
			
			if ($3->isbool) 
			{
				$3->symp = gentemp(T_INT);
				backpatch($3->trueList, nextInstruction());
				emit(EQUAL, $3->symp->name, "true");
				emit (GOTO_OP, tostr(nextInstruction()+1));
				backpatch ($3->falseList, nextInstruction());
				emit (EQUAL, $3->symp->name, "false");
			}

			$$ = new expression();
			$$->isbool = true;			
			$$->trueList = makelist (nextInstruction());
			$$->falseList = makelist (nextInstruction()+1);
			emit (NOT_EQUAL, $$->symp->name, $1->symp->name, $3->symp->name);
			emit (GOTO_OP, "");
		}
		else cout << "Type Error"<< endl;


		//printf("41\n");
	}
	;




and
	: equality
	{
		$$=$1;
		//printf("***///744\n");
	} 
	| and BITWISE_AND equality 
	{
		if (typecheck($1->symp, $3->symp))
		{
			$$ = new expression();
			$$->isbool = false;
			$$->symp = gentemp (T_INT);
			emit(B_AND, $$->symp->name, $1->symp->name, $3->symp->name);
		}
		else cout << "Type Error"<< endl;

		//printf("42\n");
	}
	;





exclusive_or
	: and 
	{
		$$=$1;
		//printf("43\n");
	}
	| exclusive_or POWER and 
	{
		if(typecheck($1->symp,$3->symp)) 
		{
			// If any is bool get its value
			if($1->isbool) 
			{
				$1->symp = gentemp(T_INT);
				backpatch($1->trueList, nextInstruction());
				emit(EQUAL, $1->symp->name, "true");
				emit (GOTO_OP, tostr(nextInstruction()+1));
				backpatch ($1->falseList, nextInstruction());
				emit (EQUAL, $1->symp->name, "false");
			}
			
			if ($3->isbool) 
			{
				$3->symp = gentemp(T_INT);
				backpatch($3->trueList, nextInstruction());
				emit(EQUAL, $3->symp->name, "true");
				emit (GOTO_OP, tostr(nextInstruction()+1));
				backpatch ($3->falseList, nextInstruction());
				emit (EQUAL, $3->symp->name, "false");
			}

			$$ = new expression();
			$$->isbool = false;
			$$->symp = gentemp (T_INT);
			emit (XOR, $$->symp->name, $1->symp->name, $3->symp->name);
		}
		else cout << "Type Error"<< endl;

		//printf("44\n");
	}
	;





inclusive_or
	: exclusive_or 
	{
		$$=$1;
		//printf("45\n");
	}
	| inclusive_or BITWISE_OR exclusive_or 
	{
		if (typecheck($1->symp, $3->symp)) 
		{
			// If any is bool get its value
			if ($1->isbool) 
			{
				$1->symp = gentemp(T_INT);
				backpatch($1->trueList, nextInstruction());
				emit(EQUAL, $1->symp->name, "true");
				emit (GOTO_OP, tostr(nextInstruction()+1));
				backpatch ($1->falseList, nextInstruction());
				emit (EQUAL, $1->symp->name, "false");
			}
			
			if ($3->isbool) 
			{
				$3->symp = gentemp(T_INT);
				backpatch($3->trueList, nextInstruction());
				emit(EQUAL, $3->symp->name, "true");
				emit (GOTO_OP, tostr(nextInstruction()+1));
				backpatch ($3->falseList, nextInstruction());
				emit (EQUAL, $3->symp->name, "false");
			}

			$$ = new expression();
			$$->isbool = false;			
			$$->symp = gentemp (T_INT);
			emit (B_OR, $$->symp->name, $1->symp->name, $3->symp->name);
		}
		else cout << "Type Error"<< endl;

		//printf("46\n");
	}	
	;




logical_and
	: inclusive_or 
	{
		$$=$1;
		//printf("47\n");
	}
	| logical_and N AND M inclusive_or 
	{
		if (!$5->isbool) 
		{
			$5->falseList = makelist (nextInstruction());
			emit(EQUALITY_OP, "", $5->symp->name, "0");
			$5->trueList = makelist (nextInstruction());
			emit(GOTO_OP, "");
		}

		//convert $1 to bool
		backpatch($2->nextlist, nextInstruction());
		if (!$1->isbool) 
		{
			$1->falseList = makelist (nextInstruction());
			emit(EQUALITY_OP, "", $1->symp->name, "0");
			$1->trueList = makelist (nextInstruction());
			emit(GOTO_OP, "");
		}

		$$ = new expression();
		$$->isbool = true;
		backpatch($1->trueList, $4);
		$$->trueList = $5->trueList;
		$$->falseList = merge ($1->falseList, $5->falseList);

		//printf("48\n");
	}
	;

logical_or 
	: logical_and 
	{
		$$=$1;
		//printf("49\n");
	}
	| logical_or N LOGICAL_OR M logical_and 
	{
		if (!$5->isbool) 
		{
			$5->falseList = makelist (nextInstruction());
			emit(EQUALITY_OP, "", $5->symp->name, "0");
			$5->trueList = makelist (nextInstruction());
			emit(GOTO_OP, "");
		}

		//to convert $1 to bool
		backpatch($2->nextlist, nextInstruction());
		if (!$1->isbool) 
		{
			$1->falseList = makelist (nextInstruction());
			emit(EQUALITY_OP, "", $1->symp->name, "0");
			$1->trueList = makelist (nextInstruction());
			emit(GOTO_OP, "");
		}

		$$ = new expression();
		$$->isbool = true;
		backpatch($$->falseList, $4);
		$$->trueList = merge($1->trueList, $5->trueList);
		$$->falseList = $5->falseList;

		//printf("50\n");
	}
	;



M 	: %empty{	// To store the address of the next instruction for further use.
		$$ = nextInstruction();
		//printf("51\n");
	};

N 	: %empty { 	// Non terminal to prevent fallthrough by emitting a goto
		debug ("n");
		$$  = new expression();
		$$->nextlist = makelist(nextInstruction());
		emit(GOTO_OP,"");
		debug("n2");

		//printf("52\n");
	}



conditional
	: logical_or 
	{
		$$=$1;

		//printf("53\n");
	}
	| logical_or N QUESTION M expression N COLON M conditional 
	{
		if (!$5->isbool) 
		{
			$5->falseList = makelist (nextInstruction());
			emit(EQUALITY_OP, "", $5->symp->name, "0");
			$5->trueList = makelist (nextInstruction());
			emit(GOTO_OP, "");
		}

		$$->symp = gentemp();
		$$->symp->update($5->symp->type);
		emit(EQUAL, $$->symp->name, $9->symp->name);
		list<int> l = makelist(nextInstruction());
		emit (GOTO_OP, "");
		backpatch($6->nextlist, nextInstruction());
		emit(EQUAL, $$->symp->name, $5->symp->name);
		list<int> m = makelist(nextInstruction());
		l = merge(l,m);
		emit (GOTO_OP, "");
		backpatch($2->nextlist, nextInstruction());
		if (!$1->isbool) 
		{
			$1->falseList = makelist (nextInstruction());
			emit(EQUALITY_OP, "", $1->symp->name, "0");
			$1->trueList = makelist (nextInstruction());
			emit(GOTO_OP, "");
		}

		backpatch ($1->trueList, $4);
		backpatch ($1->falseList, $8);
		backpatch (l, nextInstruction());

		//printf("54\n");
	}
	;




assignment
	: conditional 
	{
		$$=$1;

		//printf("55\n");
	}
	| unary assignment_operator assignment 
	{
		switch ($1->datatype) 
		{
			case T_MATRIX:
				emit(MATRIX_LEFT, $1->symp->name, $1->loc->name, $3->symp->name);	
				break;
			case T_POINTER:
				emit(PTR_LEFT, $1->symp->name, $3->symp->name);	
				break;
			default:
				emit(EQUAL, $1->symp->name, $3->symp->name);
				break;
		}
		$$ = $3;

		//printf("56\n");
	}
	;

assignment_operator
	: EQUALS 
	| MULTIPLY_EQUALS 
	| DIVIDE_EQUALS 
	| MODULO_EQUALS 
	| ADD_EQUALS 
	| SUBTRACT_EQUALS 
	| SHL_EQUALS 
	| SHR_EQUALS 
	| AND_EQUALS 
	| POWER_EQUALS 
	| BITWISE_OR 
	{printf("assignment_operator\n");}
	;



/*

//////////////// EXPRESSION PHASE ////////////////////////

*/



expression 
	: assignment 
	{
		$$=$1;

		//printf("57\n");
	}
	| expression COMMA assignment 
	{
		printf("expression\n");

		//printf("58\n");
	}
	;

constant_expression
	: conditional 
	{
		printf("constant_expression\n");

		//printf("59\n");
	}
	;



/*
	 /////////// DECLARATIONS PHASE ///////////////

*/




declaration
	: declaration_specifiers init SEMICOLON 
	{
		debug ("declaration");

		////printf("60\n");
	}
	| declaration_specifiers SEMICOLON
	{
		//printf("61\n");
	}
	;


init
	: init_declarator 
	| init COMMA init_declarator 
	{
		debug("init");
		//printf("62\n");
	}
	;

declaration_specifiers
	: type_specifier 	
	| type_specifier declaration_specifiers
	{
		printf("declaration_specifiers\n");
		//printf("63\n");
	}
	;


init_declarator
	: declarator 
	{
		$$=$1;
		//printf("64\n");
	}
	| declarator EQUALS initializer 
	{
		debug($1->name);
		debug($3->name);
		debug($3->initial);
		if ($3->initial!="") $1->initialize($3->initial);
		emit (EQUAL, $1->name, $3->name);
		debug ("here initial");

		//printf("65\n");
	}
	;

type_specifier
	: VOID_KW 
	{
		TYPE=T_VOID;

		//printf("66\n");
	}
	| CHAR_KW
	{
		TYPE=T_CHAR;

		//printf("67\n");
	}
	| SHORT_KW
	| INT_KW
	{
		TYPE=T_INT;

		//printf("68\n");
	}
	| LONG_KW
	| FLOAT_KW 
	| DOUBLE_KW 
	{
		TYPE=T_DOUBLE;

		//printf("69\n");
	}
	| MATRIX_KW 
	{
		TYPE=T_MATRIX;

		//printf("70\n");
	}
	| SIGNED_KW 
	| UNSIGNED_KW 
	| BOOL_KW 
	;



declarator
	: pointer direct_declarator 
	{
		symbol *t = $1;
		while(t->ptr!=NULL) 
			t = t->ptr;
		t->ptr = $2->type;
		$$ = $2->update($1);

		//printf("71\n");
	}
	| direct_declarator 
	
	;

direct_declarator
	: ID 
	{
		$$ = $1->update(TYPE);
		debug("encounter: "<< $$->name);
		encounter = $$;

		//printf("72\n");
	}
	| LEFT_BRACKT declarator RIGHT_BRACKT 
	{
		$$=$2;

		//printf("73\n");
	}
	| direct_declarator '[' ']' 
	{
		symbol *t = $1->type;
		symbol *prev =NULL;
		while(t->datatype == T_MATRIX) 
		{
			prev = t;
			t = t->ptr;
		}
		if (prev==NULL) 
		{
			symbol* s = new symbol(T_MATRIX,$1->type,0);
			int y = sizeoftype(s);
			$$ = $1->update(s);
		}
		else 
		{
			prev->ptr =  new symbol(T_MATRIX,t,0);
			$$ = $1->update ($1->type);
		}

		//printf("74\n");
	}
	| direct_declarator LEFT_SQ_BRACKT assignment RIGHT_SQ_BRACKT 
	{
		symbol *t = $1->type;
		symbol *prev = NULL;
		while (t->datatype == T_MATRIX) 
		{
			prev = t;
			t = t->ptr;
		}
		if (prev==NULL) 
		{
			int x = atoi($3->symp->initial.c_str());
			symbol *s = new symbol(T_MATRIX, $1->type, x);
			int y = sizeoftype(s);
			$$ = $1->update(s);
		}
		else 
		{
			prev->ptr = new symbol(T_MATRIX, t, atoi($3->symp->initial.c_str()));
			$$ = $1->update($1->type);
		}

		//printf("75\n");
	}
	| direct_declarator LEFT_BRACKT CST parameter_type RIGHT_BRACKT 
	{
		tables->tableName = $1->name;
		if ($1->type->datatype!=T_VOID) 
		{
			symbolRow *s = tables->lookup("retVal");
			s->update($1->type);		
		}
		$1 = $1->linkst(tables);
		tables->parent = global_table;
		changeTable(global_table);	
		debug ("encounter: "<< $$->name);
		encounter = $$;

		//printf("76\n");
	}
	| direct_declarator LEFT_BRACKT ID_list RIGHT_BRACKT 
	| direct_declarator LEFT_BRACKT CST RIGHT_BRACKT
	{
		tables->tableName = $1->name;			// Update function symbol table name

		if ($1->type->datatype !=T_VOID) {
			symbolRow *s = tables->lookup("retVal");// Update type of return value
			s->update($1->type);
		}
		
		$1 = $1->linkst(tables);		// Update type of function in global table	
		tables->parent = global_table;
		changeTable(global_table);				// Come back to globalsymbol table
	
		debug ("encounter: "<< $$->name);
		encounter = $$;

		//printf("77\n");
	}
	;



CST : %empty { // Used for changing to symbol table for a function
		if (encounter->nested==NULL) 
			changeTable(new symbolTable(""));	// Function symbol table doesn't already exist
		else changeTable (encounter ->nested);						// Function symbol table already exists
		emit(CALL, tables->tableName);

		//printf("78\n");
	}
	;


pointer
	: MULTIPLY pointer 
	{
		$$ = new symbol(T_POINTER,$2);

		//printf("79\n");
	}
	| MULTIPLY 
	{
		$$ = new symbol(T_POINTER);

		//printf("80\n");
	}
	;

parameter_type
	: parameter_list 
	;

parameter_list
	: parameter_declaration 
	| parameter_list COMMA parameter_declaration 
	{
		debug("parameter_list\n");

		//printf("81\n");
	}
	;

parameter_declaration
	: declaration_specifiers declarator 
	{
		debug ("here");
		//$2->category = "param";

		//printf("82\n");
	}
	| declaration_specifiers
	;

ID_list
	: ID 
	| ID_list COMMA ID 
	{
		printf("ID_list\n");

		//printf("83\n");
	}
	;

initializer
	: assignment 
	{
		$$ = $1->symp;

		//printf("84\n");
	}
	| LEFT_CUR_BRACKT initializer_row_list RIGHT_CUR_BRACKT 
	{
		printf("initializer\n");

		//printf("85\n");
	}
	;

initializer_row_list
	: initializer_row 
	| initializer_row_list SEMICOLON initializer_row 
	{
		printf("initializer_row_list\n");

		//printf("86\n");
	}
	;

initializer_row
	: designation initializer 
	| initializer 
	| initializer_row COMMA designation initializer 
	| initializer_row COMMA initializer 
	{
		printf("initializer_row\n");

		//printf("87\n");
	}
	;

designation
	: designator_list EQUALS 
	{
		printf("designation\n");

		//printf("88\n");
	}
	;

designator_list
	: designator
	| designator_list designator
	{
		printf("designator_list\n");

		//printf("89\n");
	}
	;

designator
	: LEFT_SQ_BRACKT constant_expression RIGHT_SQ_BRACKT 
	| STOP ID
	{
		printf("designator\n");

		//printf("90\n");
	} 
	;




/**

	/////////////// STATEMENT PHASE ////////////////////
**/




statement
	: labeled
	| compound
	{
		$$ = $1;
		debug("compound\n");

		//printf("91\n");
	}
	| expression_statement
	{
		$$ = new statement();
		$$->nextlist = $1->nextlist;
		debug("expression_statement\n");

		//printf("92\n");
	}
	| selection
	{
		$$ = $1;
		debug("selection\n");

		//printf("93\n");
	}
	| iteration
	{
		$$ = $1;
		debug("iteration\n");

		//printf("94\n");
	}
	| jump
	{
		$$ = $1;
		debug("jump\n");

		//printf("95\n");
	}
	;

labeled 
	: ID COLON statement
	{
		$$ = new statement();

		//printf("96\n");
	}
	| CASE_KW constant_expression COLON statement
	{
		$$ = new statement();

		//printf("97\n");
	}
	| DEFAULT_KW COLON statement
	{
		$$ = new statement();

		//printf("98\n");
	}
	;



compound
	:LEFT_CUR_BRACKT block_item_list RIGHT_CUR_BRACKT
	{
		$$ = $2;

		//printf("99\n");
	}
	| LEFT_CUR_BRACKT RIGHT_CUR_BRACKT
	{
		$$ = new statement();

		//printf("100\n");
	}
	;




block_item_list
	: block_item
	{
		$$ = $1;

		//printf("101\n");
	}
	| block_item_list M block_item
	{
		$$ = $3;
		backpatch($1->nextlist,$2);

		//printf("102\n");
	}
	;

block_item
	: declaration
	{
		$$ = new statement();

		//printf("103\n");
	}
	| statement
	{
		$$ = $1;

		//printf("104\n");
	}
	;

expression_statement
	: expression SEMICOLON
	{
		$$ = $1;

		//printf("105\n");
	}
	| SEMICOLON
	{
		$$ = new expression();

		//printf("106\n");
	}
	;


selection
	: IF_KW LEFT_BRACKT expression N RIGHT_BRACKT M statement N   %prec THEN_KW
	{
		backpatch ($4->nextlist, nextInstruction());
		
		if(!$3->isbool) 
		{
			$3->falseList = makelist(nextInstruction());
			emit(EQUALITY_OP, "", $3->symp->name, "0");
			$3->trueList = makelist(nextInstruction());
			emit(GOTO_OP, "");
		}
		$$ = new statement();
		backpatch ($3->trueList, $6);
		list<int> temp = merge($3->falseList, $7->nextlist);
		$$->nextlist = merge($8->nextlist, temp);

		//printf("107\n");
	}
	| IF_KW LEFT_BRACKT expression N RIGHT_BRACKT M statement N ELSE_KW M statement
	{
		backpatch ($4->nextlist, nextInstruction());
		if(!$3->isbool) 
		{
			$3->falseList = makelist(nextInstruction());
			emit(EQUALITY_OP, "", $3->symp->name, "0");
			$3->trueList = makelist(nextInstruction());
			emit(GOTO_OP, "");
		}
		backpatch($3->trueList, $6);
		backpatch($3->falseList, $10);
		list<int> temp = merge($7->nextlist, $8->nextlist);
		$$->nextlist = merge(temp, $11->nextlist);

		//printf("108\n");
	}
	| SWITCH_KW LEFT_BRACKT expression RIGHT_BRACKT statement
	{
		printf("selection statement");

		//printf("109\n");
	}
	;





iteration
	: WHILE_KW M LEFT_BRACKT expression RIGHT_BRACKT M statement
	{
		$$ = new statement();
		if(!$4->isbool) 
		{
			$4->falseList = makelist(nextInstruction());
			emit(EQUALITY_OP, "", $4->symp->name, "0");
			$4->trueList = makelist(nextInstruction());
			emit(GOTO_OP, "");
		}
		backpatch($7->nextlist, $2);
		backpatch($4->trueList, $6);
		$$->nextlist = $4->falseList;
		// Emit to prevent fallthrough
		emit (GOTO_OP, tostr($2));

		//printf("110\n");
	}
	| DO_KW M statement M WHILE_KW LEFT_BRACKT expression RIGHT_BRACKT SEMICOLON
	{
		$$ = new statement();
		if(!$7->isbool) 
		{
			$7->falseList = makelist(nextInstruction());
			emit(EQUALITY_OP, "", $7->symp->name, "0");
			$7->trueList = makelist(nextInstruction());
			emit(GOTO_OP, "");
		}
		backpatch($7->trueList, $2);
		backpatch($3->nextlist, $4);
		$$->nextlist = $7->falseList;

		//printf("111\n");
	}
	| FOR_KW LEFT_BRACKT expression SEMICOLON M expression SEMICOLON expression RIGHT_BRACKT M statement
	{
		$$ = new statement();
		if(!$6->isbool) 
		{
			$6->falseList = makelist(nextInstruction());
			emit(EQUALITY_OP, "", $6->symp->name, "0");
			$6->trueList = makelist(nextInstruction());
			emit(GOTO_OP, "");
		}
		backpatch ($6->trueList, $10);
		backpatch ($11->nextlist, $5);
		emit (GOTO_OP, tostr($5));
		$$->nextlist = $6->falseList;


		//printf("112\n");
	}
	| FOR_KW LEFT_BRACKT declaration M expression SEMICOLON M expression N RIGHT_BRACKT M statement
	{
		$$ = new statement();
		if(!$5->isbool) 
		{
			$5->falseList = makelist(nextInstruction());
			emit(EQUALITY_OP, "", $5->symp->name, "0");
			$5->trueList = makelist(nextInstruction());
			emit(GOTO_OP, "");
		}
		backpatch ($5->trueList, $11);
		backpatch ($9->nextlist, $4);
		backpatch ($12->nextlist, $7);
		emit (GOTO_OP, tostr($7));
		$$->nextlist = $5->falseList;


		//printf("113\n");
	}
	;

jump
	: GOTO_KW ID SEMICOLON
	{
		$$ = new statement();

		//printf("114\n");
	}
	| CONTINUE_KW SEMICOLON
	{
		$$ = new statement();

		//printf("115\n");
	}
	| BREAK_KW SEMICOLON
	{
		$$ = new statement();

		//printf("116\n");
	}
	| RETURN_KW expression SEMICOLON
	{
		$$ = new statement();
		emit(_RETURN,$2->symp->name);

		//printf("117\n");
	}
	| RETURN_KW SEMICOLON
	{
		$$ = new statement();
		emit(_RETURN,"");

		//printf("118\n");
	}
	;




/***
	//////////////////// TRANSLATION PHASE ////////////////
*/




translation_unit
	: external_dec 
	{
		//printf("121\n");
	}
	| translation_unit external_dec 
	{
		//printf("122\n");
	}
	;

external_dec
	: function_def 
	{
		//printf("123\n");
	}
	| declaration 
	{
		//printf("124\n");
	}
	;

function_def
	: declaration_specifiers declarator declaration_list CST compound 
	{
		//printf("119\n");
	}
	| declaration_specifiers declarator CST compound 
	{
		tables->parent = global_table;
		changeTable(global_table);

		//printf("120\n");
	}
	;

declaration_list
	: declaration 
	{printf("125\n");}
	| declaration_list declaration 
	{
		//printf("126\n");
	}
	;


%%

void yyerror(const char *s) {
	printf ("ERROR: %s",s);
}




