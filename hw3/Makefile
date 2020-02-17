CC = gcc
CFLAGS = -Wall
DEPS = accountSearchTree.h
A_OBJ = transfProg.c accountSearchTree.c
	
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<
	
transfProg: $(A_OBJ)
	gcc -pthread $(CFLAGS) -o $@ $^