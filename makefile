all: my_ipc

my_fm: my_ipc.c 
	gcc -Wall -o my_ipc my_ipc.c

clean:
	$(RM) my_ipc