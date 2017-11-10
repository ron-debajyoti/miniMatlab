
#include "translator.h"
#include "y.tab.h"

/* Global variables in cxx file */

symbolTable* global_table;			// Global Symbol Table
quads qarray;						// quad_array
types TYPE;
symbolTable* tables;	// pointer to current table
symbolRow* encounter;			// pointer to encountered symbol
bool gDebug=false;			// for debug mode


Singleton* Singleton::pSingleton= NULL;
Singleton::Singleton() {}
Singleton* Singleton::GetInstance() {
	if (pSingleton== NULL) {
		pSingleton = new Singleton();
	}
	return pSingleton;
}



int sizeoftype (symbol* t)
{
	switch (t->datatype) 
	{
		case T_VOID:
			return 0;
		case T_CHAR:
			return sizeofchar;
		case T_INT:
			return sizeofint;
		case T_DOUBLE:
			return sizeofdouble;
		case T_POINTER:
			return sizeofpointer;
		case T_MATRIX:
			return (t->width * sizeoftype(t->ptr) + t->dimension*sizeofint);
		default:
			return 0;
	}
}


string convert_to_string (const symbol* t)
{
	if (t==NULL) return "null";
	else
	{
		switch (t->datatype) 
		{
			case T_VOID:
				return "void";
			case T_CHAR:
				return "char";
			case T_INT:
				return "int";
			case T_DOUBLE:
				return "double";
			case T_POINTER:
				return "ptr("+ convert_to_string(t->ptr)+")";
			case T_MATRIX:
				return "arr[" + tostr(t->width) + ", "+ convert_to_string (t->ptr) + "]"+"["+tostr(t->dimension)+"]";
			default:
				return "type unidentified";
		}
	}
}



/** Considering the Classes symbol, symbolTable,symbolRow, functions are:
	lookup()
	gentemp()
	update()
	print()
	initialize()
	**/

// Class symbol
symbol::symbol(types datatype, symbol* ptr, int width): 
	datatype (datatype), 
	ptr (ptr), 
	dimension(1),
	width (width) {};


ostream& operator<<(ostream& os, const symbol* sym) 
{
	types datatype = sym->datatype;
	string str = convert_to_string(sym);
	os << str;
	return os;
}


// Class symbolRow


symbolRow::symbolRow (string name, types datatype, symbol* ptr, int width): name(name)  
{
	type = new symbol(symbol(datatype, ptr, width));
	nested = NULL;
	initial = "";
	category = "";
	offset = 0;
	size = sizeoftype(type);
}

symbolRow* symbolRow::linkst(symbolTable* t)
{
	this->nested = t;
	this->category = "function";
}

symbolRow* symbolRow::initialize(string init) 
{
	this->initial = initial;
}

symbolRow* symbolRow::update(symbol* t) 
{
	type = t;
	this->size = sizeoftype(t);
	//if(gDebug) 
	//	tables->print();
	return this;
}

symbolRow* symbolRow::update(types t) 
{
	this->type = new symbol(t);
	this->size = sizeoftype(this->type);
	//if(gDebug) 
	//	tables->print();
	return this;
}

ostream& operator<<(ostream& os, const symbolRow* row) 
{
	os << left << setw(16) << row->name;
	os << left << setw(16) << row->type;
	os << left << setw(12) << row->category;
	os << left << setw(12);
	if (row->type->datatype == T_POINTER && row->type->ptr!=NULL && row->type->ptr->datatype == T_CHAR) os << "...";
	else os << row->initial;
	os << left << setw(8) << row->size;
	os << left << setw(8) << row->offset;
	os << left;
	//if (gDebug)	printf("%p\t", it);
	if (row->nested == NULL) 
	{
		os << "null" <<  endl;	
	}
	else 
	{
	//os << "not null" << endl;
		os << row->nested->tableName <<  endl;
	}
	return os;
}



//Class symbolTable
symbolRow* symbolTable::lookup (string name) 
{
	symbolRow* sym;
	list <symbolRow>::iterator i;
	for (i=table.begin();i!=table.end();i++) 
	{
		if(i->name == name ) break;
	}
	if (i!=table.end() ) 
	{
		if(gDebug) 
			cout << name << " Name already present" << endl;
		return &*i;
	}
	else 
	{
		/*
		for (i= gTable->table.begin();i!=gTable->table.end();i++) 
		{
			if(i->name == name ) 
				break;
		}
		if (i!=gTable->table.end() ) 
		{
			if(gDebug)
				cout << name << " already present in global table" << endl;
			return &*i;
		}
		else 
		{
			*/
			sym = new symbolRow(name);
			sym->category="local";
			table.push_back(*sym);
			//if(gDebug) 
			//	print();
			return &table.back();
		//}		
	}
}

