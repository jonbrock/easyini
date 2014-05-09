test: test.o easyini.o
	gcc test.o easyini.o -o easyini

test.o: test.c easyini.h
	gcc -c test.c

easyini.o: easyini.c easyini.h
	gcc -c easyini.c
