
CFLAGS=-g -Wall -O3

OBJ=du.o\
    hash.o\

du: $(OBJ)

.PHONY: clean
clean:
	$(RM) du *.o