symbolTable::symbolTable (string name): tableName (name), tableCount(0) 
{}

symbolRow* gentemp(symbol* t, string init) 
{
	char character[20];
	sprintf(character, "t%02d", tables->tableCount++);
	symbolRow* sym = new symbolRow(character);
	sym->type = t;
	sym->initial= init;
	sym->category="temp";
	tables->table.push_back (*sym);
	//if(gDebug)
	//	tables->print();
	return &tables->table.back();
}

symbolRow* gentemp(types t, string init) 
{
	char character[20];
	sprintf(character, "t%02d", tables->tableCount++);
	symbolRow* sym = new symbolRow(character,t);
	sym-> initial = init;
	sym->category = "temp";
	tables->table.push_back(*sym);
	//if(gDebug)
	//	tables->print();
	return &tables->table.back();
}






void symbolTable::compute() 
{
	list<symbolTable*> List;
	int count;
	for(list <symbolRow>::iterator i=table.begin();i!=table.end();i++) 
	{
		if (i == table.begin()) 
		{
			i->offset = 0;
			count=i->size;
		}
		else 
		{
			i->offset = count;
			count = i->offset + i->size;
		}
		if(i->nested!=NULL) 
			List.push_back(i->nested);
	}
	for(list<symbolTable*>::iterator iterator = List.begin();iterator!= List.end();++iterator) 
	{
		//debug ("computing for child");
	    (*iterator)->compute();
		//if (gDebug) (*iterator)->print();
	}
}


void symbolTable::print(int tables_all) 
{
	list<symbolTable*> List;
	cout << '\n' << endl;
	cout << setw(80) << setfill ('=') << "="<< endl;
	cout << "\nSymbol Table::: " << setfill (' ') << left << setw(35)  << this->tableName ;
	cout << right << setw(20) << "Parent: ";
	if (this->parent!=NULL)
		cout << this->parent->tableName;
	else cout << "null" ;
	cout << endl;
	cout << setw(80) << setfill ('-') << "-"<< endl;
	cout << setfill (' ') << left << setw(16) << "Name";
	cout << left << setw(16) << "Type";
	cout << left << setw(12) << "Init Val";
	cout << left << setw(8) << "Size";
	cout << left << setw(8) << "Offset";
	cout << left << "Nested Table" << endl;
	cout << setw(80) << setfill ('-') << "-"<< setfill (' ') << endl;
	
	for (list <symbolRow>::iterator it = table.begin(); it!=table.end(); it++) 
	{
		cout << &*it;
		if (it->nested!=NULL) List.push_back (it->nested);
	}
	cout << setw(80) << setfill ('-') << "-"<< setfill (' ') << endl;
	cout << endl;
	if (tables_all) 
	{
		for (list<symbolTable*>::iterator iterator = List.begin();iterator!= List.end();++iterator)
		{
		    (*iterator)->print();
		}		
	}
}
/**
	Functions for quad and quadArray(quads)


**/
quad::quad (string result, string arg1, operations op, string arg2):
	result (result), arg1(arg1), arg2(arg2), op (op){};

quad::quad (string result, int arg1, operations op, string arg2):
	result (result), arg2(arg2), op (op) 
	{
		ostringstream ss;
		ss << arg1;
		this->arg1= ss.str();
	}

void quad::update (int address) 
{	// Used for backpatching address
	this->result = address;
}

