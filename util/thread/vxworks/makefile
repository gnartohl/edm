CFLAGS = -g2 -I/usr/users/sinclair/util

CC = cc $(CFLAGS)

compile = $(CC) -o $@ -c ./$*.c

objects : test
	@echo "done"

test : test.o ../../libutil.so
	cc -g2 -L/usr/users/sinclair/util \
	-lrt -lc_r -lmach -lpthreads -lutil -o test test.o

test.o : test.c
	$(compile)

#

# DO NOT DELETE THIS LINE -- make depend uses it