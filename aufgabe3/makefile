CC = gcc
CFLAGS = -g -std=gnu99 -pthread -Wall -DDEBUG_MESSAGES
LDFLAGS = -g -lrt -lpthread

SRC = mmanage.c vmappl.c vmaccess.c
OBJ = $(SRC:%.c=%.o)

all: mmanage vmappl
mmanage: mmanage.o
	$(CC) -o mmanage $^ $(LDFLAGS)

vmappl: vmappl.o vmaccess.o
	$(CC) -o vmappl $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -rf $(OBJ)
	rm -rf mmanage vmappl
	rm -rf logfile.txt pagefile.bin

.PHONY: deps
deps:
	$(CC) -MM $(SRC) > makefile.dependencies

include makefile.dependencies
