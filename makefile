CC = g++
#CC = gcc

CFLAGS = -g -c -Wall -pedantic
#CFLAGS = -ansi -c -Wall -pedantic

all: listener broadcaster

listener: listener.o
	    $(CC) -o listener listener.o

broadcaster: broadcaster.o
	    $(CC) -o broadcaster broadcaster.o

broadcaster.o: broadcaster.cpp
		$(CC) $(CFLAGS) broadcaster.cpp

listener.o: listener.cpp
		$(CC) $(CFLAGS) listener.cpp

clean:
	rm *.o
	rm listener
	rm broadcaster

%.o: %.c

	$(CC) $(CFLAGS) $*.c


