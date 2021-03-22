CC = gcc -Wall

main: zad_4.c
	make clear
	$(CC) -c zad_4.c
	$(CC) zad_4.o -o main
	make clear_o
main_system: zad_4_system.c
	make clear
	$(CC) -c zad_4_system.c
	$(CC) zad_4_system.o -o main_system
	make clear_o
all:
	make main
	make main_system
test: main
	make main
	time ./main input.txt output.txt haha W
	make clear_all
test_system: main
	make main_system
	time ./main_system input.txt output.txt  haha W
	make clear_all
full_test:
	make test
	make test_system
	make clear_all
clear:
	rm -f *.o *.a *.so *.d
clear_o:
	rm -f *.o
clear_all:
	rm -f *.o *.a *.so *.d main main_system