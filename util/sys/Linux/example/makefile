CFLAGS = -Wall -O0 -g -DCONFIG_linuxglibc -D_REENTRANT -I/util
LDFLAGS = -g

all : test_queue

test_queue : test_queue.o
	gcc $(LDFLAGS) -o test_queue -L/util \
	-l114135a4-6f6c-11d3-95bc-00104b8742df \
	-lpthread test_queue.o

test_queue.o : test_queue.c
	gcc $(CFLAGS) -o test_queue.o -c test_queue.c
