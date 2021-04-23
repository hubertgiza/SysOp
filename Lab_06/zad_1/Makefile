server: server.c library.h
	make clear
	$(CC) library.h
	$(CC) server.c
	ar crs liblibrary.a library.o server.o
	gcc -Wall server.c -o server -L. -llibrary
client: client.c
	make clear
	$(CC) -c client.c
	$(CC) client.o -o client
serverr: client.c
	make clear
	$(CC) -c server.c
	$(CC) server.o -o server
clear:
	rm -f *.o *.a *.so *.d
clear_o:
	rm -f *.o
clear_all:
	rm -f *.o *.a *.so *.d main