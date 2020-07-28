#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h> /* TODO to be removed */
#include <errno.h>
#include <inttypes.h>

#define CJSON_MIN_POOLSIZE 8

enum {
	CJSON_TYPE_STRING = UINT32_MAX - 16,
	CJSON_TYPE_INTEGER,
	CJSON_TYPE_FLOAT,
	CJSON_TYPE_ARRAY,
	CJSON_TYPE_OBJECT,
};

struct cjson {
	struct cjson *parent, *next;
	char *key;
	union {
		uint32_t type;
		uint32_t count; /**< children count */
	};
	uint32_t unused;
	union {
		char *s;
		int64_t i;
		double d;
		struct cjson *a;
	};
};

struct cjson_mempool {
	unsigned count;
	unsigned capacity;
	struct cjson obj[0];
};

static struct cjson *
new_obj(struct cjson_mempool **mem_p)
{
	struct cjson_mempool *mem = *mem_p;
	size_t num_items = mem->capacity * 2;

	if (mem->count < mem->capacity) {
		return &mem->obj[mem->count++];
	}

	mem = *mem_p = calloc(1, sizeof(*mem) + num_items * sizeof(struct cjson));
	if (!mem) {
		assert(false);
		return NULL;
	}

	mem->capacity = num_items;
	mem->count = 1;
	return &mem->obj[0];
}

static int
add_child(struct cjson *parent, struct cjson *child)
{
	struct cjson *last = parent->a;

	if (!parent || parent->type != CJSON_TYPE_OBJECT) {
		assert(false);
		return -EINVAL;
	}

	if (!last) {
		parent->a = child;
		assert(child->next == NULL);
		child->next = NULL;
		return 0;
	}

	while (last->next) last = last->next;
	last->next = child;
	assert(child->next == NULL);
	child->next = NULL;
	return 0;
}

struct cjson *
cjson_parse(char *str)
{
	struct cjson_mempool *mem;
	struct cjson_mempool *topmem;
	struct cjson *top_obj; 
	struct cjson *cur_obj;
	char *cur_key = NULL;
	char *b = str;
	bool need_comma = false;

	if (*b++ != '{') {
		assert(false);
		return NULL;
	}

	topmem = mem = calloc(1, sizeof(*mem) + CJSON_MIN_POOLSIZE * sizeof(struct cjson));
	if (!mem) {
		assert(false);
		return NULL;
	}
	mem->capacity = CJSON_MIN_POOLSIZE;

	top_obj = cur_obj = new_obj(&mem);
	cur_obj->parent = NULL;
	cur_obj->key = "";
	cur_obj->type = CJSON_TYPE_OBJECT;

	while (*b) {
		switch(*b) {
			case '{': {
				struct cjson *obj;
				if (!cur_key) {
					assert(false);
					return NULL;
				}

				obj = new_obj(&mem);
				if (!obj) {
					assert(false);
					return false;
				}
				obj->parent = cur_obj;
				obj->key = cur_key;
				obj->type = CJSON_TYPE_OBJECT;
				if (add_child(cur_obj, obj) != 0) {
					assert(false);
					return false;
				}
				cur_obj = obj;
				break;
			}
			case '}': {
				need_comma = false; 
				if (cur_key) {
					assert(false);
					return NULL;
				}

				cur_obj = cur_obj->parent;
				if (!cur_obj) {
					return top_obj;
				}
				break;
			}
			case '"': {
				char *start = ++b;

				while (*b && *b != '"') b++;
				if (*b == 0 || *(b + 1) == NULL) {
					return NULL;
				}
				*b++ = 0;

				if (cur_key) {
					struct cjson *obj = new_obj(&mem);

					if (!obj) {
						assert(false);
						return false;
					}
					obj->parent = cur_obj;
					obj->key = cur_key;
					obj->type = CJSON_TYPE_STRING;
					obj->s = start;
					if (add_child(cur_obj, obj) != 0) {
						assert(false);
						return NULL;
					}

					cur_key = NULL;
					need_comma = true;
				} else {
					if (cur_obj->type != CJSON_TYPE_OBJECT) {
						assert(false);
						return NULL;
					}

					cur_key = start;
				}
				break;
			}
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			case '-':
			case '.':
			{
				struct cjson *obj;
				char *end;

				if (!cur_key) {
					assert(false);
					return NULL;
				}


				obj = new_obj(&mem);
				if (!obj) {
					assert(false);
					return NULL;
				}
				obj->parent = cur_obj;
				obj->key = cur_key;

				errno = 0;
				obj->i = strtoll(b, &end, 0);
				if (end == b || errno == ERANGE) {
					assert(false);
					return NULL;
				}

				if (*end == '.' || *end == 'e' || *end == 'E') {
					obj->d = strtod(b, &end);
					if (end == b || errno == ERANGE) {
						assert(false);
						return NULL;
					}
					obj->type = CJSON_TYPE_FLOAT;
				} else {
					obj->type = CJSON_TYPE_INTEGER;
				}

				if (add_child(cur_obj, obj) != 0) {
					assert(false);
					return false;
				}

				b = end - 1; /* will be incremented */
				cur_key = NULL;
				break;
			}
			case ',':
				need_comma = false;
				break;
			case ':':
			case ' ':
			default:
				break;
		}
		b++;
	}

	return top_obj;
	
}

struct cjson *
cjson_obj(struct cjson *json, const char *key)
{
	struct cjson *entry = json->a;

	while (entry) {
		if (strcmp(entry->key, key) == 0) {
			return entry;
		}
		entry = entry->next;
	}

	return NULL;
}