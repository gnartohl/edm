CFLAGS = -g2 -I/usr/users/sinclair/util

CC = gcc $(CFLAGS)

compile = $(CC) -o $@ -c ./$*.c

objects : test
	@echo "done"

test : test.o ../../libutil.so
	gcc -g2 -L/usr/users/sinclair/util \
	-lsocket -lnsl -lrt -lc -lpthread -lutil -o test test.o

test.o : test.c
	$(compile)

#

# DO NOT DELETE THIS LINE -- make depend uses it