a.out: main.o libmyl.a
	gcc main.o -L. -lmyl

main.o:	main.c myl.h
	gcc -Wall -c main.c

libmyl.a:	ass2_15CS30012.o
		ar -rcs libmyl.a ass2_15CS30012.o

ass2_15CS30012.o:	ass2_15CS30012.c myl.h
	gcc -Wall -c ass2_15CS30012.c
clean:
	rm a.out main.o libmyl.a ass2_15CS30012.o
