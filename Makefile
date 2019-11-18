CFLAGS=-static -Wall -DMINIC_DEV
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

minic: $(OBJS)
	gcc -o $@ $(OBJS) $(CFLAGS)

test: minic
	./test/test.sh

self: minic
	./self/self-compile.sh

selftest: minic
	./self/self-test.sh

clean:
	rm -f minic *.o *~ ./test/tmp* ./self/selfminic ./self/self.s ./self/all.c ./self/tmp*

.PHONY: self test clean
