CC = gcc
CCFLAGS = -g -Wall -pthread
LDFLAGS = -g -lpthread

SRC = main.c output.c gym.c philosopher.c
OBJ = $(SRC:%.c=%.o)

all: philosophen_training
philosophen_training: $(OBJ)
	$(CC) -o philosophen_training $(LDFLAGS) $(OBJ)

# Dependencies
main.o: main.c output.h gym.h philosopher.h errors.h
gym.o: gym.c gym.h errors.h
output.o: output.c output.h
philosopher.o: philosopher.c philosopher.h errors.h