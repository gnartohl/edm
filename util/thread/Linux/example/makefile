CFLAGS = -Wall -O0 -g -I/util
#CFLAGS = -O0 -g
LDFLAGS = -g

all : test_thread test_thread_queue

test_thread : test_thread.o
	g++ $(LDFLAGS) -o test_thread -L/util \
	-lc -lpthread -lm \
	-l114135a4-6f6c-11d3-95bc-00104b8742df \
	test_thread.o

test_thread.o : test_thread.c
	g++ $(CFLAGS) -o test_thread.o -c test_thread.c

test_thread_queue : test_thread_queue.o
	g++ $(LDFLAGS) -o test_thread_queue -L/util \
	-lc -lpthread -lm \
	-l114135a4-6f6c-11d3-95bc-00104b8742df \
	test_thread_queue.o

test_thread_queue.o : test_thread_queue.c
	g++ $(CFLAGS) -o test_thread_queue.o -c test_thread_queue.c
