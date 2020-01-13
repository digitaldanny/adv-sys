CC = gcc
CFLAGS = -Wall
DEPS = dictionary.h common.h
A_OBJ = mapper.o common.o
B_OBJ = reducer.o dictionary.o common.o

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

all: mapper reducer

mapper: $(A_OBJ)
	gcc $(CFLAGS) -o $@ $^

reducer: $(B_OBJ)
	gcc $(CFLAGS) -o $@ $^