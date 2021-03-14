static:
	make clear
	gcc -Wall -c library.c
	gcc -Wall -c main.c
	ar crs liblibrary.a library.o main.o
	gcc -Wall main.c -o main -L. -llibrary
dynamic:
	make clear
	gcc -Wall -c library.c
	gcc -Wall -c main.c
	gcc -Wall -shared -fPIC -o liblibrary.so library.o main.o
	gcc main.c -Wall -o main -L. -llibrary -Wl,-rpath=`pwd`
	rm -f *.o
clear:
	rm -f *.o *.a *.so main


