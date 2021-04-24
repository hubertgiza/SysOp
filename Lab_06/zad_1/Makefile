client: client.c
	make clear
	$(CC) -c client.c
	$(CC) client.o -o client
	make clear
server: server.c
	$(CC) -c server.c
	$(CC) server.o -o server
	make clear
both: client.c server.c
	make client
	make server
s: client.c server.c
	make ca
	make both
	./server
clear:
	rm -f *.o *.a *.so *.d
clear_o:
	rm -f *.o
ca:
	rm -f *.o *.a *.so *.d main server client