void quad::print ()
{
	switch(op) 
	{
		// Binary Operations
		case ADDT:			cout << result << " = " << arg1 << " + " << arg2;				break;
		case SUB:			cout << result << " = " << arg1 << " - " << arg2;				break;
		case MULT:			cout << result << " = " << arg1 << " * " << arg2;				break;
		case DIV:			cout << result << " = " << arg1 << " / " << arg2;				break;
		case MODULO:			cout << result << " = " << arg1 << " % " << arg2;				break;
		case XOR:			cout << result << " = " << arg1 << " ^ " << arg2;				break;
		case L_SHIFT:			cout << result << " = " << arg1 << " << " << arg2;				break;
		case R_SHIFT:			cout << result << " = " << arg1 << " >> " << arg2;				break;
		case B_OR:			cout << result << " = " << arg1 << " | " << arg2;				break;
		case B_AND:			cout << result << " = " << arg1 << " & " << arg2;				break;
		case EQUAL:			cout << result << " = " << arg1 ;								break;
		// Relational Operations
		case EQUALITY_OP:			cout << "if " << arg1 <<  " == " << arg2 << " goto " << result;				break;
		case NOT_EQUAL:			cout << "if " << arg1 <<  " != " << arg2 << " goto " << result;				break;
		case L_THAN:			cout << "if " << arg1 <<  " < "  << arg2 << " goto " << result;				break;
		case G_THAN:			cout << "if " << arg1 <<  " > "  << arg2 << " goto " << result;				break;
		case G_EQUALS:			cout << "if " << arg1 <<  " >= " << arg2 << " goto " << result;				break;
		case L_EQUALS:			cout << "if " << arg1 <<  " <= " << arg2 << " goto " << result;				break;
		case GOTO_OP:		cout << "goto " << result;						break;
		//Unary Operators
		case AMPERSAND:		cout << result << " = &" << arg1;				break;
		case PTR_RIGHT:			cout << result	<< " = *" << arg1 ;				break;
		case PTR_LEFT:			cout << "*" << result	<< " = " << arg1 ;		break;
		case UMINUS:		cout << result 	<< " = -" << arg1;				break;
		case UPLUS:			cout << result 	<< " = +" << arg1;				break;
		case BINARY_NOT:			cout << result 	<< " = ~" << arg1;				break;
		case LOGICAL_NOT:			cout << result 	<< " = !" << arg1;				break;

		case MATRIX_RIGHT:	 		cout << result << " = " << arg1 << "[" << arg2 << "]";			break;
		case MATRIX_LEFT:	 		cout << result << "[" << arg1 << "]" <<" = " <<  arg2;			break;
		case _RETURN: 		cout << "ret " << result;				break;
		case PARAM: 		cout << "param " << result;				break;
		case CALL: 			cout << result << " = " << "call " << arg1<< ", " << arg2;				break;
		case FUNC: 			cout << result << ": ";
		case F_END:			break;
		default:			cout << "op";							break;
	}
	cout << endl;	
}


void quads::printTabular() 
{
	cout << "---------Quad Table ---------" << endl;
	cout << setw(8) << "index";
	cout << setw(8) << " op";
	cout << setw(8) << "arg 1";
	cout << setw(8) << "arg 2";
	cout << setw(8) << "result" << endl;
	for (vector<quad>::iterator i= arrayOfQuads.begin(); i!=arrayOfQuads.end(); i++) 
	{
		cout << left << setw(8) << i-arrayOfQuads.begin(); 
		cout << left << setw(8) << optostr(i->op);
		cout << left << setw(8) << i->arg1;
		cout << left << setw(8) << i->arg2;
		cout << left << setw(8) << i->result << endl;
	}
}


void quads::print () 
{
	cout << setw(30) << setfill ('=') << "="<< endl;
	cout << "Quad Translation" << endl;
	cout << setw(30) << setfill ('-') << "-"<< endl;
	for (vector<quad>::iterator it = arrayOfQuads.begin(); it!=arrayOfQuads.end(); it++) 
	{
		switch(it->op)
		{
			case FUNC:
				cout << "\n";
				it->print();
				cout << "\n";
			case F_END:
				break;
			default:
				cout << it - arrayOfQuads.begin() << ":\t";
				it->print();
		}
	}
	cout << setw(30) << setfill ('-') << "-"<< endl;
}




void emit(operations op, string result, string arg1, string arg2)
{
	qarray.arrayOfQuads.push_back(*(new quad(result,arg1,op,arg2)));
	if(gDebug) 
		qarray.print();
}

void emit(operations op, string result, int arg1, string arg2) 
{
	qarray.arrayOfQuads.push_back(*(new quad(result,arg1,op,arg2)));
	if(gDebug) 
		qarray.print();
}



/*** 

	Global Functions are declared here now 

**/



// function makelist()
list<int> makelist(int i) 
{
	list<int> l(1,i);
	return l;
}

// function mergelist
list<int> merge(list<int> &a, list<int> &b) 
{
	a.merge(b);
	return a;
}



