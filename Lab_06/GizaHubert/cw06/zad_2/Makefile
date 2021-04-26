client: client.c
	make clear
	gcc -Wall -o client client.c -lrt
	make clear
server: server.c
	make clear
	gcc -Wall -o server server.c -lrt
	make clear
both: client.c server.c
	make client
	make server
test: client.c server.c
	make ca
	make both
	./server
clear:
	rm -f *.o *.a *.so *.d
clear_o:
	rm -f *.o
ca:
	rm -f *.o *.a *.so *.d main server client