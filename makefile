CC = g++
CFLAGS = -G -Wall -pedantic
#CFLAGS = -ansi -c -Wall -pedantic
#TODO: delete -g after debug is done

all: mcast start_mcast

start_mcast: start_mcast.o
	$(CC) -o start_mcast start_mcast.o

mcast:mcast.o Processor.o
	$(CC) -o mcast mcast.o Processor.o

clean:
	rm *.o
	rm mcast
	rm start_mcast

%.o: %.c

	$(CC) $(CFLAGS) $*.c


