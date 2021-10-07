CC=gcc

CFLAGS = -g -c -Wall -pedantic

all: test

test: test.o recv_dbg.o
	    $(CC) -o test test.o recv_dbg.o  

clean:
	rm *.o
	rm test

%.o:    %.c
	$(CC) $(CFLAGS) $*.c
