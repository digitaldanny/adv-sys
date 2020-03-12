CFLAGS = -Wall
DEPS = fifo.h mapper.h reducer.h
OBJ = combiner.o fifo.o mapper.o reducer.o

%.o: %.c $(DEPS)
	gcc -g $(CFLAGS) -c -o $@ $<
	
combiner: $(OBJ)
	gcc -g -pthread $(CFLAGS) -o $@ $^