#ifndef TRANSLATOR
#define TRANSLATOR
#include <bits/stdc++.h>
#include <iostream>
#include <vector>
#include <algorithm>

#define sizeofchar		1
#define sizeofint  		4
#define sizeofdouble	8
#define sizeofpointer	4

#define debug(x) do { \
  if (gDebug) { cerr << x << std::endl; } \
} while (0)

/**
 #define size_of_matrix  	no.ofelements*sizeofdouble
							+ no.ofdimensions*sizeofint

only 2 dimensional matrices to be used 
*/

extern int yyparse();
extern char* yytext;
extern bool gDebug;

using namespace std;



/** Classes used as DS **/

class symbol;				// Element type in symbol table 
class symbolTable;			// Symbol table
class symbolRow;			// Symbol Row of the symbol table
class quad;					// Element of quad table 
class quads;				// Quad Array
//class symboltype;			// symbol type in the symbol table


/* Constant integer structure declarations **/ 

enum types {
	T_VOID,
	T_CHAR,
	T_INT,
	T_DOUBLE,
	T_POINTER,
	T_MATRIX,
};



enum operations {
	EQUAL, EQUAL_STR, EQUAL_CHAR,
// Relational Operators
	L_THAN, G_THAN, L_EQUALS, G_EQUALS,_RETURN, EQUALITY_OP, NOT_EQUAL,
//BINARY OPERATORS
	ADDT, SUB, MULT,DIV,L_SHIFT,R_SHIFT, MODULO,
// Unary Operators
	UMINUS, UPLUS, AMPERSAND, BINARY_NOT, LOGICAL_NOT,
// Bit Operators
	B_AND, XOR, B_OR, 
// Pointer Assign
	PTR_LEFT, PTR_RIGHT,
// Matrix Assign
	MATRIX_RIGHT, MATRIX_LEFT,
// Function call
	PARAM, CALL, FUNC, F_END, GOTO_OP
};





class quad { 					// Individual entities of quad
public:
	operations op;				// Type of the operator used here 
	string result;				// Result
	string arg1;				// Argument 
	string arg2;				// Argument 

	quad (string result, string arg1, operations op = EQUAL, string arg2 = "");
	quad (string result, int arg1, operations op = EQUAL, string arg2 = NULL);
	void print ();				// Print Quads
	void update (int address);		// Used for backpatching
	
};



class quads { 							// Array of quads
public:
	vector <quad> arrayOfQuads;;		// Vector of quads

	quads () {arrayOfQuads.reserve(300);}
	void printTabular();				// Print quads in tabular form
	void print ();
};

void emit(operations op1, string result, string arg1="", string arg2 = "");
void emit(operations op2, string result, int arg1, string arg2 = "");


class symbol { 					// Element type in symbol table
public:
	symbol(types datatype, symbol* ptr = NULL, int width = 1);
	int width;					// width of array i.e no. of elements in the array
	int dimension;				// no. of dimensions.. if it is 2 dim. matrix then its 2 .. rest of 
								// cases dimension=1
	symbol* ptr;				// Array -> array of ptr type; pointer-> pointer to ptr type 
	types datatype;
	
	friend ostream& operator<<(ostream&, const symbol);
};



class symbolRow { 				// Row in a Symbol Table
public:
	string name;				// Name of symbol
	symbol *type;				// Type of Symbol
	string initial;				// Symbol initialisation
	string category;
	int size;					// Size of the type of symbol
	int offset;					// Offset of symbol computed at the end
	symbolTable* nested;		// Pointer to nested symbol table

	symbolRow (string, types t=T_INT, symbol* ptr = NULL, int width = 0);
	symbolRow* update(symbol *t); 	// Update using type object and nested table pointer
	symbolRow* update(types t); 		// Update using raw type and nested table pointer
	symbolRow* initialize (string);
	friend ostream& operator<<(ostream&, const symbolTable*);
	symbolRow* linkst(symbolTable* t);
};



class symbolTable { 			// Symbol Table
public:
	string tableName;				// Name of Table
	int tableCount;					// Count of temporary variables
	list <symbolRow> table; 				// The table of symbol
	symbolTable* parent;			// pointer of the parent table
	map<string,int> act_rec; 			// This indicates Activation Record

	symbolTable (string name="");
	symbolRow* lookup(string name);			// Lookup for a symbol in symbol table
	void print(int all_Tables=0 );			// Print the symbol table
	void compute();							// Compute offset of table recursively
	
};

symbolRow* gentemp (types t=T_INT, string init = "");	// Generate a temporary variable and insert it in symbol table
symbolRow* gentemp (symbol* t, string init = "");		// Generate a temporary variable and insert it in symbol table
	

list<int> makelist (int);							// Make a new list
list<int> merge (list<int> &, list <int> &);		// Merge two lists

symbolRow* conv (symbolRow*, types);
bool typecheck(symbolRow* &s1, symbolRow* &s2);		// Checks if two symbol table entries have same type
bool typecheck(symbol* s1, symbol* s2);				// Check if the type objects are equivalent
void backpatch (list <int>, int);					// Function for backpatching 
int sizeoftype (symbol*);							// Calculate size of any type
string convert_to_string (const symbol*);			// For printing type structure
string optostr(int);								// Converting an Operand to a string

int nextInstruction();								// Returns the address of the next instruction
void changeTable (symbolTable* newtable);			// Changing the table


/* Global variables in cxx file */
extern symbolTable* tables;			// Current Symbol Table
extern symbolTable* global_table;			// Global Symbol Table
extern quads qarray;						// quad_array
extern symbolRow* encounter; 			// Pointer to just encountered symbol


template <typename T> string tostr(const T& t) 
{ 
	ostringstream os; 
	os<<t; 
	return os.str(); 
} 


/*
	 Extra structures for considering Boolean attributes 
*/

struct expression
{
	bool isbool;		// Boolean variable that stores if the expression is bool

	// Valid for non-bool type
	symbolRow* symp;			// Pointer to the symbol table entry

	// Valid for bool type
	list<int> trueList,falseList;		// Truelist and FalseList valid for boolean

	// Valid for statement expression
	list<int> nextlist;
};

struct statement 
{
	list<int> nextlist;		// Nextlist for statement
};

struct unary 
{
	types datatype;
	symbolRow* loc;			// Temporary used for computing array address
	symbolRow* symp;			// Pointer to symbol table
	symbol* type;		// type of the subarray generated
};


class Singleton {			// Global Symbol Table is Singleton Object
public:
   static Singleton* GetInstance();
   Singleton();
   static Singleton* pSingleton;					// singleton instance
};


void printlist (list<int> list);			// Print the list of integers

#endif









