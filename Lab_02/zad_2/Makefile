CC = gcc -Wall

main: zad_2.c
	make clear
	$(CC) -c zad_2.c
	$(CC) zad_2.o -o main
	make clear_o
main_system: zad_2_system.c
	make clear
	$(CC) -c zad_2_system.c
	$(CC) zad_2_system.o -o main_system
	make clear_o
all:
	make main
	make main_system
test: main
	make main
	time ./main a test.txt
	make clear_all
test_system: main
	make main_system
	time ./main_system a test.txt
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