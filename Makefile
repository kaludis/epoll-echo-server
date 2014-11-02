CC = gcc

# set DEBUG options
ifdef DEBUG
CFLAGS = -Wall -Wextra -ggdb -pg -DDEBUG
else
CFLAGS = -O2
endif

OBJS = test.o server.o

all: server

server: $(OBJS)
	$(CC) -o server $^

debug :
	make all DEBUG=1

%.o : %.c
	$(CC) $(CFLAGS) -o $@ -c $^

clean:
	rm -f *.o server
