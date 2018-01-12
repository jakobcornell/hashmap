/*
 * Copyright 2017 Jakob Cornell
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

struct hashmap {
	struct map_entry **table;
	size_t size;
	size_t capacity;
	double max_load;

	size_t (*hash)(void *key);
	unsigned int (*equals)(void *, void *);
};

struct hashmap_api {
	/*
	 * Initializes a hash map structure.
	 * Parameters:
	 *  - map to initialize
	 *  - maximum load (element-to-capacity ratio): should be greater than zero
	 *  - hash function for keys
	 *  - equality test function for keys
	 * Returns 1 on success, 0 on failure.
	 */
	unsigned int (*initialize)(struct hashmap *map, double max_load, size_t (*hash)(void *), unsigned int (*equals)(void *, void *));

	/*
	 * Deallocates all map resources allocated by the library.
	 */
	void (*finish)(struct hashmap *map);

	/*
	 * Fills an array with the keys or values in the map. Buffers should be large enough to hold `map->size` pointers.
	 */
	void (*fill_keys)(void *buffer[], struct hashmap *map);
	void (*fill_values)(void *buffer[], struct hashmap *map);

	/*
	 * Returns whether the map contains the specified key.
	 */
	unsigned int (*contains)(struct hashmap *map, void *key);

	/*
	 * Retrieves the value mapped to by the specified key, if any. Returns `NULL` if no mapping is present.
	 */
	void *(*get)(struct hashmap *map, void *key);

	/*
	 * Adds or updates the mapping for the specified key. Returns 1 on success, 0 on failure.
	 * `NULL` or other inaccessible pointers may be used for keys, provided that the map's hash and equality functions are implemented accordingly.
	 * Values may be arbitrary pointers.
	 */
	unsigned int (*put)(struct hashmap *map, void *key, void *value);

	/*
	 * Removes the entry with the specified key, if present. Returns whether an element was removed.
	 */
	unsigned int (*remove)(struct hashmap *map, void *key);
};
