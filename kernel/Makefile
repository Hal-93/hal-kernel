EXE_FILE := main

CFLAGS := -O0 -g3 -Wall -std=gnu99

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

%.o: %.c toyker.h
	gcc $(CFLAGS)  -c -o $@ $<

all: $(EXE_FILE)

$(EXE_FILE): $(OBJS)
	gcc -o "$(EXE_FILE)" $(OBJS)

clean:
	rm *.o $(EXE_FILE)

.PHONY: all clean
