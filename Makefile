CFLAGS=-std=c11 -g -static -Wall
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

minic: $(OBJS)
	cc -o $@ $(OBJS) $(LDFLAGS)

minic: minic.c tokenizer.c

test: minic
	./test/test.sh

clean:
	rm -f minic *.o *~ ./test/tmp*

.PHONY: test clean