symbolRow* conv(symbolRow* s, types t) 
{
	symbolRow* temp = gentemp(t);
	switch(s->type->datatype) 
	{
		case T_INT: 
		{
			switch(t) 
			{
				case T_DOUBLE: 
				{
					emit (EQUAL, temp->name, "int2double(" + s->name + "}");
					return temp;
				}
				case T_CHAR: 
				{
					emit (EQUAL, temp->name, "int2char(" + s->name + "}");
					return temp;
				}
			}
		}
		case T_DOUBLE: 
		{
			switch (t) 
			{
				case T_INT: 
				{
					emit (EQUAL, temp->name, "double2int2double(" + s->name + "}");
					return temp;
				}
				case T_CHAR: 
				{
					emit (EQUAL, temp->name, "double2char(" + s->name + "}");
					return temp;
				}
			}
		}
		case T_CHAR: 
		{
			switch (t) 
			{
				case T_INT: 
				{
					emit (EQUAL, temp->name, "char2int(" + s->name + "}");
					return temp;
				}
				case T_DOUBLE: 
				{
					emit (EQUAL, temp->name, "char2double(" + s->name + "}");
					return temp;
				}
			}
		}
	}
	return NULL;
}




bool typecheck(symbolRow*& s1, symbolRow*& s2)
{ 	// Check if the symbols have same type or not
	symbol* type1 = s1->type;
	symbol* type2 = s2->type;
	if(typecheck(type1,type2)) 
		return true;
	else if(s1 = conv(s1,type2->datatype)) return true;
	else if(s2 = conv(s2,type1->datatype)) return true;
	return false;
}

bool typecheck(symbol* t1, symbol* t2)
{ 	// Check if the symbol types are same or not
	if (t1 != NULL || t2 != NULL) 
	{
		if (t1==NULL) return false;
		if (t2==NULL) return false;
		if (t1->datatype==t2->datatype) return (t1->ptr, t2->ptr);
		else return false;
	}
	return true;
}

void backpatch (list<int> l, int addr) 
{
	for(list<int>::iterator it= l.begin(); it!=l.end(); it++) 
		qarray.arrayOfQuads[*it].result = tostr(addr);

	if(gDebug) 
		qarray.printTabular();
}

string optostr (int op)
{
	switch(op) 
	{
		case ADDT:				return " + ";
		case SUB:			return " - ";
		case MULT:			return " * ";
		case DIV:			return " / ";
		case EQUAL:				return " = ";
		case EQUALITY_OP:			return " == ";
		case NOT_EQUAL:			return " != ";
		case L_THAN:			return " < ";
		case G_THAN:		return " > ";
		case G_EQUALS:	return " >= ";
		case L_EQUALS:		return " <= ";
		case L_SHIFT: 		return " <<";
		case R_SHIFT:		return " >>";
		case GOTO_OP:				return " goto ";
		//Unary Operators
		case AMPERSAND:			return " &";
		case PTR_RIGHT:			return " *r";
		case PTR_LEFT:			return " *l";
		case UMINUS:			return " --";
		case UPLUS:				return " ++";
		case BINARY_NOT:		return " ~";
		case LOGICAL_NOT:		return " !";

		case MATRIX_RIGHT:	 		return " =[]R";
		case MATRIX_LEFT:	 		return " L=[]";
		case _RETURN: 			return " return";
		case PARAM: 			return " param ";
		case CALL: 				return " call ";
		case FUNC:   			return " function ";
		case F_END:				return " end ";
		default:				return " op ";
	}
}

int nextInstruction() 
{
	return qarray.arrayOfQuads.size();
}


void changeTable (symbolTable *newtable) 
{	// Change current symbol table
	if(gDebug)	
		cout << "Symbol table changed from " << tables->tableName;
	tables = newtable;
	if(gDebug)
		cout << " to " << tables->tableName << endl;
} 



void printlist (list<int> l)
{	// Print integers in list l 
	for (list<int>::iterator i= l.begin(); i!= l.end(); ++i) 
	{
	    if(gDebug) cout << *i << " ";
	}
	cout << endl;
}





/*
/// The Main Function
int main(int argc, char* argv[])
{
	if (argc>1) 
		gDebug = true;

	global_table = new symbolTable("Global");
	tables = global_table;
	yyparse();
	tables->compute();
	tables->print(1);
//	qarray.printtab();
	qarray.print();
	int n, x;
	cin >> n;
	if (n==10) 
	{
		while (n--) 
		{
			cin >> x;
			if(x==1) 
			{
				gentemp(T_DOUBLE);
			}
			else if (x==2) 
			{
				emit(ADDT, "a", "b", "c");
			}
		}	
	}
	
};


*/