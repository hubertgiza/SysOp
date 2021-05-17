obj_1 = dla.ascii.pgm
obj_2 = baboon.ascii.pgm
output_1 = output1.pgm
output_2 = output2.pgm
main: zad_1.c
	make clear
	gcc -Wall -o main zad_1.c -lpthread
	make clear
test:
	make test1
	make test2
	make test3
	make test4
test1: main
	make main
	./main 2 number $(obj_1) $(output_1)
	./main 4 number $(obj_1) $(output_1)
	./main 8 number $(obj_1) $(output_1)
	./main 16 number $(obj_1) $(output_1)
test2: main
	make main
	./main 2 block $(obj_1) $(output_1)
	./main 4 block $(obj_1) $(output_1)
	./main 8 block $(obj_1) $(output_1)
	./main 16 block $(obj_1) $(output_1)
test3: main
	make main
	./main 2 number $(obj_2) $(output_2)
	./main 4 number $(obj_2) $(output_2)
	./main 8 number $(obj_2) $(output_2)
	./main 16 number $(obj_2) $(output_2)
test4: main
	make main
	./main 2 block $(obj_2) $(output_2)
	./main 4 block $(obj_2) $(output_2)
	./main 8 block $(obj_2) $(output_2)
	./main 16 block $(obj_2) $(output_2)
clear:
	rm -f *.o *.a *.so *.d
clear_o:
	rm -f *.o
ca:
	rm -f *.o *.a *.so *.d main