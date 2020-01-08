CC = gcc
CFLAGS = -Wall
DEPS = common.h
A_OBJ = mapper.o common.o
#B_OBJ = reducer.o common.o

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

mapper: $(A_OBJ)
	gcc $(CFLAGS) -o $@ $^

#reducer: $(B_OBJ)
#        gcc $(CFLAGS) -o $@ $^