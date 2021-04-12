CC = gcc -Wall

main: catcher.c sender.c
	make clear
	$(CC) -c sender.c catcher.c
	$(CC) sender.o -o sender
	$(CC) catcher.o -o catcher
	make clear_o
clear:
	rm -f *.o *.a *.so *.d
clear_o:
	rm -f *.o
clear_all:
	rm -f *.o *.a *.so *.d main catcher sender