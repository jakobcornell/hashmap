CFLAGS=-std=c99 -Wall -Wextra -pedantic

test: hashmap.o test.c
	gcc ${CFLAGS} test.c hashmap.o -o test

hashmap.o: hashmap.c hashmap.h
	gcc ${CFLAGS} -c hashmap.c

clean:
	rm -f hashmap.o test
