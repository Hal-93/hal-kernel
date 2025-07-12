CC = gcc
CFLAGS = -Wall -O2

SRCS = appli.c kernel.c tqueue.c
OBJS = $(SRCS:.c=.o)
TARGET = main

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)
clean:
	rm -f $(TARGET) $(OBJS)