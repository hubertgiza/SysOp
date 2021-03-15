SMALL_TESTS = test_small_*.txt
BIG_TESTS = test_big*.txt
CC = gcc -Wall -c -O1
static:
	make clear
	$(CC) library.c
	$(CC) main.c
	ar crs liblibrary.a library.o main.o
	gcc -Wall -O1 main.c -o main -L. -llibrary
	./main create_table 50 merge_files $(BIG_TESTS) remove_block 5 remove_row 1 7
shared:
	make clear
	$(CC) -fPIC library.c
	$(CC) main.c
	gcc -Wall -shared -fPIC -o liblibrary.so library.o main.o
	gcc main.c -Wall -o main -L. -llibrary -Wl,-rpath=`pwd`
	rm -f *.o
	./main create_table 50 merge_files $(BIG_TESTS) remove_block 5 remove_row 1 7
dynamic:
	make clear
	$(CC) -fPIC  library.c
	$(CC) main_dynamic.c
	gcc -Wall -shared -fPIC -o library.so library.o main_dynamic.o
	gcc -Wall main_dynamic.c -o main_dynamic -L. -ldl -D DLL
	rm -f *.o
	./main_dynamic create_table 50 merge_files $(BIG_TESTS) remove_block 5 remove_row 1 7
clear:
	rm -f *.o *.a *.so *.d main