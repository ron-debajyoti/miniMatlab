%{

#include "translator.h"
#include "y.tab.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
extern void yyerror(const char *);

void comment(void);
void count(void);

%}

%option noyywrap

/* regex expressions */

MULTI_LINE_COMM 	\/\*([^*]|[\r\n]|(\*+([^*/]|[\r\n])))*\*+\/
SINGLE_LINE_COMM	\/\/(.)*

UNSIGNED	"unsigned"
BREAK		"break"
RETURN 		"return"
VOID 		"void"
CASE 		"case"
FLOAT 		"float"
CHAR 		"char"
SHORT 		"short"
FOR 		"for"
SIGNED		"signed"
WHILE 		"while"
GOTO 		"goto"
BOOL 		"Bool"
CONTINUE 	"continue"
IF 			"if"
DEFAULT 	"default"
DO 			"do"
INT 		"int"
SWITCH 		"switch"
DOUBLE 		"double"
LONG 		"long"
ELSE 		"else"
MATRIX 		"Matrix"

ID 					[a-zA-Z][a-zA-Z0-9]*
INT_CONST 			[1-9][0-9]*
ZERO_CONST 			[0]
DIG_SEQ 			[0-9]+
FRACTION_CONST		{DIG_SEQ}?\.{DIG_SEQ}|{DIG_SEQ}\.
SIGN 				[+-]?
EXPONENT_PART 		[eE]?{SIGN}{DIG_SEQ}
FLOAT_CONST			({FRACTION_CONST}{EXPONENT_PART}?)|{DIG_SEQ}{EXPONENT_PART}
ESCAPE_SEQ 			\\['"?\\abnfrvt]
C_CHAR				[^'\\\n]|{ESCAPE_SEQ}
C_CHAR_SEQ			{C_CHAR}+
CHAR_CON			\'{C_CHAR_SEQ}\'
S_CHAR				[^"\\\n]|{ESCAPE_SEQ}
S_CHAR_SEQ 			{S_CHAR}+
S_LITERAL			\"{S_CHAR_SEQ}\"


%%
{MULTI_LINE_COMM} 			{return MULT_COMMENT;}
{SINGLE_LINE_COMM}			{comment();}
{UNSIGNED}					{return UNSIGNED_KW;}
{BREAK}						{return BREAK_KW;}
{RETURN}					{return RETURN_KW;} 		
{VOID} 						{return VOID_KW;}
{CASE} 						{return CASE_KW;}
{FLOAT} 					{return FLOAT_KW;}
{CHAR} 						{return CHAR_KW;}
{SHORT} 					{return SHORT_KW;}
{FOR} 						{return FOR_KW;}
{SIGNED}					{return SIGNED_KW;}
{WHILE} 					{return WHILE_KW;}
{GOTO} 						{return GOTO_KW;}
{BOOL} 						{return BOOL_KW;}
{CONTINUE} 					{return CONTINUE_KW;}
{IF} 						{return IF_KW;}
{DEFAULT} 					{return DEFAULT_KW;}
{DO} 						{return DO_KW;}
{INT} 						{return INT_KW;}
{SWITCH} 					{return SWITCH_KW;}
{DOUBLE} 					{return DOUBLE_KW;}
{LONG} 						{return LONG_KW;}
{ELSE} 						{return ELSE_KW;}
{MATRIX} 					{return MATRIX_KW;}


{ID}						{	yylval.symp = tables->lookup(yytext);
								return ID;	}
{INT_CONST}					{	yylval.intval = atoi(yytext);
								return INT_NUM; }
{FLOAT_CONST}				{	yylval.charval = yytext;
								return FLOAT_NUM;}
{CHAR_CON}					{return CHAR_CONST;}
{S_LITERAL}					{	yylval.charval = yytext;
								return STRING_LITERAL;	}


"["							{return LEFT_SQ_BRACKT;}
"]"							{return RIGHT_SQ_BRACKT;}
"("							{return LEFT_BRACKT;}
")"							{return RIGHT_BRACKT;}
"{"							{return LEFT_CUR_BRACKT;}
"}"							{return RIGHT_CUR_BRACKT;}
"."							{return STOP;}
"->"						{return ARROW;}
"++"						{return INCREMENT;}
"--"						{return DECREMENT;}
"&"							{return BITWISE_AND;}

"*"							{return MULTIPLY;}
"+"							{return ADD;}
"-"							{return SUBTRACT;}
"~"							{return TILDA;}
"!"							{return NEGATION;}
"/"							{return DIVIDE;}
"%"							{return PERCENT;}
"<<"						{return SHL;}
">>"						{return SHR;}

"<"							{return LESS_THAN;}
">"							{return GREATER_THAN;}
"<="						{return LESS_THAN_EQUALS;}
">="						{return GREATER_THAN_EQUALS;}
"==" 						{return EQUALITY;}
"!="						{return NOT_EQUALS;}
"^"							{return POWER;}
"|"							{return BITWISE_OR;}
"&&"						{return AND;}
"||"						{return LOGICAL_OR;}
"?"							{return QUESTION;}
":"							{return COLON;}
";"							{return SEMICOLON;}
"="							{return EQUALS;}
"*="						{return MULTIPLY_EQUALS;}
"/="						{return DIVIDE_EQUALS;}
"%="						{return MODULO_EQUALS;}
"+="						{return ADD_EQUALS;}
"-="						{return SUBTRACT_EQUALS;}
"<<="						{return SHL_EQUALS;}
">>="						{return SHR_EQUALS;}
"&="						{return AND_EQUALS;}
"^="						{return POWER_EQUALS;}
"|="						{return OR_EQUALS;}
","							{return COMMA;}
"#"							{return HASH;}
".'"						{return UNIQUE;}


[\t\n]
.
%%


int column = 0;
void count(void) 
{
	int i;
	for (i = 0; yytext[i] != '\0'; i++)
		if (yytext[i] == '\n') column = 0;
		else if (yytext[i] == '\t')
			column += 8-(column % 8);
		else
			column++;
	ECHO;
}


void comment(void) 
{
	char c, prev = 0;
	while ((c = getchar()) != 0)
	{     /* (EOF maps to 0) */
		if (c == '/' && prev == '*')
			return;
		prev = c;
	}
//	yyerror("unterminated comment");
}




