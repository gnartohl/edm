CFLAGS = -O0 -g -I$(HOME)/util

CC = cc $(CFLAGS)

compile = $(CC) -o $@ -c ./$*.c

objects : client_test server_test
	@echo "done"

client_test : client_test.o ../../libutil.so
	cc -O0 -g -L$(HOME)/util -lrt -lc_r -lm -lutil \
	-o client_test client_test.o

server_test : server_test.o ../../libutil.so
	cc -O0 -g -L$(HOME)/util -lrt -lc_r -lm -lutil \
	-o server_test server_test.o

server_test.o : server_test.c
	$(compile)

client_test.o : client_test.c
	$(compile)

#

# DO NOT DELETE THIS LINE -- make depend uses it