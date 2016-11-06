CC = gcc
CFLAGS = -g -Wall -pthread
LDFLAGS = -g -lpthread

SRC = main.c gym.c philosophers.c
OBJ = $(SRC:%.c=%.o)

all: philosophen_training
philosophen_training: $(OBJ)
	$(CC) -o philosophen_training $(LDFLAGS) $(OBJ)

.PHONY: deps
deps:
	$(CC) -MM $(SRC) > makefile.dependencies

include makefile.dependencies