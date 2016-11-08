CC = gcc
CFLAGS = -g -std=gnu99 -Wall -pthread
LDFLAGS = -g -lpthread

SRC = main.c gym.c philosophers.c
OBJ = $(SRC:%.c=%.o)

all: philosophen_training
philosophen_training: $(OBJ)
	$(CC) -o philosophen_training $(LDFLAGS) $(OBJ)

.PHONY: clean
clean:
	rm -rf $(OBJ)
	rm -rf philosophen_training

.PHONY: deps
deps:
	$(CC) -MM $(SRC) > makefile.dependencies

include makefile.dependencies
