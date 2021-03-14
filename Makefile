CFLAGS = -o cshell

all: cshell.c
	gcc $(CFLAGS) cshell.c

clean:
	rm -f cshell *.o