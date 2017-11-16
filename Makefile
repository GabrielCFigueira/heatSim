CFLAGS= -g -Wall -pedantic 
CC=gcc

all: heatSim

heatSim: main.o thread.o matrix2d.o mplib3.o leQueue.o matrix2d.h thread.h mplib3.h leQueue.h mplib3.h
	$(CC) $(CFLAGS) -pthread -o heatSim main.o thread.o matrix2d.o mplib3.o leQueue.o

main.o: main.c thread.h matrix2d.h thread.h mplib3.h
	$(CC) $(CFLAGS) -c main.c 

thread.o: thread.c thread.h mplib3.h matrix2d.h
	$(CC) $(FLAGS) -c thread.c

matrix2d.o: matrix2d.c matrix2d.h
	$(CC) $(FLAGS) -c matrix2d.c

mplib.o: mplib3.c mplib3.h 
	$(CC) $(CFLAGS) -c mplib3.c

leQueue.o: leQueue.c leQueue.h
	$(CC) $(CFLAGS) -c leQueue.c

clean:
	rm -f *.o heatSim
