# rm -f a.out; gcc -std=gnu99 -O3 -Wall -Wextra -Werror -pedantic -Wno-unused packet_logger.c -lnetfilter_queue
CC=gcc
CFLAGS=-O3 \
       -std=gnu99\
       -Wall\
       -Werror\
       -Wextra\
       -pedantic\
       -Wno-unused

LFLAGS=-lnetfilter_queue

all: pktlogger

pktlogger:
	$(CC) $(CFLAGS) $(LFLAGS) packet_logger.c -o pktlogger

clean:
	-rm -f *.o
	-rm -f pktlogger

.PHONY: all clean
