CC = gcc
CFLAGS = -Wall -Wextra -std=c99
SRCS = src/job.c src/greedy.c src/bruteforce.c src/compare.c src/visual.c src/main.c
OBJS = $(SRCS:.c=.o)
TARGET = task_scheduler

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
