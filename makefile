CFLAGS=-std=c99 -Wall -Wextra -pedantic

.PHONY: clean

test: hashmap.a test.c
	gcc ${CFLAGS} test.c hashmap.a -o test

hashmap.a: hashmap.c hashmap.h
	gcc ${CFLAGS} -c hashmap.c
	ar rcs hashmap.a hashmap.o

clean:
	rm -f hashmap.a hashmap.o test
