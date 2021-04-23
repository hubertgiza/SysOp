CC = gcc -Wall
test: producer.c consumer.c zad_3.c
	make clear_all
	$(CC) -c producer.c
	$(CC) -c consumer.c
	$(CC) producer.o -o producer
	$(CC) consumer.o -o consumer
	$(CC) -c zad_3.c
	$(CC) zad_3.o -o main
	./main
	make clear_all
clear_all:
	rm -f *.o *.a *.so *.d fif producer consumer main