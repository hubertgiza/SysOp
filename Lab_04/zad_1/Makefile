CC = gcc -Wall

main: zad_1.c another.c
	make clear
	$(CC) -c zad_1.c another.c
	$(CC) zad_1.o -o main
	$(CC) another.o -o another
	make clear_o
clear:
	rm -f *.o *.a *.so *.d
clear_o:
	rm -f *.o
clear_all:
	rm -f *.o *.a *.so *.d main another