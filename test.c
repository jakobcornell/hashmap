/*
 * Copyright 2017 Jakob Cornell
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#include "hashmap.h"

extern struct hashmap_api hashmap;
extern const size_t INITIAL_CAPACITY;

size_t string_hash(void *key) {
	char *str = (char *) key;
	size_t hash = 37;
	for (size_t i = 0; str[i] != '\0'; i += 1) {
		hash = (hash * 54059) ^ (str[i] * 76963);
	}
	return hash;
}

unsigned int string_equals(void *left_key, void *right_key) {
	return strcmp(left_key, right_key) == 0;
}

size_t int_hash(void *key) {
	return (size_t) *((int *) key);
}

unsigned int int_equals(void *left_key, void *right_key) {
	return *((int *) left_key) == *((int *) right_key);
}

int main(void) {
	puts("--- basic tests with small map ---");
	putchar('\n');
	{
		struct hashmap map;
		hashmap.initialize(&map, 0.7, string_hash, string_equals);

		printf("%d: size when empty\n", map.size == 0);

		hashmap.put(&map, "foo", "bar");
		printf("%d: `put` and size update\n", map.size == 1);

		printf("%d: `contains`\n", hashmap.contains(&map, "foo"));

		printf("%d: `get`\n", strcmp(hashmap.get(&map, "foo"), "bar") == 0);

		char *keys[map.size];
		hashmap.fill_keys((void **) keys, &map);
		printf("%d: `keys`\n", strcmp(keys[0], "foo") == 0);

		char *values[map.size];
		hashmap.fill_values((void **) values, &map);
		printf("%d: `values`\n", strcmp(values[0], "bar") == 0);

		hashmap.remove(&map, "foo");
		printf("%d: `remove` and size update\n", map.size == 0);

		printf("%d: `contains` with absent key\n", !hashmap.contains(&map, "foo"));

		hashmap.finish(&map);
	}
	putchar('\n');

	puts("--- tests of large map with fixed capacity ---");
	putchar('\n');
	{
		struct hashmap map;
		hashmap.initialize(&map, DBL_MAX, int_hash, int_equals);

		{
			int nums[2000];
			unsigned int contains_result = 1;
			unsigned int get_result = 1;
			for (size_t i = 0; i < 2000; i += 1) {
				nums[i] = (int) i;
				hashmap.put(&map, &nums[i], &nums[i]);
				contains_result = contains_result && hashmap.contains(&map, &nums[i]);
				get_result = get_result && hashmap.get(&map, &nums[i]) == &nums[i];
			}

			printf("%d: `contains` test during insertion\n", contains_result);
			printf("%d: `get` test during insertion\n", get_result);
			printf("%d: size after insertion\n", map.size == 2000);
		}

		{
			unsigned int remove_result = 1;
			unsigned int contains_result = 1;
			unsigned int get_result = 1;
			for (size_t i = 0; i < 2000; i += 1) {
				int key = i;
				remove_result = remove_result && hashmap.remove(&map, &key);
				contains_result = contains_result && !hashmap.contains(&map, &key);
				get_result = get_result && hashmap.get(&map, &key) == NULL;
			}

			printf("%d: `remove` return value\n", remove_result);
			printf("%d: `contains` with absent key\n", contains_result);
			printf("%d: `get` with absent key\n", get_result);
			printf("%d: size after clearing\n", map.size == 0);
		}

		hashmap.finish(&map);
	}
	putchar('\n');

	puts("--- tests of map between resizes ---");
	putchar('\n');
	{
		struct hashmap map;
		hashmap.initialize(&map, 1.0, int_hash, int_equals);

		size_t size = INITIAL_CAPACITY + 1;
		int nums[size];
		for (size_t i = 0; i < size; i += 1) {
			nums[i] = (int) i;
			hashmap.put(&map, &nums[i], &nums[i]);
		}

		unsigned int contains_result = 1;
		unsigned int get_result = 1;
		for (size_t i = 0; i < size; i += 1) {
			int key = i;
			contains_result = contains_result && hashmap.contains(&map, &key);
			get_result = get_result && hashmap.get(&map, &key) == &nums[i];
		}

		printf("%d: `contains`\n", contains_result);
		printf("%d: `get`\n", get_result);
		printf("%d: size\n", map.size == size);

		hashmap.finish(&map);
	}

	return 0;
}
