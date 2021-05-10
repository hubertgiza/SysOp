main: main.c
	make clear
	gcc -Wall -o main main.c
	make clear
test: main
	make main
	./main 3 4
test_2: main
	make main
	./main 6 8
clear:
	rm -f *.o *.a *.so *.d
clear_o:
	rm -f *.o
ca:
	rm -f *.o *.a *.so *.d main server client