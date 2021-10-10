CC = g++
CFLAGS = -g -Wall -pedantic
#CFLAGS = -ansi -c -Wall -pedantic

all: mcast

mcast:mcast.o Processor.o
	$(CC) -o mcast mcast.o Processor.o

clean:
	rm *.o
	rm mcast

%.o: %.c

	$(CC) $(CFLAGS) $*.c


