CFLAGS = -Wall -O0 -g -DCONFIG_linuxglibc -D_REENTRANT -I/util
LDFLAGS = -g

all : test_avl

test_avl : test_avl.o
	gcc $(LDFLAGS) -o test_avl -L/util \
	-l114135a4-6f6c-11d3-95bc-00104b8742df \
	-lpthread test_avl.o

test_avl.o : test_avl.c
	gcc $(CFLAGS) -o test_avl.o -c test_avl.c
