CC = gcc -Wall

main: zad_2.c
	make clear
	$(CC) -c zad_2.c
	$(CC) zad_2.o -o main
	make clear_o
clear:
	rm -f *.o *.a *.so *.d
clear_o:
	rm -f *.o
clear_all:
	rm -f *.o *.a *.so *.d main