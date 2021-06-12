server: server.c
	make clear
	gcc -Wall -o server server.c -lpthread
	make clear

client: client.c
	make clear
	gcc -Wall -o client client.c -lpthread
	make clear

both:
	make server
	make client

clear:
	rm -f *.o *.a *.so *.d
clear_o:
	rm -f *.o
ca:
	rm -f *.o *.a *.so *.d main