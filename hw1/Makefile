CC = gcc
CFLAGS = -Wall
DEPS = dictionary.h common.h
A_OBJ = mapper.o common.o
B_OBJ = reducer.o dictionary.o common.o
C_OBJ = combiner.o

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

all: mapper reducer combiner

mapper: $(A_OBJ)
	gcc $(CFLAGS) -o $@ $^

reducer: $(B_OBJ)
	gcc $(CFLAGS) -o $@ $^
	
combiner: $(C_OBJ)
	gcc $(CFLAGS) -o $@ $^


