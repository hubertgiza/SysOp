main: main.c
	make clear
	gcc -Wall -o main main.c -lpthread
	make clear
test:
	make main
	./main
clear:
	rm -f *.o *.a *.so *.d
clear_o:
	rm -f *.o
ca:
	rm -f *.o *.a *.so *.d main