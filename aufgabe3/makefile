CC = gcc
CFLAGS = -g -std=gnu99 -Wall
LDFLAGS = -g

SRC = mmanage.c vmappl.c
OBJ = $(SRC:%.c=%.o)

all: mmanage vmappl
mmanage: mmanage.o
	$(CC) -o mmanage $(LDFLAGS) mmanage.o
vmappl: vmappl.o
	$(CC) -o vmappl $(LDFLAGS) vmappl.o

.PHONY: clean
clean:
	rm -rf $(OBJ)
	rm -rf mmanage vmappl

.PHONY: deps
deps:
	$(CC) -MM $(SRC) > makefile.dependencies

include makefile.dependencies
