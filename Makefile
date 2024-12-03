CC = gcc
CFLAGS = -Wall -g
DEPS = bank.h
OBJ = main.o bank.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

bank: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f *.o bank