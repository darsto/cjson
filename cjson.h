#include <stdint.h>
#include <inttypes.h>

#define CJSON_MIN_POOLSIZE 8

enum {
	CJSON_TYPE_NONE = 0,
	CJSON_TYPE_STRING,
	CJSON_TYPE_INTEGER,
	CJSON_TYPE_FLOAT,
	CJSON_TYPE_ARRAY,
	CJSON_TYPE_OBJECT,
};

struct cjson_mempool;

struct cjson {
	struct cjson *parent;
	union {
		struct cjson *next;
		struct cjson_mempool *mem;
	};
	char *key;
	uint32_t type;
	uint32_t count; /**< children count */
	union {
		char *s;
		int64_t i;
		double d;
		struct cjson *a;
	};
};

struct cjson *cjson_parse(char *str);
struct cjson *cjson_obj(struct cjson *json, const char *key);
void cjson_free(struct cjson *json);
