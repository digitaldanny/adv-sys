CFLAGS = -Wall
DEPS = fifo.h
OBJ = combiner.o fifo.o

%.o: %.c $(DEPS)
	gcc -g $(CFLAGS) -c -o $@ $<
	
combiner: $(OBJ)
	gcc -g -pthread $(CFLAGS) -o $@ $^