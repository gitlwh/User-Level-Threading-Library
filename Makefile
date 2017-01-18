all:
	

	gcc -g -c testA.c -o testA.o
	gcc -g -c uthread.c -o uthread.o
	gcc -g -o testA testA.o uthread.o

