CC = gcc
CFLAGS = -Wall
DEPS = dictionary.h mapper.h reducer.h common.h
A_OBJ = combiner.o mapper.o dictionary.o reducer.o common.o

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

all: combiner
	
combiner: $(A_OBJ)
	gcc -pthread $(CFLAGS) -o $@ $^


