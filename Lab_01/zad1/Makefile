SMALL_TESTS = test_small_*.txt
BIG_TESTS = test_big*.txt
CC = gcc -Wall -c
CC1 = gcc -Wall -c -O1
CC2 = gcc -Wall -c -O2
CC3 = gcc -Wall -c -O3
static:
	make clear
	$(CC) library.c
	$(CC) main.c
	ar crs liblibrary.a library.o main.o
	gcc -Wall main.c -o main -L. -llibrary
	./main create_table 50 merge_files $(BIG_TESTS) remove_block 5 remove_row 1 7
static1:
	make clear
	$(CC1) library.c
	$(CC1) main.c
	ar crs liblibrary.a library.o main.o
	gcc -Wall -O1 main.c -o main -L. -llibrary
	./main create_table 50 merge_files $(BIG_TESTS) remove_block 5 remove_row 1 7
static2:
	make clear
	$(CC2) library.c
	$(CC2) main.c
	ar crs liblibrary.a library.o main.o
	gcc -Wall -O2 main.c -o main -L. -llibrary
	./main create_table 50 merge_files $(BIG_TESTS) remove_block 5 remove_row 1 7
static3:
	make clear
	$(CC3) library.c
	$(CC3) main.c
	ar crs liblibrary.a library.o main.o
	gcc -Wall -O3 main.c -o main -L. -llibrary
	./main create_table 50 merge_files $(BIG_TESTS) remove_block 5 remove_row 1 7
shared:
	make clear
	$(CC) -fPIC library.c
	$(CC) main.c
	gcc -Wall -shared -fPIC -o liblibrary.so library.o main.o
	gcc main.c -Wall -o main -L. -llibrary -Wl,-rpath=`pwd`
	rm -f *.o
	./main create_table 50 merge_files $(BIG_TESTS) remove_block 5 remove_row 1 7
shared1:
	make clear
	$(CC1) -fPIC library.c
	$(CC1) main.c
	gcc -Wall -O1 -shared -fPIC -o liblibrary.so library.o main.o
	gcc main.c -O1 -Wall -o main -L. -llibrary -Wl,-rpath=`pwd`
	rm -f *.o
	./main create_table 50 merge_files $(BIG_TESTS) remove_block 5 remove_row 1 7
shared2:
	make clear
	$(CC2) -fPIC library.c
	$(CC2) main.c
	gcc -Wall -O2 -shared -fPIC -o liblibrary.so library.o main.o
	gcc main.c -O2 -Wall -o main -L. -llibrary -Wl,-rpath=`pwd`
	rm -f *.o
	./main create_table 50 merge_files $(BIG_TESTS) remove_block 5 remove_row 1 7
shared3:
	make clear
	$(CC3) -fPIC library.c
	$(CC3) main.c
	gcc -Wall -O3 -shared -fPIC -o liblibrary.so library.o main.o
	gcc main.c -O3 -Wall -o main -L. -llibrary -Wl,-rpath=`pwd`
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
dynamic1:
	make clear
	$(CC1) -fPIC  library.c
	$(CC1) main_dynamic.c
	gcc -Wall -O1 -shared -fPIC -o library.so library.o main_dynamic.o
	gcc -Wall -O1 main_dynamic.c -o main_dynamic -L. -ldl -D DLL
	rm -f *.o
	./main_dynamic create_table 50 merge_files $(BIG_TESTS) remove_block 5 remove_row 1 7
dynamic2:
	make clear
	$(CC2) -fPIC  library.c
	$(CC2) main_dynamic.c
	gcc -Wall -O2 -shared -fPIC -o library.so library.o main_dynamic.o
	gcc -Wall -O2 main_dynamic.c -o main_dynamic -L. -ldl -D DLL
	rm -f *.o
	./main_dynamic create_table 50 merge_files $(BIG_TESTS) remove_block 5 remove_row 1 7
dynamic3:
	make clear
	$(CC3) -fPIC  library.c
	$(CC3) main_dynamic.c
	gcc -Wall -O3 -shared -fPIC -o library.so library.o main_dynamic.o
	gcc -Wall -O3 main_dynamic.c -o main_dynamic -L. -ldl -D DLL
	rm -f *.o
	./main_dynamic create_table 50 merge_files $(BIG_TESTS) remove_block 5 remove_row 1 7
clear:
	rm -f *.o *.a *.so *.d main