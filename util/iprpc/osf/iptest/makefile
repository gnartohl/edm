CFLAGS = -O0 -g -I/usr2/sinclair/util -I./

CC = cc $(CFLAGS)

compile = $(CC) -o $@ -c ./$*.c

objects : client_test server_test
	@echo "done"

client_test : client_test.o rpc_app.o ../../../libutil.so
	cc -O0 -g -L/usr2/sinclair/util -lrt -lc_r -lutil \
	-o client_test client_test.o rpc_app.o

server_test : server_test.o ../../../libutil.so
	cc -O0 -g -L/usr2/sinclair/util \
	-lrt -lc_r -lpthreads -lmach -lutil \
	-o server_test server_test.o

server_test.o : server_test.c
	$(compile)

client_test.o : client_test.c
	$(compile)

rpc_app.o : rpc_app.c
	$(compile)

#

# DO NOT DELETE THIS LINE -- make depend uses it
