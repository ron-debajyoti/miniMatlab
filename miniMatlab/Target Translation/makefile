CODE_DIR = library

.PHONY: library

miniMatlab: lex.yy.o y.tab.o translator.o target_translator.o
	g++ lex.yy.o y.tab.o translator.o \
	target_translator.o -lfl -o miniMatlab \
	-lboost_program_options

target_translator.o: target_translator.cxx
	g++ -c target_translator.cxx

translator.o: translator.cxx translator.h
	g++ -c translator.h
	g++ -c translator.cxx


lex.yy.o: lex.yy.c
	g++ -c lex.yy.c

y.tab.o: y.tab.c
	g++ -c y.tab.c

lex.yy.c: lexer.l y.tab.h translator.h
	flex lexer.l

y.tab.c: parser.y
	yacc -dtv parser.y -W

y.tab.h: parser.y
	yacc -dtv parser.y -W

clean:
	rm lex.yy.c y.tab.c y.tab.h lex.yy.o y.tab.o translator.o y.output miniMatlab target_translator.o test.log test.s test.out

cleanLib:
	$(MAKE) -C $(CODE_DIR) clean

test: 
	./miniMatlab test.c

asm:
	./miniMatlab -s test.s test.c > test.log

library:
	$(MAKE) -C $(CODE_DIR)