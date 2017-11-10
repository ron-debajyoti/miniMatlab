#include "translator.h"
#include <boost/program_options.hpp>
//#include <cstdio>


extern vector <string> allstrings;
extern FILE *yyin;
namespace p_out = boost::program_options;

int global_count=0;
ofstream out;
vector<quad> array;
string file_path;
string input_file;
std::map<int,int> labels;


void findlabels() 
{ // Find all the target for labels from the jump statements
	for (vector<quad>::iterator i=array.begin(); i!=array.end(); i++)
	{
		int count;
		switch (i->op) 
		{
			case L_THAN:
			case G_THAN:
			case L_EQUALS:
			case G_EQUALS:
			case EQUALITY_OP:
			case NOT_EQUAL:
			case GOTO_OP:
				count= atoi(i->result.c_str());
				labels[count] = 1;
				break;
			default:
				break;
		}
	}
	int j= 0;
	for (std::map<int,int>::iterator i=labels.begin(); i!=labels.end(); ++i)
		i->second = j++;
}




string removeExtension(const string filename) 
{ // Used to remove extension from file name
	size_t lastdot = filename.find_last_of('.');
	if(lastdot == string::npos)
		return filename;
	return filename.substr(0, lastdot); 
}



inline bool isInteger(const std::string & s) 
{
	if(s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false ;
	char *p;
	strtol(s.c_str(), &p, 10) ;
	return (*p == 0) ;
}


void computeActivationRecord(symbolTable* s_table) 
{
	int param = 8;
	int local = -4;
	for (list <symbolRow>::iterator it = s_table->table.begin(); it!=s_table->table.end(); it++) 
	{
		if (it->name=="retVal") continue;
		else 
		{
			if (it->category =="param") 
			{
			s_table->act_rec[it->name] = param;
			param +=it->size;			
			}	
			else {
				s_table->act_rec[it->name] = local;
				local -=it->size;
			}
		}

	}
}


void generate_asm() 
{
	array = qarray.arrayOfQuads;
	findlabels();
	list<symbolTable*> tablelist;
	for (list <symbolRow>::iterator i = global_table->table.begin(); i!=global_table->table.end(); i++) {
		if (i->nested!=NULL) tablelist.push_back (i->nested);
	}
	for (list<symbolTable*>::iterator iterator = tablelist.begin(); 
		iterator != tablelist.end(); ++iterator) {
		computeActivationRecord(*iterator);
	}

	ofstream asmfile;
	asmfile.open (file_path.c_str());

	asmfile << "\t.file	\"test.c\"\n";
	for (list <symbolRow>::iterator it = tables->table.begin(); it!=tables->table.end(); it++)
	{
		if (it->category!="function") 
		{
			if (it->type->datatype==T_INT) 
			{ // Global int
				if (it->initial!="") 
				{
					asmfile << "\t.globl\t" << it->name << "\n";
					asmfile << "\t.data\n";
					asmfile << "\t.align 4\n";
					asmfile << "\t.type\t" << it->name << ", @object\n";
					asmfile << "\t.size\t" << it->name << ", 4\n";
					asmfile << it->name <<":\n";
					asmfile << "\t.long\t" << it->initial << "\n";
				}
				else
					asmfile << "\t.comm\t" << it->name << ",4,4\n";
			}
			if (it->type->datatype==T_CHAR) 
			{ // Global char
				if (it->initial!="") 
				{
					asmfile << "\t.globl\t" << it->name << "\n";
					asmfile << "\t.type\t" << it->name << ", @object\n";
					asmfile << "\t.size\t" << it->name << ", 1\n";
					asmfile << it->name <<":\n";
					asmfile << "\t.byte\t" << atoi( it->initial.c_str()) << "\n";
				}
				else 
					asmfile << "\t.comm\t" << it->name << ",1,1\n";
			}
			if (it->type->datatype==T_DOUBLE) 
			{ // Global float
				if (it->initial!="") 
				{
					asmfile << "\t.globl\t" << it->name << "\n";
					asmfile << "\t.align 4\n";
					asmfile << "\t.type\t" << it->name << ", @object\n";
					asmfile << "\t.size\t" << it->name << ", 1\n";
					asmfile << it->name <<":\n";
					asmfile << "\t.long\t" << it->initial << "\n";
				}
				else 
					asmfile << "\t.comm\t" << it->name << ",1,1\n";
			}
		}
	}
	if (allstrings.size()) 
	{
		asmfile << "\t.section\t.rodata\n";
		for (vector<string>::iterator it = allstrings.begin(); it!=allstrings.end(); it++) 
		{
			asmfile << ".LC" << it-allstrings.begin() << ":\n";
			asmfile << "\t.string\t" << *it << "\n";	
		}	
	}

	asmfile << "\t.text	\n";
	for (vector<quad>::iterator it = array.begin(); it!=array.end(); it++) 
	{
		//it->print();
		if (labels.count(it - array.begin())) {
			asmfile << ".L" << (2*global_count+labels.at(it-array.begin())+2) << ": " << endl;
		}
		asmfile << &*it;
		//it->print();
	}
	asmfile << 	"\t.ident\t	\"Compiled by Debajyoti Halder\"\n";
	asmfile << 	"\t.section\t.note.GNU-stack,\"\",@progbits\n";
	asmfile.close();
}






vector<string> parameters;

ostream& operator<<(ostream& os, const quad* q)
{
	operations op = q->op;
	string result = q->result;
	string arg1 = q->arg1;
	string arg2 = q->arg2;

	if(op==PARAM)
	{
		parameters.push_back(result);
		return os;
	}
	os << "\t";

	switch(op) 
	{
		// Binary Operations
		case ADDT: 
		{
			if (isInteger(arg2))
				os << "addl \t$" << atoi(arg2.c_str()) << ", " << tables->act_rec[arg1] << "(%ebp)";
			else {
				os << "movl \t" << tables->act_rec[arg1] << "(%ebp), " << "%eax" << endl;
					os << "\tmovl \t" << tables->act_rec[arg2] << "(%ebp), " << "%edx" << endl;
				os << "\taddl \t%edx, %eax\n";
				os << "\tmovl \t%eax, " << tables->act_rec[result] << "(%ebp)";
			}

			printf("\n1");

			break;
		}
		case SUB:
		{
			os << "movl \t" << tables->act_rec[arg1] << "(%ebp), " << "%eax" << endl;
			os << "\tmovl \t" << tables->act_rec[arg2] << "(%ebp), " << "%edx" << endl;
			os << "\tsubl \t%edx, %eax\n";
			os << "\tmovl \t%eax, " << tables->act_rec[result] << "(%ebp)";

			printf("\n2");
			
			break;
		}
		case MULT: 
		{
			os << "movl \t" << tables->act_rec[arg1] << "(%ebp), " << "%eax" << endl;
			if (isInteger(arg2))
				os << "\timull \t$" << atoi(arg2.c_str()) << ", " << "%eax" << endl;
			else
				os << "\timull \t" << tables->act_rec[arg2] << "(%ebp), " << "%eax" << endl;
			os << "\tmovl \t%eax, " << tables->act_rec[result] << "(%ebp)";
			break;		

			printf("\n3");
			
		}
		case DIV:
		{
			os << "movl \t" << tables->act_rec[arg1] << "(%ebp), " << "%eax" << endl;
			os << "\tcltd" << endl;
			os << "\tidivl \t" << tables->act_rec[arg2] << "(%ebp)" << endl;
			os << "\tmovl \t%eax, " << tables->act_rec[result] << "(%ebp)";


			printf("\n4");
			
			break;			
		}

		// Unary Operators
		case AMPERSAND:
		{
			os << "leal\t" << tables->act_rec[arg1] << "(%ebp), %eax\n";
			os << "\tmovl \t%eax, " <<  tables->act_rec[result] << "(%ebp)";


			printf("\n5");
			
			break;
		}
		case PTR_RIGHT: 
		{
			os << "movl\t" << tables->act_rec[arg1] << "(%ebp), %eax\n";
			os << "\tmovl\t(%eax),%eax\n";
			os << "\tmovl \t%eax, " <<  tables->act_rec[result] << "(%ebp)";

			printf("\n6");
			
			break;	
		}
		case PTR_LEFT: 
		{
			os << "movl\t" << tables->act_rec[result] << "(%ebp), %eax\n";
			os << "\tmovl\t" << tables->act_rec[arg1] << "(%ebp), %edx\n";
			os << "\tmovl\t%edx, (%eax)";

			printf("\n7");
			
			break;
		} 			
		case UMINUS:
		{
			os << "negl\t" << tables->act_rec[arg1] << "(%ebp)";

			printf("\n8");
			
			break;
		}

		// Bit Operators /* Ignored */
		case MODULO:			os << result << " = " << arg1 << " % " << arg2;				break;
		case XOR:			os << result << " = " << arg1 << " ^ " << arg2;				break;
		case B_OR:			os << result << " = " << arg1 << " | " << arg2;				break;
		case B_AND:			os << result << " = " << arg1 << " & " << arg2;				break;
		// Shift Operations /* Ignored */
		case L_SHIFT:		os << result << " = " << arg1 << " << " << arg2;				break;
		case R_SHIFT:		os << result << " = " << arg1 << " >> " << arg2;				break;

							
		// Relational Operations
		case EQUALITY_OP: 
		{
			os << "movl\t" << tables->act_rec[arg1] << "(%ebp), %eax\n";
			os << "\tcmpl\t" << tables->act_rec[arg2] << "(%ebp), %eax\n";
			os << "\tje .L" << (2*global_count+labels.at(atoi( result.c_str() )) +2);

			printf("\n9");
			
			break;
		}
		case NOT_EQUAL: 
		{
			os << "movl\t" << tables->act_rec[arg1] << "(%ebp), %eax\n";
			os << "\tcmpl\t" << tables->act_rec[arg2] << "(%ebp), %eax\n";
			os << "\tjne .L" << (2*global_count+labels.at(atoi( result.c_str() )) +2 );

			printf("\n10");
			
			break;
		}
		
		case EQUAL:	
		{
			if(isInteger(arg1))
				os << "movl\t$" << atoi(arg1.c_str()) << ", " << "%eax" << endl;
			else
				os << "movl\t" << tables->act_rec[arg1] << "(%ebp), " << "%eax" << endl;
			os << "\tmovl \t%eax, " << tables->act_rec[result] << "(%ebp)";

			printf("\n11");
			
			break;
		}			
		case EQUAL_STR:	
		{
			os << "movl \t$.LC" << arg1 << ", " << tables->act_rec[result] << "(%ebp)";

			printf("\n12");
			
			break;
		}
		case EQUAL_CHAR:	
		{
			os << "movb\t$" << atoi(arg1.c_str()) << ", " << tables->act_rec[result] << "(%ebp)";

			printf("\n13");
			
			break;
		}

		case G_EQUALS:
		{
			os << "movl\t" << tables->act_rec[arg1] << "(%ebp), %eax\n";
			os << "\tcmpl\t" << tables->act_rec[arg2] << "(%ebp), %eax\n";
			os << "\tjge .L" << (2*global_count+labels.at(atoi( result.c_str() )) +2 );

			printf("\n14");
			
			break;
		}
		case L_EQUALS:
		{
			os << "movl\t" << tables->act_rec[arg1] << "(%ebp), %eax\n";
			os << "\tcmpl\t" << tables->act_rec[arg2] << "(%ebp), %eax\n";
			os << "\tjle .L" << (2*global_count+labels.at(atoi( result.c_str() )) +2 );

			printf("\n15");
			
			break;
		}
		case L_THAN: 
		{
			os << "movl\t" << tables->act_rec[arg1] << "(%ebp), %eax\n";
			os << "\tcmpl\t" << tables->act_rec[arg2] << "(%ebp), %eax\n";
			os << "\tjl .L" << (2*global_count+labels.at(atoi( result.c_str() )) +2 );

			printf("\n16");
			
			break;
		}
		case G_THAN:
		{
			os << "movl\t" << tables->act_rec[arg1] << "(%ebp), %eax\n";
			os << "\tcmpl\t" << tables->act_rec[arg2] << "(%ebp), %eax\n";
			os << "\tjg .L" << (2*global_count+labels.at(atoi( result.c_str() )) +2 );

			printf("\n17");
			
			break;
		}
		case GOTO_OP:
		{
			os << "jmp .L" << (2*global_count+labels.at(atoi( result.c_str() )) +2 );

			printf("\n18");
			
			break;
		}
		
		case BINARY_NOT:			os << result 	<< " = ~" << arg1;				break;
		case LOGICAL_NOT:			os << result 	<< " = !" << arg1;				break;
		case MATRIX_RIGHT: 
		{
			os << "movl\t" << tables->act_rec[arg2] << "(%ebp), "<<"%eax" << endl;
			os << "\tmovl\t" << tables->act_rec[arg1] << "(%ebp,%eax,1), "<<"%eax" << endl;
			os << "\tmovl \t%eax, " <<  tables->act_rec[result] << "(%ebp)";

			printf("\n19");
			
			break;
		}	 			
		case MATRIX_LEFT: 
		{
			os << "movl\t" << tables->act_rec[arg1] << "(%ebp), "<<"%eax" << endl;
			os << "\tmovl\t" << tables->act_rec[arg2] << "(%ebp), "<<"%edx" << endl;
			os << "\tmovl\t" << "%edx, " << tables->act_rec[result] << "(%ebp,%eax,1)";

			printf("\n20");
			
			break;
		}	 
		case _RETURN: 
		{
			if(result!="") os << "movl\t" << tables->act_rec[result] << "(%ebp), "<<"%eax";
			else os << "nop";

			printf("\n21");
			
			break;
		}
		case PARAM: 
		{
			parameters.push_back(result);

			printf("\n22");
			
			break;
		}
		case CALL: 
		{

			printf("\n23");
			
			symbolTable* t = global_table->lookup (arg1)->nested;
			int i;	// index
			for (list <symbolRow>::iterator it = t->table.begin(); it!=t->table.end(); it++) {
				i = distance ( t->table.begin(), it);
				if (it->category== "param") 
				{
					os << "movl \t" << tables->act_rec[parameters[i]] << "(%ebp), " << "%eax" << endl;
					os << "\tmovl \t%eax, " << (t->act_rec[it->name]-8 )<< "(%esp)\n\t";
				}
				else break;
			}
			parameters.clear();
			os << "call\t"<< arg1 << endl;
			os << "\tmovl\t%eax, " << tables->act_rec[result] << "(%ebp)";
			break;
		}
		case FUNC: 
		{
			os <<".globl\t" << result << "\n";
			os << "\t.type\t"	<< result << ", @function\n";
			os << result << ": \n";
			os << ".LFB" << global_count <<":" << endl;
			os << "\t.cfi_startproc" << endl;
			os << "\tpushl \t%ebp" << endl;
			os << "\t.cfi_def_cfa_offset 8" << endl;
			os << "\t.cfi_offset 5, -8" << endl;
			os << "\tmovl \t%esp, %ebp" << endl;
			os << "\t.cfi_def_cfa_register 5" << endl;
			tables = global_table->lookup(result)->nested;
			os << "\tsubl\t$" << tables->table.back().offset << ", %esp";

			printf("\n24");
			
			break;
		}		
		case F_END: 
		{
			os << "leave\n";
			os << "\t.cfi_restore 5\n";
			os << "\t.cfi_def_cfa 4, 4\n";
			os << "\tret\n";
			os << "\t.cfi_endproc" << endl;
			os << ".LFE" << global_count++ <<":" << endl;
			os << "\t.size\t"<< result << ", .-" << result;

			printf("\n26");
			
			break;

		}
		default:			os << "op";							break;
	}
	os << endl;
	return os;
}



void trans() 
{
	global_table = new symbolTable("Global");
	tables = global_table;
	yyin = fopen(input_file.c_str(),"r"); 
//	out << "here" << input_file;
	yyparse();
	tables = global_table;
	tables->compute();
	tables->print(1);
	qarray.print ();
	generate_asm();
}


template<class T>
ostream& operator<<(ostream& os, const vector<T>& v)
{
	copy(v.begin(), v.end(), ostream_iterator<T>(os, " ")); 
	return os;
}


int  main(int argc, char* argv[])
{
	int x;
	string ofile_path;
	
	try 
	{
		p_out::options_description desc("Available options");
		desc.add_options()
		("help", " --> help message")
		("debug", p_out::value<int>(&x), "set debug mode on/off")
		("file_path,s", p_out::value< vector<string> >(), "filename for .s file")
		("ofilename,l", p_out::value< vector<string> >(), "filename for log file")
		("input-file,i", p_out::value< vector<string> >(), "input file name")
		;

		p_out::positional_options_description p;
		p.add("input-file", -1);

		p_out::variables_map vm;
		p_out::store(p_out::command_line_parser(argc, argv).
			options(desc).positional(p).run(), vm);
		p_out::notify(vm);

		if (vm.count("help")) {
			out << desc << "\n";
			return 0;
		}

		if (vm.count("input-file"))	input_file = vm["input-file"].as< vector<string> >()[0];
		else input_file = "test.c";

		if (vm.count("ofilename")) ofile_path = vm["ofilename"].as< vector<string> >()[0];
		else ofile_path = removeExtension(input_file) = "test.log";

		if (vm.count("file_path")) file_path = vm["file_path"].as< vector<string> >()[0];
		else file_path = removeExtension(input_file) = "asm.s";


		out.open (ofile_path.c_str());
	}
	catch(exception& e) {
		cerr << "error: " << e.what() << "\n";
		return 1;
	}
	catch(...) {
		cerr << "Exception of unknown type!\n";
	}

//    std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
  //  std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!

	cout << "Input file is " << input_file << endl;
	cout << "Assembly file generated is " << file_path << endl;
	cout << "Quads and Symbol Table are stored in " << ofile_path <<  endl << endl;
//	input_file = "test.c";
	trans();
};





