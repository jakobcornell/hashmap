Files
=====

- `hashmap.h`: type definitions and function prototypes (with documentation) for hash map API
- `hashmap.c`: API implementation
- `test.c`: test suite

How to use
==========

The test suite can be used to quickly get an idea of how to use the API. For function documentation, see `hashmap.h`.

To avoid function name collisions and ugly function names, the API functions are packaged in a structure in `hashmap.c`. To use them, declare the API structure:

	extern struct hashmap_api hashmap;

and then access the functions as members of the struct, e.g.:

	struct hashmap map;
	hashmap.initialize(&map, ...);
	hashmap.put(&map, "foo", "bar");
	hashmap.finish(&map);

Client programs should include `hashmap.h` and compile against `hashmap.c`. The API uses the type `size_t`, so programs should also include their standard library's `stddef.h` or `stdlib.h`. The header file `hashmap.h` uses these type definitions, so these system headers should be included first.

Testing
=======

The test program prints test results as an integer followed by a description of the test. The integer is `0` if the test fails, and `1` if it passes.
