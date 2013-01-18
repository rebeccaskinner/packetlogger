# rm -f a.out; gcc -std=gnu99 -O3 -Wall -Wextra -Werror -pedantic -Wno-unused packet_logger.c -lnetfilter_queue
CC=gcc
CFLAGS=-O3 \
       -std=gnu99\
       -Wall\
       -Werror\
       -Wextra\
       -pedantic\
       -Wno-unused\
       $(shell pkg-config --cflags libnetfilter_queue)

LFLAGS=$(shell pkg-config --libs libnetfilter_queue)

all: packet_logger

%.o:%.c
	$(CC) -c $(CFLAGS) $(<) -o $(@)

packet_logger: packet_logger.o
	$(CC) $(<) $(LFLAGS) -o $(@)

clean:
	-rm -f *.o
	-rm -f pktlogger

.PHONY: all clean
