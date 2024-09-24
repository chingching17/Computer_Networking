all:
	gcc -o client client.c
	gcc -o server server.c

test: all
	gcc -o test test.c
	stdbuf -oL ./test

clean:
	rm -f server client test
