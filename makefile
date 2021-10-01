CC=gcc

CFLAGS = -g -c -Wall -pedantic
#CFLAGS = -ansi -c -Wall -pedantic

all: listener broadcaster

listener: listener.o
	    $(CC) -o listener listener.o

broadcaster: broadcaster.o
	    $(CC) -o broadcaster broadcaster.o

clean:
	rm *.o
	rm listener
	rm broadcaster

%.o: %.c
	$(CC) $(CFLAGS) $*.c


