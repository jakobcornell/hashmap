VERSION=`cat version`

test: hashmap.h hashmap.c test.c
	gcc -g -std=c99 -Wall -Wextra -pedantic -o test test.c hashmap.c

tar: hashmap.h hashmap.c test.c makefile license readme version changelog
	tar --create --directory=..\
		hashmap/hashmap.h\
		hashmap/hashmap.c\
		hashmap/test.c\
		hashmap/makefile\
		hashmap/license\
		hashmap/readme\
		hashmap/version\
		hashmap/changelog\
	>hashmap-$(VERSION).tar

clean:
	rm -f test hashmap-$(VERSION).tar
