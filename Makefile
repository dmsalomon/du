
CFLAGS=-g -Wall -O3

OBJ=du.o\
    hash.o\

du: $(OBJ)

test: hash.c
	$(CC) -DTEST_HASH $< -o hash
	./hash

.PHONY: clean test
clean:
	$(RM) du hash *.o
