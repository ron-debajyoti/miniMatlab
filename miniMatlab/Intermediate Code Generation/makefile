a.out: lex.yy.o y.tab.o translator.o
	g++ lex.yy.o y.tab.o translator.o -lfl

translator.o: translator.cxx translator_1.h
	g++ -c translator_1.h
	g++ -c translator.cxx

lex.yy.o: lex.yy.c
	g++ -c lex.yy.c

y.tab.o: y.tab.c
	g++ -c y.tab.c

lex.yy.c: lexer.l y.tab.h translator_1.h
	flex lexer.l

y.tab.c: parser.y
	yacc -dtv parser.y -W

y.tab.h: parser.y
	yacc -dtv parser.y -W

clean:
	rm lex.yy.c y.tab.c y.tab.h lex.yy.o y.tab.o translator.o y.output translator_1.h.gch a.out


test:
	./a.out < test.mm

run:
	./a.out < test.mm > output_quad.out
