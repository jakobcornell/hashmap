/*
 * Copyright 2017 Jakob Cornell
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stddef.h>

#include "hashmap.h"

const size_t INITIAL_CAPACITY = 64;

struct map_entry {
	void *key;
	void *value;

	struct map_entry *next;
};

static unsigned int initialize(struct hashmap *map, double max_load, size_t (*hash)(void *key), unsigned int (*equals)(void *, void *));
static void finish(struct hashmap *map);
static void fill_keys(void *buffer[], struct hashmap *map);
static void fill_values(void *buffer[], struct hashmap *map);
static unsigned int contains(struct hashmap *map, void *key);
static void *get(struct hashmap *map, void *key);
static unsigned int put(struct hashmap *map, void *key, void *value);
static unsigned int remove(struct hashmap *map, void *key);

static void fill_entries(struct map_entry *buffer[], struct hashmap *map);
static size_t key_index(struct hashmap *map, void *key);

struct hashmap_api hashmap = {
	.initialize = initialize,
	.finish = finish,
	.fill_keys = fill_keys,
	.fill_values = fill_values,
	.contains = contains,
	.get = get,
	.put = put,
	.remove = remove,
};

static unsigned int initialize(struct hashmap *map, double max_load, size_t (*hash)(void *key), unsigned int (*equals)(void *, void *)) {
	map->table = malloc(INITIAL_CAPACITY * sizeof (struct map_entry *));
	if (map->table == NULL) {
		return 0;
	}
	else {
		for (size_t i = 0; i < INITIAL_CAPACITY; i += 1) {
			map->table[i] = NULL;
		}
		map->size = 0;
		map->capacity = INITIAL_CAPACITY;
		map->max_load = max_load;
		map->hash = hash;
		map->equals = equals;
		return 1;
	}
}

static void finish(struct hashmap *map) {
	struct map_entry *entries[map->size];
	fill_entries(entries, map);
	for (size_t i = 0; i < map->size; i += 1) {
		free(entries[i]);
	}
	free(map->table);
}

static void fill_entries(struct map_entry *buffer[], struct hashmap *map) {
	size_t dst_i = 0;
	for (size_t src_i = 0; src_i < map->capacity; src_i += 1) {
		struct map_entry *entry = map->table[src_i];
		while (entry != NULL) {
			buffer[dst_i] = entry;
			dst_i += 1;
			entry = entry->next;
		}
	}
}

static void fill_keys(void *buffer[], struct hashmap *map) {
	struct map_entry *entries[map->size];
	fill_entries(entries, map);
	for (size_t i = 0; i < map->size; i += 1) {
		buffer[i] = entries[i]->key;
	}
}

static void fill_values(void *buffer[], struct hashmap *map) {
	struct map_entry *entries[map->size];
	fill_entries(entries, map);
	for (size_t i = 0; i < map->size; i += 1) {
		buffer[i] = entries[i]->value;
	}
}

static unsigned int contains(struct hashmap *map, void *key) {
	size_t index = key_index(map, key);
	struct map_entry *entry = map->table[index];
	while (entry != NULL) {
		if (map->equals(entry->key, key)) {
			return 1;
		} else {
			entry = entry->next;
		}
	}
	return 0;
}

static size_t key_index(struct hashmap *map, void *key) {
	size_t c = map->capacity;
	return (map->hash(key) % c + c) % c; // nonnegative remainder
}

static void *get(struct hashmap *map, void *key) {
	size_t index = key_index(map, key);
	struct map_entry *entry = map->table[index];
	while (entry != NULL) {
		if (map->equals(entry->key, key)) {
			return entry->value;
		} else {
			entry = entry->next;
		}
	}
	return NULL;
}

static unsigned int resize(struct hashmap *map) {
	size_t new_capacity = map->capacity * 2;

	struct map_entry **new_table = malloc(new_capacity * sizeof (struct map_entry *));
	for (size_t i = 0; i < new_capacity; i += 1) {
		new_table[i] = NULL;
	}

	if (new_table == NULL) {
		return 0;
	} else {
		for (size_t i = 0; i < map->capacity; i += 1) {
			struct map_entry *entry = map->table[i];
			struct map_entry *next;
			while (entry != NULL) {
				next = entry->next;
				size_t index = key_index(map, entry->key);
				entry->next = new_table[index];
				new_table[index] = entry;
				entry = next;
			}
		}

		free(map->table);
		map->table = new_table;
		map->capacity = new_capacity;
		return 1;
	}
}

static unsigned int put(struct hashmap *map, void *key, void *value) {
	unsigned int present = 0;
	struct map_entry *target = NULL;

	for (size_t i = 0; i < map->capacity; i += 1) {
		struct map_entry *entry = map->table[i];
		while (entry != NULL) {
			if (map->equals(entry->key, key)) {
				present = 1;
				target = entry;
				goto SEARCH_END;
			} else {
				entry = entry->next;
			}
		}
	}
	SEARCH_END:

	if (present) {
		target->value = value;
		return 1;
	} else {
		int success;
		if ((double) (map->size + 1) / (double) map->capacity > map->max_load) {
			success = resize(map);
		} else {
			success = 1;
		}

		if (success) {
			size_t index = key_index(map, key);
			struct map_entry *entry = malloc(sizeof (struct map_entry));
			entry->key = key;
			entry->value = value;
			entry->next = map->table[index];
			map->table[index] = entry;
			map->size += 1;
			return 1;
		} else {
			return 0;
		}
	}
}

static unsigned int remove(struct hashmap *map, void *key) {
	size_t i = key_index(map, key);
	struct map_entry *entry = map->table[i];
	struct map_entry **parent = &map->table[i];
	while (entry != NULL) {
		if (map->equals(entry->key, key)) {
			*parent = entry->next;
			free(entry);
			map->size -= 1;
			return 1;
		} else {
			parent = &entry->next;
			entry = entry->next;
		}
	}
	return 0;
}
