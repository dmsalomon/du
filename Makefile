
CFLAGS=-g -Wall -O1

du: du.o set.o

.PHONY: clean
clean:
	$(RM) du *.o
