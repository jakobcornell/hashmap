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

static void initialize(struct hashmap *, double, size_t (*)(void *), unsigned int (*)(void *, void *));
static void clean_up(struct hashmap *);
static void **keys(struct hashmap *);
static void **values(struct hashmap *);
static unsigned int contains(struct hashmap *, void *);
static void *get(struct hashmap *, void *);
static unsigned int put(struct hashmap *, void *, void *);
static unsigned int remove(struct hashmap *, void *);

static struct map_entry **get_entries(struct hashmap *);
static size_t key_index(struct hashmap *, void *);

struct hashmap_api hashmap = {
	.initialize = initialize,
	.clean_up = clean_up,
	.keys = keys,
	.values = values,
	.contains = contains,
	.get = get,
	.put = put,
	.remove = remove,
};

static void initialize(struct hashmap *map, double max_load, size_t (*hash)(void *), unsigned int (*equals)(void *, void *)) {
	map->table = calloc(INITIAL_CAPACITY, sizeof (struct map_entry *));
	map->size = 0;
	map->capacity = INITIAL_CAPACITY;
	map->max_load = max_load;
	map->hash = hash;
	map->equals = equals;
}

static void clean_up(struct hashmap *map) {
	struct map_entry **entries = get_entries(map);
	for (size_t i = 0; i < map->size; i += 1) {
		free(entries[i]);
	}
	free(entries);
	free(map->table);
}

static struct map_entry **get_entries(struct hashmap *map) {
	struct map_entry **storage = malloc(map->size * sizeof (struct map_entry *));
	size_t dst_i = 0;
	for (size_t src_i = 0; src_i < map->capacity; src_i += 1) {
		struct map_entry *entry = map->table[src_i];
		while (entry != NULL) {
			storage[dst_i] = entry;
			dst_i += 1;
			entry = entry->next;
		}
	}
	return storage;
}

static void **keys(struct hashmap *map) {
	struct map_entry **entries = get_entries(map);
	void **keys = malloc(map->size * sizeof (void *));
	for (size_t i = 0; i < map->size; i += 1) {
		keys[i] = entries[i]->key;
	}
	free(entries);
	return keys;
}

static void **values(struct hashmap *map) {
	struct map_entry **entries = get_entries(map);
	void **values = malloc(map->size * sizeof (void *));
	for (size_t i = 0; i < map->size; i += 1) {
		values[i] = entries[i]->value;
	}
	free(entries);
	return values;
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

	struct map_entry **new_table = calloc(new_capacity, sizeof (struct map_entry *));
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
