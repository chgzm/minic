CFLAGS=-static -Wall -DMINIC_DEV
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

minic: $(OBJS)
	gcc -o $@ $(OBJS) $(CFLAGS)

test: minic
	./test/test.sh

self: minic
	./self/self-compile.sh

clean:
	rm -f minic *.o *~ ./test/tmp* ./test/selfminic ./self/self.s ./self/all.c

.PHONY: self test clean
