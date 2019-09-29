CFLAGS=-static -Wall
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

minic: $(OBJS)
	gcc -o $@ $(OBJS) $(CFLAGS)

test: minic
	./test/test.sh

clean:
	rm -f minic *.o *~ ./test/tmp*

.PHONY: test clean
