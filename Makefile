CFLAGS=-static -Wall
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

minic: $(OBJS)
	gcc -D MINIC_DEV -o $@ $(OBJS) $(CFLAGS)

test: minic
	./test/test.sh

self: minic
	./self/self-compile.sh

clean:
	rm -f minic *.o *~ ./test/tmp* ./self/self.s ./self/all.c

.PHONY: self test clean
