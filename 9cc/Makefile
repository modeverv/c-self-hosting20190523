CFLAGS=-Wall -std=c11 -D_GNU_SOURCE -Wno-int-conversion -Wno-int-to-pointer-cast -Wno-return-type -Wno-implicit-function-declaration -Wno-pointer-to-int-cast
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

9cc: $(OBJS)
		$(CC) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): 9cc.h

test: 9cc
	./9cc -test
	./test.sh

clean:
	rm -f 9cc *.o *~ tmp*

