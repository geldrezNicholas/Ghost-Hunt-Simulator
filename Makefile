CC = gcc
CFLAGS = -Wall -pthread

TARGET = ghost_sim

SRCS = main.c house.c hunter.c ghost.c room.c roomstack.c evidence.c helpers.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c defs.h helpers.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET) log_*.csv
