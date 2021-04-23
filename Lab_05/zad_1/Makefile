test: zad_1.c
	make clear
	$(CC) -c zad_1.c
	$(CC) zad_1.o -o main
	./main commands.txt
	make clear_all
clear:
	rm -f *.o *.a *.so *.d
clear_o:
	rm -f *.o
clear_all:
	rm -f *.o *.a *.so *.d main