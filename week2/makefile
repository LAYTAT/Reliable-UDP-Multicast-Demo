CC=gcc

CFLAGS = -g -c -Wall -pedantic
#CFLAGS = -ansi -c -Wall -pedantic -D_GNU_SOURCE

all: bcast mcast


bcast: bcast.o
	$(CC) -o bcast bcast.o 

mcast: mcast.o
	$(CC) -o mcast mcast.o 

clean:
	rm *.o
	rm bcast
	rm mcast
